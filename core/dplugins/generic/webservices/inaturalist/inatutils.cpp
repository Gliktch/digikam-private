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

#include "inatutils.h"

// Qt includes

#include <QHttpPart>
#include <QFile>
#include <QFileInfo>
#include <QtMath>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericINatPlugin
{

// Constants

const QLocale locale      = QLocale();
const bool    isEnglish   = (locale.language() == QLocale::English)     ||
                            (locale.language() == QLocale::C)           ||
                            (locale.language() == QLocale::AnyLanguage);

const double meterInFeet  = 3.28084;
const double meterInMiles = 0.00062137;

// Functions

QHttpMultiPart* getMultiPart(const QList<Parameter>& parameters,
                             const QString& imageName,
                             const QString& imageNameArg,
                             const QString& imagePath)
{
    static const QString paramForm = QLatin1String("form-data; name=\"%1\"");
    static const QString imageForm = QLatin1String("form-data; name=\"%1\"; "
                                                   "filename=\"%2\"");

    QHttpMultiPart* const result   = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    for (const auto& param : parameters)
    {
        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant(paramForm.arg(param.first)));
        part.setBody(param.second.toUtf8());
        result->append(part);
    }

    QHttpPart imagePart;
    QFileInfo fileInfo(imagePath);
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader,
                        QVariant(QString(QLatin1String("image/%1")).
                                 arg(fileInfo.suffix().toLower())));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant(imageForm.arg(imageName, imageNameArg)));
    QFile* const file = new QFile(imagePath);

    if (!file->open(QIODevice::ReadOnly))
    {
        qCWarning(DIGIKAM_WEBSERVICES_LOG) << "Cannot open file to read" << imagePath;
    }

    imagePart.setBodyDevice(file);
    file->setParent(result);
    result->append(imagePart);

    return result;
}

/**
 * converts degree to radian
 */
static inline double deg2rad(double deg)
{
    return (deg * M_PI / 180);
}

/**
 * returns distance in meters between two coordinates, Haversine formula
 */
double distanceBetween(double latitude1, double longitude1,
                       double latitude2, double longitude2)
{
    static const double earthRadiusInMeters = 6.371e6;

    double lat1r = deg2rad(latitude1);
    double lon1r = deg2rad(longitude1);
    double lat2r = deg2rad(latitude2);
    double lon2r = deg2rad(longitude2);
    double u     = sin((lat2r - lat1r) / 2);
    double v     = sin((lon2r - lon1r) / 2);

    return (2.0 * earthRadiusInMeters *
            asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v)));
}

