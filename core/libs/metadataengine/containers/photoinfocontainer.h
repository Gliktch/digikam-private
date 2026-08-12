/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-21
 * Description : photo information container
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QDateTime>
#include <QDebug>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT PhotoInfoContainer
{

public:

    PhotoInfoContainer()                                     = default;
    PhotoInfoContainer(const PhotoInfoContainer&)            = default;
    ~PhotoInfoContainer()                                    = default;

    PhotoInfoContainer& operator=(const PhotoInfoContainer&) = default;
    PhotoInfoContainer& operator=(PhotoInfoContainer&&)      = default;

    bool isEmpty()                               const;
    bool isNull()                                const;

    bool operator==(const PhotoInfoContainer& t) const;

public:

    QString   make;
    QString   model;
    QString   lens;
    QString   exposureTime;
    QString   exposureMode;
    QString   exposureProgram;
    QString   aperture;
    QString   focalLength;
    QString   focalLength35mm;
    QString   sensitivity;
    QString   flash;
    QString   whiteBalance;

    QDateTime dateTime;

    bool      hasCoordinates = false;  ///< true if GPS info are present
};

DIGIKAM_EXPORT QDataStream& operator<<(QDataStream& ds, const PhotoInfoContainer& info);
DIGIKAM_EXPORT QDataStream& operator>>(QDataStream& ds, PhotoInfoContainer& info);

//! qDebug() stream operator. Writes property @a t to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const PhotoInfoContainer& t);

} // namespace Digikam
