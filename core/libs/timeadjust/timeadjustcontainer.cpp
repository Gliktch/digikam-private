/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-04-19
 * Description : time adjust settings container.
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "timeadjustcontainer.h"

// Qt includes

#include <QRegularExpression>

// Local includes

#include "digikam_globals.h"

namespace Digikam
{

bool TimeAdjustContainer::atLeastOneUpdateToProcess() const
{
    return (
            updFileModDate ||
            updEXIFModDate ||
            updEXIFOriDate ||
            updEXIFDigDate ||
            updEXIFThmDate ||
            updIPTCDate    ||
            updXMPVideo    ||
            updXMPDate
           );
}

QDateTime TimeAdjustContainer::calculateAdjustedDate(const QDateTime& originalTime, int index)
{
    int sign          = 0;
    QDateTime newTime = originalTime;

    int seconds       = adjustmentTime.time().second();
    seconds          += 60*adjustmentTime.time().minute();
    seconds          += 60*60*adjustmentTime.time().hour();
    seconds          += 24*60*60*adjustmentDays;

    switch (adjustmentType)
    {
        case ADDVALUE:
        {
            sign = 1;
            break;
        }

        case SUBVALUE:
        {
            sign = -1;
            break;
        }

        case INTERVAL:
        {
            newTime = newTime.addSecs(index * seconds);

            return newTime;
        }

        default: // COPYVALUE
        {
            return originalTime;
        }
    }

    newTime = newTime.addSecs(sign * seconds);

    return newTime;
}

QDateTime TimeAdjustContainer::getDateTimeFromString(const QString& dateStr) const
{
    if (dateStr.isEmpty())
    {
        return QDateTime();
    }

    QStringList regExpStrings;

    // Do not change the order of the list.

    regExpStrings << QLatin1String("(.*?)([0-9]{4}[-_:\\/ ]?[0-9]{2}[-_:\\/ ]?[0-9]{2})"
                                   "(.+)?([0-9]{2}[hH][0-9]{2})(.+)?");
    regExpStrings << QLatin1String("(.*?)([0-9]{8}[-_:\\/ ]?[0-9]{6}[-_:\\/ ]?[0-9]{3})(.+)?");
    regExpStrings << QLatin1String("(.*?)([0-9]{8}[T-_ ][0-9]{6})(.+)?");
    regExpStrings << QLatin1String("(.*?)([0-9]{4}[-_:\\/ ]?[0-9]{2}[-_:\\/ ]?[0-9]{2})"
                                   "(.+)?([0-9]{2}[-_:.]?[0-9]{2}[-_:.]?[0-9]{2}"
                                   "[-_:.]?[0-9]{3})(.+)?");
    regExpStrings << QLatin1String("(.*?)([0-9]{4}[-_:\\/ ]?[0-9]{2}[-_:\\/ ]?[0-9]{2})"
                                   "(.+)?([0-9]{2}[-_:.]?[0-9]{2}[-_:.]?[0-9]{2})(.+)?");
    regExpStrings << QLatin1String("(.*?)([0-9]{2}[-_:\\/.]?[0-9]{2}[-_:\\/.]?[0-9]{4})"
                                   "(.+)?([0-9]{2}[-_:.]?[0-9]{2}[-_:.]?[0-9]{2})(.+)?");
    regExpStrings << QLatin1String("(.*?)([0-9]{2}[-_:\\/.]?[0-9]{2}[-_:\\/.]?[0-9]{4})"
                                   "(.+)?([0-9]{2}[-_:.]?[0-9]{2})(.+)?");
    regExpStrings << QLatin1String("(.*?)([0-9]{2}-[0-9]{2}-[0-9]{2})"
                                   "[ -]?([0-9]{2}[0-9]{2})(.+)?");
    regExpStrings << QLatin1String("(.*?)([0-9]{4}[-_:\\/ ]?[0-9]{2}[-_:\\/ ]?[0-9]{2})(.+)?");
    regExpStrings << QLatin1String("(.*?)([0-9]{2}[-_:\\/.]?[0-9]{2}[-_:\\/.]?[0-9]{4})(.+)?");
    regExpStrings << QLatin1String("(.*?)([0-9]{2}[0-9]{2}[0-9]{4})([0-9]{3})(.+)?");
    regExpStrings << QLatin1String("(.*?)([0-9]{2}_[0-9]{2} [0-9]{2})(.+)?");

    QList <QPair<QString, QString> > formatStrings;
    formatStrings << qMakePair(QLatin1String("yyyyMMddhhmm"),      QString());
    formatStrings << qMakePair(QLatin1String("yyyyMMddhhmmsszzz"), QString());
    formatStrings << qMakePair(QLatin1String("yyyyMMddhhmmss"),    QString());
    formatStrings << qMakePair(QLatin1String("yyyyMMddhhmmsszzz"), QString());
    formatStrings << qMakePair(QLatin1String("yyyyMMddhhmmss"),    QString());
    formatStrings << qMakePair(QLatin1String("ddMMyyyyhhmmss"),    QLatin1String("MMddyyyyhhmmss"));
    formatStrings << qMakePair(QLatin1String("ddMMyyyyhhmm"),      QLatin1String("MMddyyyyhhmm"));
    formatStrings << qMakePair(QLatin1String("ddMMyyhhmm"),        QString());
    formatStrings << qMakePair(QLatin1String("yyyyMMdd"),          QString());
    formatStrings << qMakePair(QLatin1String("ddMMyyyy"),          QLatin1String("MMddyyyy"));
    formatStrings << qMakePair(QLatin1String("ddMMyyyy"),          QLatin1String("MMddyyyy"));
    formatStrings << qMakePair(QLatin1String("ddMM yy"),           QString());

    QDateTime dateTime;

    for (int index = 0 ; index < regExpStrings.count() ; ++index)
    {
        QRegularExpression dateRegExp(QRegularExpression::anchoredPattern(regExpStrings.at(index)));
        QRegularExpressionMatch match = dateRegExp.match(dateStr);

        if (match.hasMatch())
        {
            QString dateString   = match.captured(2);
            QString format       = formatStrings.at(index).first;
            QString secondFormat = formatStrings.at(index).second;

            if (format.contains(QLatin1String("hhmm")))
            {
                dateString.append(match.captured(4));
            }

            dateString.remove(QLatin1Char(' '));
            dateString.remove(QLatin1Char('-'));
            dateString.remove(QLatin1Char('_'));
            dateString.remove(QLatin1Char(':'));
            dateString.remove(QLatin1Char('.'));
            dateString.remove(QLatin1Char('/'));
            dateString.remove(QLatin1Char('T'));
            dateString.remove(QLatin1Char('h'));
            dateString.remove(QLatin1Char('H'));

            dateTime = asDateTimeUTC(QDateTime::fromString(dateString, format));

            if (!dateTime.isValid() && !secondFormat.isEmpty())
            {
                format   = secondFormat;
                dateTime = asDateTimeUTC(QDateTime::fromString(dateString, format));
            }

            if (dateTime.isValid() && (format.count(QLatin1Char('y')) == 2))
            {
                if (dateTime.date().year() < 1970)
                {
                    dateTime.setDate(dateTime.date().addYears(100));
                }
            }

            if (!dateTime.isValid()             ||
                (dateTime.date().year() < 1800) ||
                (dateTime.date().year() > 2200))
            {
                dateTime = QDateTime();
            }
            else
            {
                break;
            }
        }
    }

    return dateTime;
}

QMap<QString, bool> TimeAdjustContainer::getDateTimeTagsMap() const
{
    QMap<QString, bool> tagsMap;

    tagsMap.insert(QLatin1String("Exif.Image.DateTime"),           updEXIFModDate);
    tagsMap.insert(QLatin1String("Exif.Photo.DateTimeOriginal"),   updEXIFOriDate);
    tagsMap.insert(QLatin1String("Exif.Photo.DateTimeDigitized"),  updEXIFDigDate);
    tagsMap.insert(QLatin1String("Exif.Image.PreviewDateTime"),    updEXIFThmDate);

    tagsMap.insert(QLatin1String("Iptc.Application2.DateCreated"), updIPTCDate);
    tagsMap.insert(QLatin1String("Iptc.Application2.TimeCreated"), updIPTCDate);

    tagsMap.insert(QLatin1String("Xmp.exif.DateTimeOriginal"),     updXMPDate);
    tagsMap.insert(QLatin1String("Xmp.exif.DateTimeDigitized"),    updXMPDate);
    tagsMap.insert(QLatin1String("Xmp.photoshop.DateCreated"),     updXMPDate);
    tagsMap.insert(QLatin1String("Xmp.xmp.MetadataDate"),          updXMPDate);
    tagsMap.insert(QLatin1String("Xmp.xmp.CreateDate"),            updXMPDate);
    tagsMap.insert(QLatin1String("Xmp.xmp.ModifyDate"),            updXMPDate);
    tagsMap.insert(QLatin1String("Xmp.tiff.DateTime"),             updXMPDate);

    tagsMap.insert(QLatin1String("Xmp.video.DateTimeOriginal"),    updXMPVideo);
    tagsMap.insert(QLatin1String("Xmp.video.DateTimeDigitized"),   updXMPVideo);
    tagsMap.insert(QLatin1String("Xmp.video.ModificationDate"),    updXMPVideo);
    tagsMap.insert(QLatin1String("Xmp.video.DateUTC"),             updXMPVideo);

    tagsMap.insert(QLatin1String("Xmp.xmpDM.shotDate"),            updXMPVideo);

    return tagsMap;
}

} // namespace Digikam
