/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-04-21
 * Description : a menu widget to pick a date.
 *
 * SPDX-FileCopyrightText: 2004      by Bram Schoenmakers <bramschoenmakers at kde dot nl>
 * SPDX-FileCopyrightText: 2006      by Mikolaj Machowski <mikmach at wp dot pl>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDateTime>
#include <QMenu>

// Local includes

#include "ddatepicker.h"

namespace Digikam
{

/**
 * @short This menu helps the user to select a date quickly.
 *
 * @brief This menu helps the user to select a date quickly. It offers various ways of selecting, e.g. with a DDatePicker or with words like "Tomorrow".
 *
 * The available items are:
 *
 * @li NoDate: A menu-item with "No Date". If chosen, the datepicker will emit a null QDate.
 * @li DatePicker: Show a DDatePicker-widget.
 * @li Words: Show items like "Today", "Tomorrow" or "Next Week".
 *
 * When supplying multiple items, separate each item with a bitwise OR.
 */
class DDatePickerPopup : public QMenu
{
    Q_OBJECT

public:

    enum ItemFlag
    {
        NoDate     = 1,
        DatePicker = 2,
        Words      = 4
    };

    Q_DECLARE_FLAGS(Items, ItemFlag)

public:

    /**
     * @brief A constructor for the DDatePickerPopup.
     *
     * @param items List of all desirable items, separated with a bitwise OR.
     * @param date Initial date of datepicker-widget.
     * @param parent The object's parent.
     */
    explicit DDatePickerPopup(Items items,
                              const QDate& date = QDate::currentDate(),
                              QWidget* const parent = nullptr);
    ~DDatePickerPopup()             override;

    /**
     * @return A pointer to the private variable mDatePicker, an instance of
     * DDatePicker.
     */
    DDatePicker* datePicker() const;

    void setDate(const QDate& date);

#if 0

    /**
     * @brief Set items which should be shown and rebuilds the menu afterwards. Only if the menu is not visible.
     * @param Items List of all desirable items, separated with a bitwise OR.
     */
    void setItems(int items = 1);

#endif

    /**
     * @return The bitwise result of the active items in the popup.
     */
    int items()               const;

Q_SIGNALS:

    /**
     *  @brief This signal emits the new date (selected with datepicker or other
     *  menu-items).
     */
    void dateChanged(const QDate&);

protected Q_SLOTS:

    void slotDateChanged(const QDate&);
    void slotToday();
    void slotTomorrow();
    void slotNextWeek();
    void slotNextMonth();

    void slotYesterday();
    void slotPrevMonday();
    void slotPrevFriday();
    void slotPrevWeek();
    void slotPrevMonth();

    void slotNoDate();

private:

    void buildMenu();

private:

    /// @note disabled
    DDatePickerPopup() = delete;

private:

    class Private;
    Private* const d = nullptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DDatePickerPopup::Items)

} // namespace Digikam
