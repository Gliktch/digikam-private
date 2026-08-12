/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : image quality sorter maintenance tool
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imagequalitysorter.h"

// Qt includes

#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QString>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dimg.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "albummanager.h"
#include "collectionmanager.h"
#include "tagscache.h"
#include "picklabelwidget.h"
#include "maintenancethread.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImageQualitySorter::Private
{
public:

    Private() = default;

public:

    ImageQualitySettings  quality;

    QStringList           allPicturesPath;

    QFuture<void>         affectedAlbumTask;
    QFutureWatcher<void>  affectedAlbumWatcher;

    MaintenanceThread*    thread    = nullptr;
};

ImageQualitySorter::ImageQualitySorter(const ImageQualitySettings& quality,
                                       ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("ImageQualitySorter"), parent),
      d              (new Private)
{
    init(quality);
}

ImageQualitySorter::ImageQualitySorter(const QStringList& itemPaths,
                                       const ImageQualitySettings& quality,
                                       ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("ImageQualitySorter"), parent),
      d              (new Private)
{
    d->allPicturesPath = itemPaths;

    init(quality);
}

ImageQualitySorter::~ImageQualitySorter()
{
    delete d;
}

void ImageQualitySorter::init(const ImageQualitySettings& quality)
{
    d->quality = quality;
    d->thread  = new MaintenanceThread(this);

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotDone()));

    connect(d->thread, SIGNAL(signalAdvanceProgress(ItemInfo,QImage,int)),
            this, SLOT(slotAdvance(ItemInfo,QImage,int)));
}

void ImageQualitySorter::setUseMultiCoreCPU(bool b)
{
    d->thread->setUseMultiCore(b);
}

void ImageQualitySorter::slotCancel()
{
    d->thread->cancel();
    MaintenanceTool::slotCancel();
}

void ImageQualitySorter::slotStart()
{
    MaintenanceTool::slotStart();

    setLabel(i18n("Image Quality Scanner"));
    setThumbnail(QIcon::fromTheme(QLatin1String("flag-green")).pixmap(48));

    if (!d->allPicturesPath.isEmpty())
    {
        slotAffectedAlbumsFinished();

        return;
    }

    // Activate progress bar during album calculation.

    setTotalItems(1);

    connect(&d->affectedAlbumWatcher, &QFutureWatcher<void>::finished,
            this, &ImageQualitySorter::slotAffectedAlbumsFinished);

    d->affectedAlbumTask =
        QtConcurrent::run(

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                          &ImageQualitySorter::calculateAffectedAlbums, this

#else

                          this, &ImageQualitySorter::calculateAffectedAlbums

#endif

                         );

    d->affectedAlbumWatcher.setFuture(d->affectedAlbumTask);
}

void ImageQualitySorter::slotAffectedAlbumsFinished()
{
    if (d->allPicturesPath.isEmpty())
    {
        slotDone();

        return;
    }

    setTotalItems(d->allPicturesPath.count());

    d->thread->sortByImageQuality(d->allPicturesPath, d->quality);
    d->thread->start();
}

void ImageQualitySorter::calculateAffectedAlbums()
{
    if (d->quality.albums.isEmpty())
    {
        d->quality.albums = AlbumManager::instance()->allPAlbums();
    }

    // Get all item in DB which do not have any Pick Label assigned.

    QStringList dirty = CoreDbAccess().db()->getItemsURLsWithTag(TagsCache::instance()->tagForPickLabel(NoPickLabel));

    // Get all digiKam albums collection pictures path, depending of d->rebuildAll flag.

    for (AlbumList::ConstIterator it = d->quality.albums.constBegin() ;
         !canceled() && (it != d->quality.albums.constEnd()) ; ++it)
    {
        QStringList aPaths;

        if      ((*it)->type() == Album::PHYSICAL)
        {
            const auto paths = CoreDbAccess().db()->getItemURLsInAlbum((*it)->id());

            for (const QString& path : paths)
            {
                if (!aPaths.contains(path))
                {
                    aPaths << path;
                }
            }
        }
        else if ((*it)->type() == Album::TAG)
        {
            const auto paths = CoreDbAccess().db()->getItemURLsInTag((*it)->id());

            for (const QString& path : paths)
            {
                if (!aPaths.contains(path))
                {
                    aPaths << path;
                }
            }
        }

        if (d->quality.scanMode == ImageQualitySettings::NonAssignedItems)
        {
            for (const QString& path : std::as_const(aPaths))
            {
                if (dirty.contains(path))
                {
                    d->allPicturesPath += path;
                }
            }
        }
        else  // AllItems
        {
            d->allPicturesPath += aPaths;
        }
    }
}

void ImageQualitySorter::slotAdvance(const ItemInfo& inf, const QImage& img, int pick)
{
    uint adv = 0;

    if ((adv = checkProgressNeeded()) == 0)
    {
        return;
    }

    QString album = CollectionManager::instance()->albumRootLabel(inf.albumRootId());

    QString lbl = i18n("Image Quality: %1\n", inf.name());
    lbl.append(i18n("Album: %1\n", album + inf.relativePath()));
    lbl.append(i18n("Pick Label: %1", PickLabelWidget::labelPickName((PickLabel)pick)));
    setLabel(lbl);

    setThumbnail(QIcon(QPixmap::fromImage(img)));
    advance(adv);
}

void ImageQualitySorter::slotDone()
{
    setThumbnail(QIcon::fromTheme(QLatin1String("flag-green")).pixmap(48));

    QString lbl;

    if (totalItems() > 1)
    {
        lbl.append(i18n("Items scanned for quality: %1", totalItems()));
    }
    else
    {
        lbl.append(i18n("Item scanned for quality: %1", totalItems()));
    }

    setLabel(lbl);

    MaintenanceTool::slotDone();
}

} // namespace Digikam

#include "moc_imagequalitysorter.cpp"
