/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : calendar parameters.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2012      by Angelo Naselli <anaselli at linux dot it>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "calsettings.h"

// Local includes

#include "digikam_debug.h"


#ifdef HAVE_KCALENDAR

// KCalCore includes

#   if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

#       include <kcalendarcore/icalformat.h>
#       include <kcalendarcore/filestorage.h>
#       include <kcalendarcore/memorycalendar.h>

using namespace KCalendarCore;

#   else

#       include <kcalcore/icalformat.h>
#       include <kcalcore/filestorage.h>
#       include <kcalcore/memorycalendar.h>

using namespace KCalCore;

#   endif

    // Qt includes

#   include <QTimeZone>

#endif // HAVE_KCALENDAR

namespace DigikamGenericCalendarPlugin
{

class Q_DECL_HIDDEN CalSettings::Private
{
public:

    Private() = default;

    QMap<int, QUrl>  monthMap;
    QMap<QDate, Day> special;
};

QPointer<CalSettings> CalSettings::s_instance;

CalSettings::CalSettings(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    setImagePos(CalParams::Top);
}

CalSettings::~CalSettings()
{
    delete d;
}

CalSettings* CalSettings::instance(QObject* const parent)
{
    if (s_instance.isNull())
    {
        s_instance = new CalSettings(parent);
    }

    return s_instance;
}

void CalSettings::setYear(int year)
{
    params.year = year;

    Q_EMIT settingsChanged();
}

int CalSettings::year() const
{
    return params.year;
}

void CalSettings::setImage(int month, const QUrl& path)
{
    d->monthMap.insert(month, path);
}

QUrl CalSettings::image(int month) const
{
    return (d->monthMap.contains(month) ? d->monthMap[month] : QUrl());
}

void CalSettings::setPaperSize(const QString& paperSize)
{
    if      (paperSize == QLatin1String("A4"))
    {
        params.paperWidth  = 210;
        params.paperHeight = 297;
        params.pageSize    = QPageSize::A4;
    }
    else if (paperSize == QLatin1String("US Letter"))
    {
        params.paperWidth  = 216;
        params.paperHeight = 279;
        params.pageSize    = QPageSize::Letter;
    }

    Q_EMIT settingsChanged();
}

void CalSettings::setResolution(const QString& resolution)
{
    if      (resolution == QLatin1String("High"))
    {
        params.printResolution = QPrinter::HighResolution;
    }
    else if (resolution == QLatin1String("Low"))
    {
        params.printResolution = QPrinter::ScreenResolution;
    }

    Q_EMIT settingsChanged();
}

void CalSettings::setImagePos(int pos)
{
    const int previewSize = 300;

    switch (pos)
    {
        case CalParams::Top:
        {
            float zoom    = qMin((float)previewSize / params.paperWidth,
                                 (float)previewSize / params.paperHeight);
            params.width  = (int)(params.paperWidth  * zoom);
            params.height = (int)(params.paperHeight * zoom);

            params.imgPos = CalParams::Top;
            break;
        }

        case CalParams::Left:
        {
            float zoom    = qMin((float)previewSize / params.paperWidth,
                                 (float)previewSize / params.paperHeight);
            params.width  = (int)(params.paperHeight  * zoom);
            params.height = (int)(params.paperWidth   * zoom);

            params.imgPos = CalParams::Left;
            break;
        }

        default:
        {
            float zoom    = qMin((float)previewSize / params.paperWidth,
                                 (float)previewSize / params.paperHeight);
            params.width  = (int)(params.paperHeight  * zoom);
            params.height = (int)(params.paperWidth   * zoom);

            params.imgPos = CalParams::Right;
            break;
        }
    }

    Q_EMIT settingsChanged();
}

void CalSettings::setDrawLines(bool draw)
{
    if (params.drawLines != draw)
    {
        params.drawLines = draw;

        Q_EMIT settingsChanged();
    }
}

void CalSettings::setRatio(int ratio)
{
    if (params.ratio != ratio)
    {
        params.ratio = ratio;

        Q_EMIT settingsChanged();
    }
}

void CalSettings::setFont(const QString& font)
{
    if (params.baseFont.family() != font)
    {
        params.baseFont = QFont(font);

        Q_EMIT settingsChanged();
    }
}

void CalSettings::clearSpecial()
{
    d->special.clear();
}

void CalSettings::addSpecial(const QDate& date, const Day& info)
{
    if (d->special.contains(date))
    {
        d->special[date].second.append(QLatin1String("; ")).append(info.second);
    }
    else
    {
        d->special[date] = info;
    }
}

bool CalSettings::isPrayDay(const QDate& date) const
{
    return (date.dayOfWeek() == Qt::Sunday);
}

/*!
 * \returns true if d->special formatting is to be applied to the particular day
 */
bool CalSettings::isSpecial(int month, int day) const
{
    QDate dt = CalSystem().date(params.year, month, day);

    return (isPrayDay(dt) || d->special.contains(dt));
}

/*!
 * \returns the color to be used for painting of the day info
 */
QColor CalSettings::getDayColor(int month, int day) const
{
    QDate dt = CalSystem().date(params.year, month, day);

    if (isPrayDay(dt))
    {
        return Qt::red;
    }

    if (d->special.contains(dt))
    {
        return d->special[dt].first;
    }

    // default

    return Qt::black;
}

/*!
 * \returns the description of the day to be painted on the calendar.
 */
QString CalSettings::getDayDescr(int month, int day) const
{
    QDate dt = CalSystem().date(params.year, month, day);

    QString ret;

    if (d->special.contains(dt))
    {
        ret = d->special[dt].second;
    }

    return ret;
}

QPrinter::PrinterMode CalSettings::resolution() const
{
    return params.printResolution;
}

#ifdef HAVE_KCALENDAR

void CalSettings::loadSpecial(const QUrl& url, const QColor& color)
{
    if (url.isEmpty())
    {
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Loading calendar from file failed: No valid url provided!";
        return;
    }

#   ifdef HAVE_KCALENDAR_QDATETIME

    MemoryCalendar::Ptr memCal(new MemoryCalendar(QTimeZone::utc()));
    using DateTime = QDateTime;

#   else

    MemoryCalendar::Ptr memCal(new MemoryCalendar(QLatin1String("UTC")));
    using DateTime = KDateTime;

#   endif

    FileStorage::Ptr fileStorage(new FileStorage(memCal, url.toLocalFile(), new ICalFormat));

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Loading calendar from file " << url.toLocalFile();

    if (!fileStorage->load())
    {
        qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Failed to load calendar file!";
    }
    else
    {
        CalSystem calSys;
        QDate     qFirst, qLast;

        qFirst = calSys.date(params.year, 1, 1);
        qLast  = calSys.date(params.year + 1, 1, 1);
        qLast  = qLast.addDays(-1);

        DateTime dtFirst(qFirst, QTime(0, 0));
        DateTime dtLast(qLast, QTime(0, 0));
        DateTime dtCurrent;

        int counter      = 0;
        Event::List list = memCal->rawEvents(qFirst, qLast);

        for (const Event::Ptr& event : std::as_const(list))
        {
            qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << event->summary() << Qt::endl << "--------";
            counter++;

            if (event->recurs())
            {
                Recurrence* const recur = event->recurrence();

                for (dtCurrent = recur->getNextDateTime(dtFirst.addDays(-1)) ;
                     (dtCurrent <= dtLast) && dtCurrent.isValid() ;
                     dtCurrent = recur->getNextDateTime(dtCurrent))
                {
                    addSpecial(dtCurrent.date(), Day(color, event->summary()));
                }
            }
            else
            {
                addSpecial(event->dtStart().date(), Day(color, event->summary()));
            }
        }

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Loaded " << counter << " events";

#   if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

        memCal->close();

#   endif

        if (fileStorage->close())
        {
            qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Failed to close calendar file!";
        }
    }
}

#endif // HAVE_KCALENDAR

} // namespace Digikam

#include "moc_calsettings.cpp"
