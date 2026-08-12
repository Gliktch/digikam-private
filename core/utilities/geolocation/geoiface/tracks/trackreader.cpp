/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-19
 * Description : Track file reader
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "trackreader.h"

// Qt includes

#include <QFile>
#include <QXmlStreamAttributes>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "actionthreadbase.h"

namespace Digikam
{

class Q_DECL_HIDDEN TrackReader::Private
{
public:

    Private() = default;

    TrackReadResult* fileData = nullptr;
};

TrackReader::TrackReader(TrackReadResult* const dataTarget)
    : QObject(nullptr),
      d      (new Private)
{
    d->fileData = dataTarget;
}

TrackReader::~TrackReader()
{
}

QDateTime TrackReader::parseTime(const QString& tstring)
{
    QString timeString = tstring;

    if (timeString.isEmpty())
    {
        return QDateTime();
    }

    // we want to be able to parse these formats:
    // "2010-01-14T09:26:02.287-02:00" <-- here we have to cut off the -02:00 and replace it with 'Z'
    // "2010-01-14T09:26:02.287+02:00" <-- here we have to cut off the +02:00 and replace it with 'Z'
    // "2009-03-11T13:39:55.622Z"

    const int timeStringLength      = timeString.length();
    const int timeZoneSignPosition  = timeStringLength-6;

    // does the string contain a timezone offset?

    int timeZoneOffsetSeconds       = 0;
    const int timeZonePlusPosition  = timeString.lastIndexOf(QLatin1Char('+'));
    const int timeZoneMinusPosition = timeString.lastIndexOf(QLatin1Char('-'));

    if (
        (timeZonePlusPosition  == timeZoneSignPosition)   ||
        (timeZoneMinusPosition == timeZoneSignPosition)
       )
    {
        const int timeZoneSign       = (timeZonePlusPosition == timeZoneSignPosition) ? +1 : -1;

        // cut off the last digits:

        const QString timeZoneString = timeString.right(6);
        timeString.chop(6);
        timeString                  += QLatin1Char('Z');

        // determine the time zone offset:

        bool okayHour                = false;
        bool okayMinute              = false;
        const int hourOffset         = timeZoneString.mid(1, 2).toInt(&okayHour);
        const int minuteOffset       = timeZoneString.mid(4, 2).toInt(&okayMinute);

        if (okayHour&&okayMinute)
        {
            timeZoneOffsetSeconds  = hourOffset*3600 + minuteOffset*60;
            timeZoneOffsetSeconds *= timeZoneSign;
        }
    }

    QDateTime theTime = QDateTime::fromString(timeString, Qt::ISODate);
    theTime           = theTime.addSecs(-timeZoneOffsetSeconds);

    return theTime;
}

void TrackReader::parseTrack(QXmlStreamReader& xml)
{
    // check that really getting a track.

    if (
        (xml.tokenType() != QXmlStreamReader::StartElement) &&
        (xml.name()      == QLatin1String("trkpt"))
       )
    {
        return;
    }

    TrackManager::TrackPoint currentDataPoint;

    // get first the attributes for track points.

    QXmlStreamAttributes attributes = xml.attributes();

    // check that track has lat or lon attribute.

    if (
        attributes.hasAttribute(QLatin1String("lat")) &&
        attributes.hasAttribute(QLatin1String("lon"))
       )
    {
        currentDataPoint.coordinates.setLatLon(attributes.value(QLatin1String("lat")).toDouble(),
                                               attributes.value(QLatin1String("lon")).toDouble());
    }

    // Next element...

    xml.readNext();

    while (
           !(
             (xml.tokenType() == QXmlStreamReader::EndElement) &&
             (xml.name()      == QLatin1String("trkpt"))
            )      &&
           !xml.hasError()
          )
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            QString eText = xml.readElementText(QXmlStreamReader::IncludeChildElements);

            if      (xml.name() == QLatin1String("time"))
            {
                currentDataPoint.dateTime = parseTime(eText.trimmed());
            }
            else if (xml.name() == QLatin1String("sat"))
            {
                bool okay       = false;
                int nSatellites = eText.toInt(&okay);

                if (okay && (nSatellites >= 0))
                {
                    currentDataPoint.nSatellites = nSatellites;
                }
            }
            else if (xml.name() == QLatin1String("hdop"))
            {
                bool okay  = false;
                qreal hDop = eText.toDouble(&okay);

                if (okay)
                {
                    currentDataPoint.hDop = hDop;
                }
            }
            else if (xml.name() == QLatin1String("pdop"))
            {
                bool okay  = false;
                qreal pDop = eText.toDouble(&okay);

                if (okay)
                {
                    currentDataPoint.pDop = pDop;
                }
            }
            else if (xml.name() == QLatin1String("fix"))
            {
                if      (eText == QLatin1String("2d"))
                {
                    currentDataPoint.fixType = 2;
                }
                else if (eText == QLatin1String("3d"))
                {
                    currentDataPoint.fixType = 3;
                }
            }
            else if (xml.name() == QLatin1String("ele"))
            {
                bool haveAltitude = false;
                const qreal alt   = eText.toDouble(&haveAltitude);

                if (haveAltitude)
                {
                    currentDataPoint.coordinates.setAlt(alt);
                }
            }
            else if (xml.name() == QLatin1String("speed"))
            {
                bool haveSpeed    = false;
                const qreal speed = eText.toDouble(&haveSpeed);

                if (haveSpeed)
                {
                    currentDataPoint.speed = speed;
                }
            }
        }

