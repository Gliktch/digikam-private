/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-01-10
 * Description : a combo box to list date.
 *               this widget come from libkdepim.
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2002      by Cornelius Schumacher <schumacher at kde dot org>
 * SPDX-FileCopyrightText: 2003-2004 by Reinhold Kainhofer <reinhold at kainhofer dot com>
 * SPDX-FileCopyrightText: 2004      by Tobias Koenig <tokoe at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMouseEvent>
#include <QEvent>
#include <QComboBox>

namespace Digikam
{

/**
 * @brief A date editing widget that consists of an editable combo box.
 * The combo box contains the date in text form, and clicking the combo
 * box arrow will display a 'popup' style date picker.
 *
 * This widget also supports advanced features like allowing the user
 * to type in the day name to get the date. The following keywords
 * are supported (in the native language): tomorrow, yesterday, today,
 * Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday.
 *
 */
class DDateEdit : public QComboBox
{
    Q_OBJECT

public:

    explicit DDateEdit(QWidget* const parent = nullptr, const QString& name = QString());
    ~DDateEdit() override;

    /**
     * @return The date entered. This date could be invalid,
     *         you have to check validity yourself.
     */
    QDate date()                                                const;

    /**
     * @brief Sets whether the widget is read-only for the user. If read-only,
     * the date picker pop-up is inactive, and the displayed date cannot be edited.
     *
     * @param readOnly True to set the widget read-only, false to set it read-write.
     */
    void setReadOnly(bool readOnly);

    /**
     * @return True if the widget is read-only, false if read-write.
     */
    bool isReadOnly()                                           const;

    void showPopup()                                                  override;

Q_SIGNALS:

    /**
     * @brief This signal is emitted whenever the user modifies the date.
     * The passed date can be invalid.
     */
    void dateChanged(const QDate& date);

public Q_SLOTS:

    /**
     * @brief Sets the date.
     *
     * @param date The new date to display. This date must be valid or
     *             it will not be set
     */
    void setDate(const QDate& date);

protected Q_SLOTS:

    void lineEnterPressed();
    void slotTextChanged(const QString&);
    void dateEntered(const QDate&);
    void dateSelected(const QDate&);

protected:

    bool eventFilter(QObject*, QEvent*)                               override;
    void mousePressEvent(QMouseEvent*)                                override;

    /**
     * @brief Sets the date, without altering the display.
     * This method is used internally to set the widget's date value.
     * As a virtual method, it allows derived classes to perform additional validation
     * on the date value before it is set. Derived classes should return true if
     * QDate::isValid(@p date) returns false.
     *
     * @param date The new date to set.
     * @return True if the date was set, false if it was considered invalid and
     *         remains unchanged.
     */
    virtual bool assignDate(const QDate& date);

    /**
     * @brief Fills the keyword map. Re-implement it if you want additional
     * keywords.
     */
    void setupKeywords();

private:

    QDate parseDate(bool* = nullptr)                            const;
    void updateView();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
