/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-09-12
 * Description : Metadata info containers
 *
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMetaType>
#include <QDebug>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT IptcCoreLocationInfo
{
public:

    bool operator==(const IptcCoreLocationInfo& t) const;
    bool isEmpty()                                 const;
    bool isNull()                                  const;
    void merge(const IptcCoreLocationInfo& t, bool merge);

public:

    QString country;
    QString countryCode;
    QString provinceState;
    QString city;
    QString location;
};

//! qDebug() stream operator. Writes property @a inf to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const IptcCoreLocationInfo& inf);

// ---------------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT IptcCoreContactInfo
{
public:

    bool operator==(const IptcCoreContactInfo& t) const;
    bool isEmpty()                                const;
    bool isNull()                                 const;
    void merge(const IptcCoreContactInfo& t, bool merge);

public:

    QString city;
    QString country;
    QString address;
    QString postalCode;
    QString provinceState;
    QString email;
    QString phone;
    QString webUrl;
};

//! qDebug() stream operator. Writes property @a inf to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const IptcCoreContactInfo& inf);

// ---------------------------------------------------------------------------------------------------

namespace MetadataInfo
{

enum Field
{
    Comment,                          ///< String (one of the following three values)
    CommentJfif,                      ///< String
    CommentExif,                      ///< String
    CommentIptc,                      ///< String (see also IptcCoreDescription)
    Description,                      ///< Map language -> String
    Title,                            ///< Map language -> String
    Headline,                         ///< String
    DescriptionWriter,                ///< String

    Keywords,                         ///< StringList

    Rating,                           ///< Int
    CreationDate,                     ///< DateTime
    DigitizationDate,                 ///< DateTime
    Orientation,                      ///< Int, enum from libMetaEngine

    Make,                             ///< String
    Model,                            ///< String
    Lens,                             ///< String
    Aperture,                         ///< Double, FNumber
    FocalLength,                      ///< Double, mm
    FocalLengthIn35mm,                ///< Double, mm
    ExposureTime,                     ///< Double, s
    ExposureProgram,                  ///< Int, enum from Exif
    ExposureMode,                     ///< Int, enum from Exif
    Sensitivity,                      ///< Int, ISO sensitivity
    FlashMode,                        ///< Int, bit mask from Exif
    WhiteBalance,                     ///< Int, enum from Exif
    WhiteBalanceColorTemperature,     ///< double, color temperature in K
    MeteringMode,                     ///< Int, enum from Exif
    SubjectDistance,                  ///< double, m
    SubjectDistanceCategory,          ///< int, enum from Exif

    Latitude,                         ///< String (as XMP GPSCoordinate)
    LatitudeNumber,                   ///< double, degrees
    Longitude,                        ///< String (as XMP GPSCoordinate)
    LongitudeNumber,                  ///< double, degrees
    Altitude,                         ///< double, m
    PositionOrientation,              ///< double, ?
    PositionTilt,                     ///< double, ?
    PositionRoll,                     ///< double, ?
    PositionAccuracy,                 ///< double, m
    PositionDescription,              ///< String

    IptcCoreCopyrightNotice,          ///< Map language -> String
    IptcCoreCreator,                  ///< List of type String
    IptcCoreProvider,                 ///< String
    IptcCoreRightsUsageTerms,         ///< Map language -> String
    IptcCoreSource,                   ///< String

    IptcCoreCreatorJobTitle,          ///< String
    IptcCoreInstructions,             ///< String

    IptcCoreLocationInfo,             ///< object of IptcCoreLocation, including:
    IptcCoreCountryCode,              ///< String
    IptcCoreCountry,                  ///< String
    IptcCoreCity,                     ///< String
    IptcCoreLocation,                 ///< String
    IptcCoreProvinceState,            ///< String

    IptcCoreIntellectualGenre,        ///< String
    IptcCoreJobID,                    ///< String
    IptcCoreScene,                    ///< List of type String
    IptcCoreSubjectCode,              ///< List of type String

    IptcCoreContactInfo,              ///< object of IptcCoreContactInfo, including:
    IptcCoreContactInfoCity,          ///< String
    IptcCoreContactInfoCountry,       ///< String
    IptcCoreContactInfoAddress,       ///< String
    IptcCoreContactInfoPostalCode,    ///< String
    IptcCoreContactInfoProvinceState, ///< String
    IptcCoreContactInfoEmail,         ///< String
    IptcCoreContactInfoPhone,         ///< String
    IptcCoreContactInfoWebUrl,        ///< String

    Faces,                            ///< QMap<QString, QVariant>

    // Description, DescriptionWriter, Headline, Title: see above
    // DateCreated: see above, CreationDate
    // Keywords: see above, Keywords
    // not supported: CreatorContactInfo

    // Dublin Core: Description, Title, Subject (keywords) see above

    AspectRatio,                      ///< String
    AudioBitRate,                     ///< String
    AudioChannelType,                 ///< String
    AudioCodec,                       ///< String
    Duration,                         ///< String
    FrameRate,                        ///< String
    VideoCodec,                       ///< String
    VideoBitDepth,                    ///< String
    VideoHeight,                      ///< String
    VideoWidth,                       ///< String
    VideoColorSpace                   ///< String
};

} // namespace MetadataInfo

typedef QList<MetadataInfo::Field> MetadataFields;

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::IptcCoreContactInfo)
Q_DECLARE_METATYPE(Digikam::IptcCoreLocationInfo)
