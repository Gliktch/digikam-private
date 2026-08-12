/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-01
 * Description : GPSSync common functions and structures
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include <gpscommon.h>

// Qt includes

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dmetadata.h"
#include "metaenginesettings.h"
#include "metaenginesettingscontainer.h"
#include "dmessagebox.h"

namespace Digikam
{

QString getUserAgentName()
{
    // Legacy user agent name. Do not change it.

    return QLatin1String("KIPI-Plugins GPSSync - kde-imaging@kde.org");
}

void coordinatesToClipboard(const GeoCoordinates& coordinates,
                            const QUrl& url,
                            const QString& title)
{
    const QString lat       = coordinates.latString();
    const QString lon       = coordinates.lonString();
    const bool haveAltitude = coordinates.hasAltitude();
    const QString altitude  = coordinates.altString();
    const QString nameToUse = title.isEmpty() ? url.toLocalFile() : title;

    /**
     * NOTE: importing this representation into Marble does not show anything,
     * but Merkaartor shows the point
     */
    const QString kmlCoordinatesString = haveAltitude ? QString::fromLatin1("%1,%2,%3").arg(lon).arg(lat).arg(altitude)
                                                      : QString::fromLatin1("%1,%2").arg(lon).arg(lat);

    const QString kmlRepresentation = QString::fromLatin1(
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n"        // krazy:exclude=insecurenet
      "<Document>\n"
      " <Placemark>\n"
      "   <name>%1</name>\n"
      "   <Point>\n"
      "     <coordinates>%2</coordinates>\n"
      "   </Point>\n"
      " </Placemark>\n"
      "</Document>\n"
      "</kml>\n"
      ).arg(nameToUse).arg(kmlCoordinatesString);

    /// importing this data into Marble and Merkaartor works
    const QString gpxElevationString = haveAltitude ? QString::fromLatin1("   <ele>%1</ele>\n").arg(altitude)
                                                    : QString();

    const QString gpxRepresentation  = QString::fromLatin1(
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n"
      "<gpx xmlns=\"http://www.topografix.com/GPX/1/1\" creator=\"trippy\" version=\"0.1\"\n"       // krazy:exclude=insecurenet
      " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"                                  // krazy:exclude=insecurenet
      " xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1"                                     // krazy:exclude=insecurenet
      " http://www.topografix.com/GPX/1/1/gpx.xsd\">\n"                                             // krazy:exclude=insecurenet
      "  <wpt lat=\"%1\" lon=\"%2\">\n"
      "%3"
/*
      "   <time></time>\n"
*/
      "   <name>%4</name>\n"
      "  </wpt>\n"
      "</gpx>\n"
      ).arg(lat).arg(lon).arg(gpxElevationString).arg(nameToUse);

    QMimeData* const myMimeData = new QMimeData();
    myMimeData->setText(coordinates.geoUrl());
    myMimeData->setData(QLatin1String("application/vnd.google-earth.kml+xml"), kmlRepresentation.toUtf8());
    myMimeData->setData(QLatin1String("application/gpx+xml"),                  gpxRepresentation.toUtf8());

    QClipboard* const clipboard = QApplication::clipboard();
    clipboard->setMimeData(myMimeData);
}

bool checkSidecarSettings()
{
    if (
        (MetaEngineSettings::instance()->settings().metadataWritingMode != DMetadata::WRITE_TO_FILE_ONLY) &&
        (!MetaEngineSettings::instance()->settings().useXMPSidecar4Reading)
       )
    {
        const int result = DMessageBox::showContinueCancel(QMessageBox::Warning,
                                                           QApplication::activeWindow(),
                                                           i18n("Warning: Sidecar settings"),
                                                           i18n("You have enabled writing to sidecar files for metadata storage in the host application,"
                                                                " but not for reading."
                                                                " This means that any metadata stored in the sidecar files will be overwritten here.\n"
                                                                "Please enable reading of sidecar files in the host application or continue at your own risk.")
                                                          );

        if (result != QMessageBox::Yes)
        {
            return false;
        }
    }

    return true;
}

} // namespace Digikam
