/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-30-08
 * Description : batch thumbnails generator
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "thumbsgenerator.h"

// Qt includes

#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QApplication>
#include <QString>
#include <QPixmap>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "coredbalbuminfo.h"
#include "collectionmanager.h"
#include "applicationsettings.h"
#include "albummanager.h"
#include "iteminfo.h"
#include "thumbsdb.h"
#include "thumbsdbaccess.h"
#include "maintenancethread.h"

namespace Digikam
{

class Q_DECL_HIDDEN ThumbsGenerator::Private
{
public:

    Private() = default;

public:

    bool                 rebuildAll = true;

    AlbumList            albumList;

    QStringList          allPicturesPath;

    QFuture<void>        affectedAlbumTask;
    QFutureWatcher<void> affectedAlbumWatcher;

    MaintenanceThread*   thread     = nullptr;
};

ThumbsGenerator::ThumbsGenerator(const bool rebuildAll,
                                 const AlbumList& list,
                                 ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("ThumbsGenerator"), parent),
      d              (new Private)
{
    d->albumList = list;

    init(rebuildAll);
}

ThumbsGenerator::ThumbsGenerator(const bool rebuildAll,
                                 int albumId,
                                 ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("ThumbsGenerator"), parent),
      d              (new Private)
{
    d->albumList.append(AlbumManager::instance()->findPAlbum(albumId));

    init(rebuildAll);
}

ThumbsGenerator::~ThumbsGenerator()
{
    delete d;
}

void ThumbsGenerator::init(const bool rebuildAll)
{
    d->rebuildAll = rebuildAll;
    d->thread     = new MaintenanceThread(this);

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotDone()));

    connect(d->thread, SIGNAL(signalAdvanceInfo(ItemInfo,QImage)),
            this, SLOT(slotAdvance(ItemInfo,QImage)));
}

void ThumbsGenerator::setUseMultiCoreCPU(bool b)
{
    d->thread->setUseMultiCore(b);
}

void ThumbsGenerator::slotCancel()
{
    d->thread->cancel();
    MaintenanceTool::slotCancel();
}

void ThumbsGenerator::slotStart()
{
    MaintenanceTool::slotStart();

    setLabel(i18n("Generate Thumbnails"));
    setThumbnail(QIcon::fromTheme(QLatin1String("photo")).pixmap(48));

    // Activate progress bar during album calculation.

    setTotalItems(1);

    connect(&d->affectedAlbumWatcher, &QFutureWatcher<void>::finished,
            this, &ThumbsGenerator::slotAffectedAlbumsFinished);

    d->affectedAlbumTask =
        QtConcurrent::run(

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                          &ThumbsGenerator::calculateAffectedAlbums, this

#else

                          this, &ThumbsGenerator::calculateAffectedAlbums

#endif

                         );

    d->affectedAlbumWatcher.setFuture(d->affectedAlbumTask);
}

void ThumbsGenerator::slotAffectedAlbumsFinished()
{
    if (d->allPicturesPath.isEmpty())
    {
        slotDone();

        return;
    }

    setTotalItems(d->allPicturesPath.count());

    d->thread->generateThumbs(d->allPicturesPath);
    d->thread->start();
}

void ThumbsGenerator::calculateAffectedAlbums()
{
    if (d->albumList.isEmpty())
    {
        d->albumList = AlbumManager::instance()->allPAlbums();
    }

    for (AlbumList::const_iterator it = d->albumList.constBegin() ;
         !canceled() && (it != d->albumList.constEnd()) ; ++it)
    {
        if (!(*it))
        {
            continue;
        }

        if      ((*it)->type() == Album::PHYSICAL)
        {
            const auto paths = CoreDbAccess().db()->getItemURLsInAlbum((*it)->id());

            for (const QString& path : paths)
            {
                if (!d->allPicturesPath.contains(path))
                {
                    d->allPicturesPath << path;
                }
            }
        }
        else if ((*it)->type() == Album::TAG)
        {
            const auto paths = CoreDbAccess().db()->getItemURLsInTag((*it)->id());

            for (const QString& path : paths)
            {
                if (!d->allPicturesPath.contains(path))
                {
                    d->allPicturesPath << path;
                }
            }
        }
    }

    if (!d->rebuildAll)
    {
        QHash<QString, int> filePaths = ThumbsDbAccess().db()->getFilePathsWithThumbnail();
        QStringList::iterator it      = d->allPicturesPath.begin();

        while (it != d->allPicturesPath.end())
        {
            if (filePaths.contains(*it))
            {
                it = d->allPicturesPath.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    // Remove non-image or video files from the list.

    QStringList::iterator it = d->allPicturesPath.begin();

    while (it != d->allPicturesPath.end())
    {
        ItemInfo info = ItemInfo::fromLocalFile(*it);

        if (
            (info.category() != DatabaseItem::Image) &&
            (info.category() != DatabaseItem::Video) &&
            (info.category() != DatabaseItem::Audio)
           )
        {
            it = d->allPicturesPath.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ThumbsGenerator::slotAdvance(const ItemInfo& inf, const QImage& img)
{
    uint adv = 0;

    if ((adv = checkProgressNeeded()) == 0)
    {
        return;
    }

    QString album = CollectionManager::instance()->albumRootLabel(inf.albumRootId());

    QString lbl   = i18n("Thumbnail: %1\n", inf.name());
    lbl.append(i18n("Album: %1\n", album + inf.relativePath()));
    setLabel(lbl);
    setThumbnail(QPixmap::fromImage(img));
    advance(adv);
}

void ThumbsGenerator::slotDone()
{
    setThumbnail(QIcon::fromTheme(QLatin1String("photo")).pixmap(48));

    QString lbl;

    if (totalItems() > 1)
    {
        lbl.append(i18n("Items scanned for thumbnails: %1", totalItems()));
    }
    else
    {
        lbl.append(i18n("Item scanned for thumbnails: %1", totalItems()));
    }

    setLabel(lbl);

    MaintenanceTool::slotDone();
}

} // namespace Digikam

#include "moc_thumbsgenerator.cpp"
