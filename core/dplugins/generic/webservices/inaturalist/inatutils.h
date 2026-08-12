/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-03-20
 * Description : a tool to export images to iNaturalist web service
 *
 * SPDX-FileCopyrightText: 2021      by Joerg Lohse <joergmlpts at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QLocale>
#include <QPair>
#include <QHttpMultiPart>

namespace DigikamGenericINatPlugin
{

extern const QLocale locale;
extern const bool    isEnglish;

/**
 * conversion factors for the US, one meter in feet and miles
 */
extern const double  meterInFeet;
extern const double  meterInMiles;

/**
 * multi-part message, used to upload pictures
 */
typedef QPair<QString, QString> Parameter;
extern QHttpMultiPart* getMultiPart(const QList<Parameter>& parameters,
                                    const QString& imageName,
                                    const QString& imageNameArg,
                                    const QString& imagePath);

/**
 * distance in meters between two geolocations
 */
extern double  distanceBetween(double latitude1, double longitude1,
                               double latitude2, double longitude2);

/**
 * geolocation in local language
 */
extern QString localizedLocation(double latitude, double longitude,
                                 int precision);

/**
 * taxonomic rank (e.g. kingdom, order, genus, species) in local language
 */
extern QString localizedTaxonomicRank(const QString& rank);

/**
 * distance in local language; for US returns feet or miles instead of meters
 */
extern QString localizedDistance(double distMeters, char format, int precision);

/**
 * time difference (e.g. "5 hours, 7 minutes") in local language
 */
extern QString localizedTimeDifference(quint64 diffSeconds);

} // namespace DigikamGenericINatPlugin
