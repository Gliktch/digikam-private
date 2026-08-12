/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-31
 * Description : maintenance manager settings
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "maintenancesettings.h"

namespace Digikam
{

/**
 * qCDebug(DIGIKAM_MAINTENANCE_LOG) stream operator. Writes property @a s to the debug output in a nicely formatted way.
 */
QDebug operator<<(QDebug dbg, const MaintenanceSettings& s)
{
    dbg.nospace() << Qt::endl;
    dbg.nospace() << "wholeAlbums            : " << s.wholeAlbums                         << Qt::endl;
    dbg.nospace() << "wholeTags              : " << s.wholeTags                           << Qt::endl;
    dbg.nospace() << "Albums                 : " << s.albums.count()                      << Qt::endl;
    dbg.nospace() << "Tags                   : " << s.tags.count()                        << Qt::endl;
    dbg.nospace() << "useMutiCoreCPU         : " << s.useMutiCoreCPU                      << Qt::endl;
    dbg.nospace() << "newItems               : " << s.newItems                            << Qt::endl;
    dbg.nospace() << "thumbnails             : " << s.thumbnails                          << Qt::endl;
    dbg.nospace() << "scanThumbs             : " << s.scanThumbs                          << Qt::endl;
    dbg.nospace() << "fingerPrints           : " << s.fingerPrints                        << Qt::endl;
    dbg.nospace() << "scanFingerPrints       : " << s.scanFingerPrints                    << Qt::endl;
    dbg.nospace() << "duplicates             : " << s.duplicates                          << Qt::endl;
    dbg.nospace() << "minSimilarity          : " << s.minSimilarity                       << Qt::endl;
    dbg.nospace() << "maxSimilarity          : " << s.maxSimilarity                       << Qt::endl;
    dbg.nospace() << "duplicatesRestriction  : " << s.duplicatesRestriction               << Qt::endl;
    dbg.nospace() << "faceManagement         : " << s.faceManagement                      << Qt::endl;
    dbg.nospace() << "AutotagsAssignment     : " << s.autotagsAssignment                  << Qt::endl;
    dbg.nospace() << "qualitySort            : " << s.qualitySort                         << Qt::endl;
    dbg.nospace() << "qualitySettings        : " << s.qualitySettings                     << Qt::endl;
    dbg.nospace() << "metadataSync           : " << s.metadataSync                        << Qt::endl;
    dbg.nospace() << "syncDirection          : " << s.syncDirection                       << Qt::endl;
    dbg.nospace() << "databaseCleanup        : " << s.databaseCleanup                     << Qt::endl;
    dbg.nospace() << "cleanThumbDb           : " << s.cleanThumbDb                        << Qt::endl;
    dbg.nospace() << "cleanFacesDb           : " << s.cleanFacesDb                        << Qt::endl;
    dbg.nospace() << "cleanSimilarityDb      : " << s.cleanSimilarityDb                   << Qt::endl;
    dbg.nospace() << "shrinkDatabases        : " << s.shrinkDatabases                     << Qt::endl;

    return dbg.space();
}

} // namespace Digikam
