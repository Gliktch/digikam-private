/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-10-28
 * Description : scan item controller - private containers.
 *
 * SPDX-FileCopyrightText: 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "scancontroller_p.h"

namespace Digikam
{

SimpleCollectionScannerObserver::SimpleCollectionScannerObserver(bool* const var)
    : m_continue(var)
{
    *m_continue = true;
}

bool SimpleCollectionScannerObserver::continueQuery()
{
    return *m_continue;
}

// ------------------------------------------------------------------------------

QPixmap ScanController::Private::albumPixmap()
{
    if (albumPix.isNull())
    {
        albumPix = QIcon::fromTheme(QLatin1String("folder-pictures")).pixmap(32);
    }

    return albumPix;
}

QPixmap ScanController::Private::rootPixmap()
{
    if (rootPix.isNull())
    {
        rootPix = QIcon::fromTheme(QLatin1String("folder-open")).pixmap(32);
    }

    return rootPix;
}

QPixmap ScanController::Private::actionPixmap()
{
    if (actionPix.isNull())
    {
        actionPix = QIcon::fromTheme(QLatin1String("system-run")).pixmap(32);
    }

    return actionPix;
}

QPixmap ScanController::Private::errorPixmap()
{
    if (errorPix.isNull())
    {
        errorPix = QIcon::fromTheme(QLatin1String("dialog-error")).pixmap(32);
    }

    return errorPix;
}

QPixmap ScanController::Private::restartPixmap()
{
    if (errorPix.isNull())
    {
        errorPix = QIcon::fromTheme(QLatin1String("view-refresh")).pixmap(32);
    }

    return errorPix;
}

// --------------------------------------------------------------------------------------------

ScanControllerLoadingCacheFileWatch::ScanControllerLoadingCacheFileWatch()
{
    CoreDbWatch* const dbwatch = CoreDbAccess::databaseWatch();

    // we opt for a queued connection to make stuff a bit relaxed

    connect(dbwatch, SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotImageChanged(ImageChangeset)),
            Qt::QueuedConnection);
}

void ScanControllerLoadingCacheFileWatch::slotImageChanged(const ImageChangeset& changeset)
{
    const auto ids = changeset.ids();

    for (const qlonglong& imageId : ids)
    {
        DatabaseFields::Set changes = changeset.changes();

        if ((changes & DatabaseFields::ModificationDate) || (changes & DatabaseFields::Orientation))
        {
            ItemInfo info(imageId);
/*
            qCDebug(DIGIKAM_DATABASE_LOG) << imageId << info.filePath();
*/
            notifyFileChanged(info.filePath());
        }
    }
}

} // namespace Digikam

#include "moc_scancontroller_p.cpp"
