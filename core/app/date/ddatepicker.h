/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 1997-04-21
 * Description : A date selection widget.
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

// Qt includes

#include <QDateTime>
#include <QFrame>

// Local includes

#include "digikam_export.h"

class QLineEdit;

namespace Digikam
{

class DDateTable;

/**
 * @brief Provides a widget for calendar date input.
 */
class DIGIKAM_GUI_EXPORT DDatePicker : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QDate date READ date WRITE setDate NOTIFY dateChanged USER true)
    Q_PROPERTY(bool closeButton READ hasCloseButton WRITE setCloseButton)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize)

public:

    /**
     * @brief The constructor. The current date will be displayed initially.
     */
    explicit DDatePicker(QWidget* const parent = nullptr);

    /**
     * @brief The constructor. The given date will be displayed initially.
     */
    explicit DDatePicker(const QDate& dt, QWidget* const parent = nullptr);

    /**
     * @brief The destructor.
     */
    ~DDatePicker()                                                            override;

    /**
     * @brief The size hint for date pickers. The size hint recommends the
     * minimum size of the widget so that all elements may be placed
     * without clipping. This sometimes looks ugly, so when using the
     * size hint, try adding 28 to each of the reported numbers of
     * pixels.
     */
    QSize sizeHint()                                                    const override;

    /**
     * @brief Sets the date.
     *
     * @return False and does not change anything if the date given is invalid.
     */
    bool setDate(const QDate& date);

    /**
     * @return The selected date.
     */
    const QDate& date()                                                 const;

    /**
     * @return The DDateTable widget child of this DDatePicker
     * widget.
     */
    DDateTable* dateTable()                                             const;

    /**
     * @brief Sets the font size of the widgets elements.
     */
    void setFontSize(int);

    /**
     * @return The font size of the widget elements.
     */
    int fontSize()                                                      const;

    /**
     * @brief By calling this method with @p enable = true, DDatePicker will show
     * a little close-button in the upper button-row. Clicking the
     * close-button will cause the DDatePicker's topLevelWidget()'s close()
     * method being called. This is mostly useful for toplevel datepickers
     * without a window manager decoration.
     * @see hasCloseButton
     */
    void setCloseButton(bool enable);

    /**
     * @return True if a DDatePicker shows a close-button.
     * @see setCloseButton
     */
    bool hasCloseButton()                                               const;

protected:

    /// @brief to catch move keyEvents when QLineEdit has keyFocus
    bool eventFilter(QObject*, QEvent*)                                       override;

    /// @brief the resize event
    void resizeEvent(QResizeEvent*)                                           override;
    void changeEvent(QEvent*)                                                 override;

protected Q_SLOTS:

    void dateChangedSlot(const QDate& date);
    void tableClickedSlot();
    void monthForwardClicked();
    void monthBackwardClicked();
    void yearForwardClicked();
    void yearBackwardClicked();
    void selectMonthClicked();
    void selectYearClicked();
    void uncheckYearSelector();
    void lineEnterPressed();
    void todayButtonClicked();
    void weekSelected(int);

Q_SIGNALS:

    /**
     * @brief This signal is emitted each time the selected date is changed.
     * Usually, this does not mean that the date has been entered,
     * since the date also changes, for example, when another month is
     * selected.
     * @see dateSelected
     */
    void dateChanged(const QDate& date);

    /**
     * @brief This signal is emitted each time a day has been selected by
     * clicking on the table (hitting a day in the current month). It
     * has the same meaning as dateSelected() in older versions of
     * DDatePicker.
     */
    void dateSelected(const QDate& date);

    /**
     * @brief This signal is emitted when enter is pressed and a VALID date
     * has been entered before into the line edit. Connect to both
     * dateEntered() and dateSelected() to receive all events where the
     * user really enters a date.
     */
    void dateEntered(const QDate& date);

    /**
     * @brief This signal is emitted when the day has been selected by
     * clicking on it in the table.
     */
    void tableClicked();

private:

    void initWidget(const QDate& date);

private:

    class Private;
    Private* const d = nullptr;

    friend class Private;
};

} // namespace Digikam
