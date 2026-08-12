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

#include "ddatetable_p.h"

// C++ includes


// Qt includes

#include <QAction>
#include <QFontDatabase>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QActionEvent>
#include <QApplication>
#include <QMenu>

namespace Digikam
{

DDateTable::Private::Private(DDateTable* const qq)
    : QObject(qq),
      q      (qq)
{
    setDate(QDate::currentDate());
}

void DDateTable::Private::nextMonth()
{
    // setDate does validity checking for us

    q->setDate(date.addMonths(1));
}

void DDateTable::Private::previousMonth()
{
    // setDate does validity checking for us

    q->setDate(date.addMonths(-1));
}

void DDateTable::Private::beginningOfMonth()
{
    // setDate does validity checking for us

    q->setDate(QDate(date.year(), date.month(), 1));
}

void DDateTable::Private::endOfMonth()
{
    // setDate does validity checking for us

    q->setDate(QDate(date.year(), date.month() + 1, 0));
}


void DDateTable::Private::beginningOfWeek()
{
    // setDate does validity checking for us

    q->setDate(date.addDays(1 - date.dayOfWeek()));
}

void DDateTable::Private::endOfWeek()
{
    // setDate does validity checking for us

    q->setDate(date.addDays(7 - date.dayOfWeek()));
}

void DDateTable::Private::setDate(const QDate& dt)
{
    date                = dt;
    weekDayFirstOfMonth = QDate(date.year(), date.month(), 1).dayOfWeek();
    numDaysThisMonth    = date.daysInMonth();
    numDayColumns       = 7;
}

} // namespace Digikam

#include "moc_ddatetable_p.cpp"
