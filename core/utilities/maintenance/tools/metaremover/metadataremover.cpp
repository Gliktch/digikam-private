/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-21-06
 * Description : Batch remover of metadata.
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2023      by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metadataremover.h"

// Qt includes

#include <QApplication>
#include <QMessageBox>
#include <QString>
#include <QTimer>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "albummanager.h"
#include "collectionmanager.h"
#include "iteminfojob.h"
#include "maintenancethread.h"

namespace Digikam
{

class Q_DECL_HIDDEN MetadataRemover::Private
{
public:

    Private() = default;

public:

    AlbumList                     palbumList;
    AlbumList::Iterator           albumsIt;

    ItemInfoJob*                  imageInfoJob  = nullptr;

    ItemInfoList                  imageInfoList;

    MaintenanceThread*            thread        = nullptr;

    MetadataRemover::RemoveAction removeAction  = MetadataRemover::None;
};

MetadataRemover::MetadataRemover(const AlbumList& list, RemoveAction action, ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("MetadataRemover"), parent),
      d              (new Private)
{
    if (list.isEmpty())
    {
        d->palbumList = AlbumManager::instance()->allPAlbums();
    }
    else
    {
        d->palbumList = list;
    }

    init(action);
}

MetadataRemover::MetadataRemover(const ItemInfoList& list, RemoveAction action, ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("MetadataRemover"), parent),
      d              (new Private)
{
    d->imageInfoList = list;

    init(action);
}

// Common methods ----------------------------------------------------------------------------

void MetadataRemover::init(RemoveAction action)
{
    d->removeAction = action;
    d->thread       = new MaintenanceThread(this);

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotDone()));

    connect(d->thread, SIGNAL(signalAdvanceInfo(ItemInfo,QImage)),
            this, SLOT(slotAdvance(ItemInfo)));
}

void MetadataRemover::setUseMultiCoreCPU(bool b)
{
    d->thread->setUseMultiCore(b);
}

void MetadataRemover::slotStart()
{
    MaintenanceTool::slotStart();

    d->imageInfoJob = new ItemInfoJob;

    connect(d->imageInfoJob, SIGNAL(signalItemsInfo(ItemInfoList)),
            this, SLOT(slotAlbumParsed(ItemInfoList)));

    connect(d->imageInfoJob, SIGNAL(signalCompleted()),
            this, SLOT(slotOneAlbumIsComplete()));

    connect(this, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SLOT(slotCancel()));

    QTimer::singleShot(500, this, SLOT(slotParseAlbums()));
}

MetadataRemover::~MetadataRemover()
{
    delete d->imageInfoJob;
    delete d;
}

void MetadataRemover::slotCancel()
{
    d->imageInfoJob->stop();
    d->thread->cancel();
    MaintenanceTool::slotCancel();
}

// Parse Albums methods ------------------------------------------------------------------

void MetadataRemover::slotParseAlbums()
{
    setUsesBusyIndicator(true);
    d->albumsIt = d->palbumList.begin();
    processOneAlbum();
}

void MetadataRemover::processOneAlbum()
{
    if (canceled())
    {
        return;
    }

    if (d->albumsIt == d->palbumList.end())     // All albums are parsed.
    {
        parseList();
        return;
    }

    d->imageInfoJob->allItemsFromAlbum(*d->albumsIt);
}

void MetadataRemover::slotAlbumParsed(const ItemInfoList& list)
{
    d->imageInfoList << list;
}

void MetadataRemover::slotOneAlbumIsComplete()
{
    d->albumsIt++;
    processOneAlbum();
}

// Parse info list methods -----------------------------------------------------------------------

void MetadataRemover::parseList()
{
    setUsesBusyIndicator(false);

    if      (d->removeAction == None)
    {
        slotDone();
        return;
    }
    else if (d->removeAction == Faces)
    {
        setLabel(i18n("Remove all faces from image"));
        setThumbnail(QIcon::fromTheme(QLatin1String("list-remove-user")).pixmap(48));
    }
    else if (d->removeAction == Tags)
    {
        setLabel(i18n("Remove all tags from image"));
        setThumbnail(QIcon::fromTheme(QLatin1String("tag-delete")).pixmap(48));
    }

    if (d->imageInfoList.isEmpty())
    {
        slotDone();
        return;
    }

    if (d->imageInfoList.size() > 1)
    {
        QString message;

        if      (d->removeAction == MetadataRemover::Faces)
        {
            message = i18n("All faces are removed from %1 selected images.\n"
                           "Do you want to continue?", d->imageInfoList.size());
        }
        else if (d->removeAction == MetadataRemover::Tags)
        {
            message = i18n("All tags are removed from %1 selected images.\n"
                           "Do you want to continue?", d->imageInfoList.size());
        }

        int ret = QMessageBox::question(qApp->activeWindow(), qApp->applicationName(),
                                        message, QMessageBox::Yes | QMessageBox::No);

        if (ret == QMessageBox::No)
        {
            slotDone();
            return;
        }
    }

    setTotalItems(d->imageInfoList.count());

    d->thread->removeMetadata(d->imageInfoList, d->removeAction);
    d->thread->start();
}

void MetadataRemover::slotAdvance(const ItemInfo& inf)
{
    uint adv = 0;

    if ((adv = checkProgressNeeded()) == 0)
    {
        return;
    }

    QString album = CollectionManager::instance()->albumRootLabel(inf.albumRootId());

    QString lbl = i18n("Clear Metadata: %1\n", inf.name());
    lbl.append(i18n("Album: %1\n", album + inf.relativePath()));
    setLabel(lbl);
    advance(adv);
}

void MetadataRemover::slotDone()
{
    QString lbl;

    if      (d->removeAction == Faces)
    {
        if (totalItems() > 1)
        {
            lbl.append(i18n("Items scanned to remove faces: %1", totalItems()));
        }
        else
        {
            lbl.append(i18n("Item scanned to remove faces: %1", totalItems()));
        }

        setThumbnail(QIcon::fromTheme(QLatin1String("list-remove-user")).pixmap(48));
    }
    else if (d->removeAction == Tags)
    {
        if (totalItems() > 1)
        {
            lbl.append(i18n("Items scanned to remove tags: %1", totalItems()));
        }
        else
        {
            lbl.append(i18n("Item scanned to remove tags: %1", totalItems()));
        }

        setThumbnail(QIcon::fromTheme(QLatin1String("tag-delete")).pixmap(48));
    }

    setLabel(lbl);

    MaintenanceTool::slotDone();
}

} // namespace Digikam

#include "moc_metadataremover.cpp"
