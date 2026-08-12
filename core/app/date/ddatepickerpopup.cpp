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

#include "ddatepickerpopup.h"

// Qt includes

#include <QWidgetAction>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class Q_DECL_HIDDEN DDatePickerAction : public QWidgetAction
{
    Q_OBJECT

public:

    DDatePickerAction(DDatePicker* const widget, QObject* const parent)
        : QWidgetAction   (parent),
          m_datePicker    (widget),
          m_originalParent(widget->parentWidget())
    {
    }

protected:

    QWidget* createWidget(QWidget* const parent) override
    {
        m_datePicker->setParent(parent);

        return m_datePicker;
    }

    void deleteWidget(QWidget* const widget) override
    {
        if (widget != m_datePicker)
        {
            return;
        }

        m_datePicker->setParent(m_originalParent);
    }

private:

    DDatePicker* m_datePicker       = nullptr;
    QWidget*     m_originalParent   = nullptr;
};

// ---------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DDatePickerPopup::Private
{
public:

    Private() = default;

    DDatePicker* datePicker = nullptr;
    Items        items;
};

DDatePickerPopup::DDatePickerPopup(Items items, const QDate& date, QWidget* const parent)
    : QMenu(parent),
      d    (new Private)
{
    d->items      = items;
    d->datePicker = new DDatePicker(this);
    d->datePicker->setCloseButton(false);

    connect(d->datePicker, &DDatePicker::dateEntered,
            this, &DDatePickerPopup::slotDateChanged);

    connect(d->datePicker, &DDatePicker::dateSelected,
            this, &DDatePickerPopup::slotDateChanged);

    d->datePicker->setDate(date);

    buildMenu();
}

DDatePickerPopup::~DDatePickerPopup()
{
    delete d;
}

void DDatePickerPopup::buildMenu()
{
    if (isVisible())
    {
        return;
    }

    clear();

    if (d->items & DatePicker)
    {
        addAction(new DDatePickerAction(d->datePicker, this));

        if ((d->items & NoDate ) || (d->items & Words))
        {
            addSeparator();
        }
    }

    if (d->items & Words)
    {
        addAction(i18nc("@item: date menu", "&Today"),       this, SLOT(slotToday()));
        addAction(i18nc("@item: date menu", "To&morrow"),    this, SLOT(slotTomorrow()));
        addAction(i18nc("@item: date menu", "Next &Week"),   this, SLOT(slotNextWeek()));
        addAction(i18nc("@item: date menu", "Next M&onth"),  this, SLOT(slotNextMonth()));
        addAction(i18nc("@item: date menu", "Y&esterday"),   this, SLOT(slotYesterday()));
        addAction(i18nc("@item: date menu", "Last &Monday"), this, SLOT(slotPrevMonday()));
        addAction(i18nc("@item: date menu", "Last &Friday"), this, SLOT(slotPrevFriday()));
        addAction(i18nc("@item: date menu", "Last &Week"),   this, SLOT(slotPrevWeek()));
        addAction(i18nc("@item: date menu", "Last M&onth"),  this, SLOT(slotPrevMonth()));

        if (d->items & NoDate)
        {
            addSeparator();
        }
    }

    if (d->items & NoDate)
    {
        addAction(i18nc("@item: date menu", "No Date"), this, SLOT(slotNoDate()));
    }
}

DDatePicker* DDatePickerPopup::datePicker() const
{
    return d->datePicker;
}

void DDatePickerPopup::setDate(const QDate& date)
{
    d->datePicker->setDate(date);
}

#if 0
void DDatePickerPopup::setItems(int items)
{
    d->items = items;
    buildMenu();
}
#endif

int DDatePickerPopup::items() const
{
    return d->items;
}

void DDatePickerPopup::slotDateChanged(const QDate& date)
{
    Q_EMIT dateChanged(date);

    hide();
}

void DDatePickerPopup::slotToday()
{
    Q_EMIT dateChanged(QDate::currentDate());
}

void DDatePickerPopup::slotTomorrow()
{
    Q_EMIT dateChanged(QDate::currentDate().addDays(1));
}

void DDatePickerPopup::slotNoDate()
{
    Q_EMIT dateChanged(QDate());
}

void DDatePickerPopup::slotNextWeek()
{
    Q_EMIT dateChanged(QDate::currentDate().addDays(7));
}

void DDatePickerPopup::slotNextMonth()
{
    Q_EMIT dateChanged(QDate::currentDate().addMonths(1));
}

void DDatePickerPopup::slotYesterday()
{
    Q_EMIT dateChanged(QDate::currentDate().addDays(-1));
}

void DDatePickerPopup::slotPrevFriday()
{
    QDate date = QDate::currentDate();
    int day    = date.dayOfWeek();

    if (day < 6)
    {
        date = date.addDays(5 - 7 - day);
    }
    else
    {
        date = date.addDays(5 - day);
    }

    Q_EMIT dateChanged(date);
}

void DDatePickerPopup::slotPrevMonday()
{
    QDate date = QDate::currentDate();

    Q_EMIT dateChanged(date.addDays(1 - date.dayOfWeek()));
}

void DDatePickerPopup::slotPrevWeek()
{
    Q_EMIT dateChanged(QDate::currentDate().addDays(-7));
}

void DDatePickerPopup::slotPrevMonth()
{
    Q_EMIT dateChanged(QDate::currentDate().addMonths(-1));
}

} // namespace Digikam

#include "ddatepickerpopup.moc"

#include "moc_ddatepickerpopup.cpp"
