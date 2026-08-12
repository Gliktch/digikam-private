/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 1997-04-21
 * Description : Date selection table.
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 1997      by Tim D. Gilman <tdgilman at best dot org>
 * SPDX-FileCopyrightText: 1998-2001 by Mirko Boehm <mirko at kde dot org>
 * SPDX-FileCopyrightText: 2007      by John Layt <john at layt dot net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "ddatetable.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QObject>
#include <QColor>
#include <QDate>
#include <QRectF>
#include <QHash>

namespace Digikam
{

class Q_DECL_HIDDEN DDateTable::Private : public QObject
{
    Q_OBJECT

public:

    class Q_DECL_HIDDEN DatePaintingMode
    {
    public:

        QColor         fgColor;
        QColor         bgColor;
        BackgroundMode bgMode;
    };

public:

    explicit Private(DDateTable* const qq);
    ~Private() override = default;

public Q_SLOTS:

    void setDate(const QDate&);
    void nextMonth();
    void previousMonth();
    void beginningOfMonth();
    void endOfMonth();
    void beginningOfWeek();
    void endOfWeek();

public:

    DDateTable*                   q                     = nullptr;

    /**
     * The currently selected date.
     */
    QDate                         date;

    /**
     * The weekday number of the first day in the month [1..daysInWeek()].
     */
    int                           weekDayFirstOfMonth   = 0;

    /**
     * The number of days in the current month.
     */
    int                           numDaysThisMonth      = 0;

    /**
     * Save the size of the largest used cell content.
     */
    QRectF                        maxCell;

    /**
     * How many week rows we are to draw.
     */
    int                           numWeekRows           = 0;

    /**
     * How many day columns we are to draw, i.e. days in a week.
     */
    int                           numDayColumns         = 0;

    /**
     * The font size of the displayed text.
     */
    int                           fontsize              = 0;

    bool                          popupMenuEnabled      = false;
    bool                          useCustomColors       = false;

    QHash <int, DatePaintingMode> customPaintingModes;

    int                           hoveredPos            = -1;
};

} // namespace Digikam
