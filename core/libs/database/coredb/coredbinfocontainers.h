/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-04
 * Description : Container classes holding user presentable information
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QDateTime>

namespace Digikam
{

class ImageCommonContainer
{
public:

    ImageCommonContainer() = default;

    QString   fileName;
    QDateTime fileModificationDate;
    qint64    fileSize      = 0;

    int       rating        = -1;
    QDateTime creationDate;
    QDateTime digitizationDate;
    QString   orientation;
    int       width         = 0;
    int       height        = 0;
    QString   format;
    int       colorDepth    = 0; ///< bits per channel, 8/16
    QString   colorModel;
};

// ------------------------------------------------------------

class ImageMetadataContainer
{
public:

    ImageMetadataContainer() = default;

    bool allFieldsNull = true;

    QString make;
    QString model;
    QString lens;
    QString aperture;
    QString focalLength;
    QString focalLength35;
    QString exposureTime;
    QString exposureProgram;
    QString exposureMode;
    QString sensitivity;
    QString flashMode;
    QString whiteBalance;
    QString whiteBalanceColorTemperature;
    QString meteringMode;
    QString subjectDistance;
    QString subjectDistanceCategory;
};

// ------------------------------------------------------------

class VideoMetadataContainer
{
public:

    VideoMetadataContainer() = default;

    bool    allFieldsNull = true;

    QString aspectRatio;
    QString audioBitRate;
    QString audioChannelType;
    QString audioCodec;
    QString duration;
    QString frameRate;
    QString videoCodec;
};

} // namespace Digikam
