/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-12-08
 * Description : a widget to display date and time statistics of pictures
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QWidget>
#include <QDateTime>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QMouseEvent>

// Local includes

#include "searchmodificationhelper.h"

namespace Digikam
{

class TimeLineWidget : public QWidget
{
    Q_OBJECT

public:

    enum TimeUnit
    {
        Day = 0,
        Week,
        Month,
        Year
    };

    enum SelectionMode
    {
        Unselected = 0,    ///< No selection.
        FuzzySelection,    ///< Partially selected.
        Selected           ///< Fully selected.
    };

    enum ScaleMode
    {
        LinScale = 0,      ///< Linear scale.
        LogScale           ///< Logarithmic scale.
    };

public:

    explicit TimeLineWidget(QWidget* const parent = nullptr);
    ~TimeLineWidget()                               override;

    void      setTimeUnit(TimeUnit timeUnit);
    TimeUnit  timeUnit()                                                        const;

    void      setScaleMode(ScaleMode scaleMode);
    ScaleMode scaleMode()                                                       const;

    void      setCursorDateTime(const QDateTime& dateTime);
    QDateTime cursorDateTime()                                                  const;
    int       cursorInfo(QString& infoDate)                                     const;

    /**
     * @return A list of Date-Range based on selection performed on days-map
     */
    DateRangeList selectedDateRange(int& totalCount)                            const;
    void          setSelectedDateRange(const DateRangeList& list);

    int  totalIndex()                                                           const;
    int  indexForRefDateTime()                                                  const;
    int  indexForCursorDateTime()                                               const;
    void setCurrentIndex(int index);

Q_SIGNALS:

    void signalCursorPositionChanged();
    void signalSelectionChanged();
    void signalRefDateTimeChanged();
    void signalDateMapChanged();

public Q_SLOTS:

    void slotDatesMap(const QMap<QDateTime, int>&);
    void slotPrevious();
    void slotNext();
    void slotBackward();
    void slotForward();
    void slotResetSelection();

private Q_SLOTS:

    void slotThemeChanged();

private:

    QDateTime     prevDateTime(const QDateTime& dt)                             const;
    QDateTime     nextDateTime(const QDateTime& dt)                             const;

    double        daysOfTimeUnit()                                              const;

    int           maxCount()                                                    const;
    int           indexForDateTime(const QDateTime& date)                       const;
    int           statForDateTime(const QDateTime& dt, SelectionMode& selected) const;
    void          setRefDateTime(const QDateTime& dateTime);

    void          paintEvent(QPaintEvent*)          override;
    void          wheelEvent(QWheelEvent*)          override;

    void          mousePressEvent(QMouseEvent*)     override;
    void          mouseMoveEvent(QMouseEvent*)      override;
    void          mouseReleaseEvent(QMouseEvent*)   override;

    void          keyPressEvent(QKeyEvent *e)       override;
    void          keyReleaseEvent(QKeyEvent *)      override;
    void          keyScroll(bool isScrollNext);

    QDateTime     dateTimeForPoint(const QPoint& pt,
                                   bool& isOnSelectionArea);
    QDateTime     firstDayOfWeek(int year, int weekNumber)                      const;

    void          resetSelection();
    void          setDateTimeSelected(const QDateTime& dt,
                                      SelectionMode selected);
    void          setDaysRangeSelection(const QDateTime& dts,
                                        const QDateTime& dte,
                                        SelectionMode selected);
    SelectionMode checkSelectionForDaysRange(const QDateTime& dts,
                                             const QDateTime& dte)              const;
    void          updateWeekSelection(const QDateTime& dts,
                                      const QDateTime& dte);
    void          updateMonthSelection(const QDateTime& dts,
                                       const QDateTime& dte);
    void          updateYearSelection(const QDateTime& dts,
                                      const QDateTime& dte);
    void          updateAllSelection();

    /**
     * @brief Helper methods for painting
     */
    int           calculateTop(int& val)                                        const;
    void          paintItem(QPainter& p,
                            const QRect& barRect,
                            const QDateTime& ref,
                            const int& separatorPosition,
                            const QColor& dateColor,
                            const QColor& subDateColor);

    void          handleSelectionRange(const QDateTime& selEndDateTime);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
