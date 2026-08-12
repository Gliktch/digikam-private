/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-05-29
 * Description : a combobox with time zones.
 *
 * SPDX-FileCopyrightText: 2015      by Maik Qualmann <metzpinguin at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "timezonecombobox.h"

namespace Digikam
{

TimeZoneComboBox::TimeZoneComboBox(QWidget* const parent)
    : QComboBox(parent)
{
    QStringList timeZones;
    timeZones << QLatin1String("")       << QLatin1String("-12:00")
              << QLatin1String("-11:00") << QLatin1String("-10:00")
              << QLatin1String("-09:30") << QLatin1String("-09:00")
              << QLatin1String("-08:00") << QLatin1String("-07:00")
              << QLatin1String("-06:00") << QLatin1String("-05:00")
              << QLatin1String("-04:30") << QLatin1String("-04:00")
              << QLatin1String("-03:30") << QLatin1String("-03:00")
              << QLatin1String("-02:00") << QLatin1String("-01:00")
              << QLatin1String("+00:00") << QLatin1String("+01:00")
              << QLatin1String("+02:00") << QLatin1String("+03:00")
              << QLatin1String("+03:30") << QLatin1String("+04:00")
              << QLatin1String("+04:30") << QLatin1String("+05:00")
              << QLatin1String("+05:30") << QLatin1String("+05:45")
              << QLatin1String("+06:00") << QLatin1String("+06:30")
              << QLatin1String("+07:00") << QLatin1String("+08:00")
              << QLatin1String("+09:00") << QLatin1String("+09:30")
              << QLatin1String("+10:00") << QLatin1String("+10:30")
              << QLatin1String("+11:00") << QLatin1String("+11:30")
              << QLatin1String("+12:00") << QLatin1String("+12:45")
              << QLatin1String("+13:00") << QLatin1String("+13:45")
              << QLatin1String("+14:00");

    addItems(timeZones);
}

void TimeZoneComboBox::setToUTC()
{
    setCurrentIndex(findText(QLatin1String("+00:00")));
}

void TimeZoneComboBox::setTimeZone(const QString& timeStr)
{
    if (timeStr.length() < 6)
    {
        setCurrentIndex(0);
        return;
    }

    QString timeZone = timeStr.right(6);

    if      (timeZone.endsWith(QLatin1Char('Z')))
    {
        setToUTC();
    }
    else if (
              (
               timeZone.startsWith(QLatin1Char('+'))  ||
               timeZone.startsWith(QLatin1Char('-'))
              ) &&
              (timeZone.mid(3, 1) == QLatin1String(":"))
            )
    {
        setCurrentIndex(findText(timeZone));
    }
    else
    {
        setCurrentIndex(0);
    }
}

QString TimeZoneComboBox::getTimeZone() const
{
    return currentText();
}

int TimeZoneComboBox::timeZoneOffset() const
{
    QString tz = currentText();

    if (tz.isEmpty())
    {
        return 0;
    }

    int hh     = QString(QString(tz[1]) + QString(tz[2])).toInt();
    int mm     = QString(QString(tz[4]) + QString(tz[5])).toInt();

    int offset = hh*3600 + mm*60;

    if (tz[0] == QLatin1Char('-'))
    {
        offset = (-1) * offset;
    }

    return offset;
}

} // namespace Digikam

#include "moc_timezonecombobox.cpp"
