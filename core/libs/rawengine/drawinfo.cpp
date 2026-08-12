/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-05-02
 * Description : RAW file identification information container
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Local includes

#include "drawinfo.h"

namespace Digikam
{

DRawInfo::DRawInfo()
{
    for (int x = 0 ; x < 3 ; ++x)
    {
        for (int y = 0 ; y < 4 ; ++y)
        {
            cameraColorMatrix1[x][y] = 0.0F;
            cameraColorMatrix2[x][y] = 0.0F;
            cameraXYZMatrix[y][x]    = 0.0F; ///< NOTE: see bug #253911 : [y][x] not [x][y]
        }
    }
}

QDebug operator<<(QDebug dbg, const DRawInfo& c)
{
    dbg.nospace() << "DRawInfo::sensitivity: "      << c.sensitivity      << ", ";
    dbg.nospace() << "DRawInfo::exposureTime: "     << c.exposureTime     << ", ";
    dbg.nospace() << "DRawInfo::aperture: "         << c.aperture         << ", ";
    dbg.nospace() << "DRawInfo::focalLength: "      << c.focalLength      << ", ";
    dbg.nospace() << "DRawInfo::pixelAspectRatio: " << c.pixelAspectRatio << ", ";
    dbg.nospace() << "DRawInfo::rawColors: "        << c.rawColors        << ", ";
    dbg.nospace() << "DRawInfo::rawImages: "        << c.rawImages        << ", ";
    dbg.nospace() << "DRawInfo::hasIccProfile: "    << c.hasIccProfile    << ", ";
    dbg.nospace() << "DRawInfo::isDecodable: "      << c.isDecodable      << ", ";
    dbg.nospace() << "DRawInfo::daylightMult: "     << c.daylightMult     << ", ";
    dbg.nospace() << "DRawInfo::cameraMult: "       << c.cameraMult       << ", ";
    dbg.nospace() << "DRawInfo::blackPoint: "       << c.blackPoint       << ", ";
    dbg.nospace() << "DRawInfo::whitePoint: "       << c.whitePoint       << ", ";
    dbg.nospace() << "DRawInfo::topMargin: "        << c.topMargin        << ", ";
    dbg.nospace() << "DRawInfo::leftMargin: "       << c.leftMargin       << ", ";
    dbg.nospace() << "DRawInfo::orientation: "      << c.orientation;

    return dbg.space();
}

} // namespace Digikam