        // ...and next...

        xml.readNext();
    }

    if (currentDataPoint.dateTime.isValid() && currentDataPoint.coordinates.hasCoordinates())
    {
        d->fileData->track.points << currentDataPoint;
    }
}

TrackReader::TrackReadResult TrackReader::loadTrackFile(const QUrl& url)
{
    ActionThreadBase::setCurrentThreadName(QLatin1String(__FUNCTION__));       // To customize thread name

    // TODO: store some kind of error message

    TrackReadResult parsedData;
    TrackReader trackReader(&parsedData);
    parsedData.track.url = url;
    parsedData.isValid   = false;

    QFile file(url.toLocalFile());

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        parsedData.loadError = i18n("Could not open: %1", file.errorString());

        return parsedData;
    }

    if (file.size() == 0)
    {
        parsedData.loadError = i18n("File is empty.");

        return parsedData;
    }

    QXmlStreamReader XmlReader(&file);

    while (!XmlReader.atEnd() && !XmlReader.hasError() )
    {
        QXmlStreamReader::TokenType token = XmlReader.readNext();

        if (token == QXmlStreamReader::StartElement)
        {
            if (XmlReader.name().toString() != QLatin1String("trkpt"))
            {
                continue;
            }
            else
            {
                trackReader.parseTrack(XmlReader);
            }
        }
    }

    // error not well formed XML file

    parsedData.isValid = !XmlReader.hasError() ;

    if (!parsedData.isValid)
    {
        if (XmlReader.error() == QXmlStreamReader::NotWellFormedError)
        {
            parsedData.loadError = i18n("Probably not a GPX file.");
        }

        return parsedData;
    }

    // error no track points

    parsedData.isValid = !parsedData.track.points.isEmpty();

    if (!parsedData.isValid)
    {
        parsedData.loadError = i18n("File is a GPX file, but no track points "
                                    "with valid timestamps were found.");

        return parsedData;
    }

    // The correlation algorithm relies on sorted data, therefore sort now

    std::sort(parsedData.track.points.begin(),
              parsedData.track.points.end(),
              TrackManager::TrackPoint::EarlierThan);

    return parsedData;
}

} // namespace Digikam

#include "moc_trackreader.cpp"