QString localizedTaxonomicRank(const QString& rank)
{
    if      (rank == QLatin1String("kingdom"))
    {
        return i18nc("taxonomic rank", "kingdom");
    }
    else if (rank == QLatin1String("subkingdom"))
    {
        return i18nc("taxonomic rank", "subkingdom");
    }
    else if (rank == QLatin1String("phylum"))
    {
        return i18nc("taxonomic rank", "phylum");
    }
    else if (rank == QLatin1String("subphylum"))
    {
        return i18nc("taxonomic rank", "subphylum");
    }
    else if (rank == QLatin1String("superorder"))
    {
        return i18nc("taxonomic rank", "superorder");
    }
    else if (rank == QLatin1String("order"))
    {
        return i18nc("taxonomic rank", "order");
    }
    else if (rank == QLatin1String("suborder"))
    {
        return i18nc("taxonomic rank", "suborder");
    }
    else if (rank == QLatin1String("infraorder"))
    {
        return i18nc("taxonomic rank", "infraorder");
    }
    else if (rank == QLatin1String("parvorder"))
    {
        return i18nc("taxonomic rank", "parvorder");
    }
    else if (rank == QLatin1String("zoosection"))
    {
        return i18nc("taxonomic rank", "zoosection");
    }
    else if (rank == QLatin1String("zoosubsection"))
    {
        return i18nc("taxonomic rank", "zoosubsection");
    }
    else if (rank == QLatin1String("superfamily"))
    {
        return i18nc("taxonomic rank", "superfamily");
    }
    else if (rank == QLatin1String("epifamily"))
    {
        return i18nc("taxonomic rank", "epifamily");
    }
    else if (rank == QLatin1String("family"))
    {
        return i18nc("taxonomic rank", "family");
    }
    else if (rank == QLatin1String("subfamily"))
    {
        return i18nc("taxonomic rank", "subfamily");
    }
    else if (rank == QLatin1String("supertribe"))
    {
        return i18nc("taxonomic rank", "supertribe");
    }
    else if (rank == QLatin1String("tribe"))
    {
        return i18nc("taxonomic rank", "tribe");
    }
    else if (rank == QLatin1String("subtribe"))
    {
        return i18nc("taxonomic rank", "subtribe");
    }
    else if (rank == QLatin1String("genus"))
    {
        return i18nc("taxonomic rank", "genus");
    }
    else if (rank == QLatin1String("genushybrid"))
    {
        return i18nc("taxonomic rank", "genushybrid");
    }
    else if (rank == QLatin1String("subgenus"))
    {
        return i18nc("taxonomic rank", "subgenus");
    }
    else if (rank == QLatin1String("section"))
    {
        return i18nc("taxonomic rank", "section");
    }
    else if (rank == QLatin1String("subsection"))
    {
        return i18nc("taxonomic rank", "subsection");
    }
    else if (rank == QLatin1String("complex"))
    {
        return i18nc("taxonomic rank", "complex");
    }
    else if (rank == QLatin1String("species"))
    {
        return i18nc("taxonomic rank", "species");
    }
    else if (rank == QLatin1String("hybrid"))
    {
        return i18nc("taxonomic rank", "hybrid");
    }
    else if (rank == QLatin1String("subspecies"))
    {
        return i18nc("taxonomic rank", "subspecies");
    }
    else if (rank == QLatin1String("variety"))
    {
        return i18nc("taxonomic rank", "variety");
    }
    else if (rank == QLatin1String("form"))
    {
        return i18nc("taxonomic rank", "form");
    }
    else if (rank == QLatin1String("infrahybrid"))
    {
        return i18nc("taxonomic rank", "infrahybrid");
    }
    else
    {
        return rank;
    }
}

QString localizedLocation(double latitude, double longitude, int precision)
{
    return (
            locale.toString(latitude, 'f', precision) +
            QLatin1String(", ")                       +
            locale.toString(longitude, 'f', precision)
           );
}

QString localizedDistance(double distMeters, char format, int precision)
{
    if (locale.measurementSystem() == QLocale::ImperialUSSystem)
    {
        if (
            locale.toString(distMeters * meterInMiles, format, precision) ==
            locale.toString(0.0, format, precision)
           )
        {
            return locale.toString(distMeters * meterInFeet, format, precision) +
                   QLatin1String(" ft");
        }
        else
        {
            return locale.toString(distMeters * meterInMiles, format,
                                   precision) + QLatin1String(" mi");
        }
    }
    else
    {
        if (distMeters >= 1000.0)
        {
            return locale.toString(distMeters / 1000.0, format, precision) +
                   QLatin1String(" km");
        }
        else
        {
            QString one    = locale.toString(1.0, format, precision);
            QString result = locale.toString(distMeters, format, precision);

            return (result + QLatin1Char(' ') + ((result == one)
                                                 ? i18nc("distance", "meter")
                                                 : i18nc("distance", "meters")));
        }
    }
}

QString localizedTimeDifference(quint64 diffSeconds)
{
    QString result;

    quint64 hours = diffSeconds / 3600;

    if (hours)
    {
        diffSeconds %= 3600;
        result       = QString::number(hours) + QLatin1Char(' ') +
                       (hours == 1 ? i18nc("time", "hour")
                                   : i18nc("time", "hours"));
    }

    quint64 minutes = diffSeconds / 60;

    if (minutes)
    {
        diffSeconds %= 60;

        if (!result.isEmpty())
        {
            result += QLatin1String(", ");
        }

        result += QString::number(minutes) + QLatin1Char(' ') +
                  (minutes == 1 ? i18nc("time", "minute")
                                : i18nc("time", "minutes"));
    }

    if (diffSeconds || result.isEmpty())
    {
        if (!result.isEmpty())
        {
            result += QLatin1String(", ");
        }

        result += QString::number(diffSeconds) + QLatin1Char(' ') +
                  (diffSeconds == 1 ? i18nc("time", "second")
                                    : i18nc("time", "seconds"));
    }

    return result;
}

} // namespace DigikamGenericINatPlugin
