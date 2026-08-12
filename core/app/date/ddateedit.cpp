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

#include "ddateedit.h"

// Qt includes

#include <QAbstractItemView>
#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QValidator>
#include <QWindow>
#include <QScreen>
#include <QLocale>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "ddatepickerpopup.h"

namespace Digikam
{

class Q_DECL_HIDDEN DateValidator : public QValidator
{
    Q_OBJECT

public:

    DateValidator(const QStringList& keywords, const QString& dateFormat, QWidget* const parent)
        : QValidator (parent),
          mKeywords  (keywords),
          mDateFormat(dateFormat)
    {
    }

    State validate(QString& str, int&) const override
    {
        int length = str.length();

        // empty string is intermediate so one can clear the edit line and start from scratch

        if (length <= 0)
        {
            return Intermediate;
        }

        if (mKeywords.contains(str.toLower()))
        {
            return Acceptable;
        }

        bool ok = QDate::fromString(str, mDateFormat).isValid();

        if (ok)
        {
            return Acceptable;
        }
        else
        {
            return Intermediate;
        }
    }

private:

    QStringList mKeywords;
    QString     mDateFormat;
};

// -----------------------------------------------------------------------------------

class Q_DECL_HIDDEN DDateEdit::Private
{
public:

    Private() = default;

public:

    bool                readOnly                = false;
    bool                textChanged             = false;
    bool                discardNextMousePress   = false;

    QDate               date;
    QString             dateFormat;

    QMap<QString, int>  keywordMap;

    DDatePickerPopup*   popup                   = nullptr;
};

DDateEdit::DDateEdit(QWidget* const parent, const QString& name)
    : QComboBox(parent),
      d        (new Private)
{
    setObjectName(name);

    // need at least one entry for popup to work

    setMaxCount(1);
    setEditable(true);

    d->date       = QDate::currentDate();
    d->dateFormat = getUserDateFormatString();
    QString today = d->date.toString(d->dateFormat);

    addItem(today);
    setCurrentIndex(0);
    setMinimumSize(sizeHint());
    setMinimumSize(minimumSizeHint());

    connect(lineEdit(), SIGNAL(returnPressed()),
            this, SLOT(lineEnterPressed()));

    connect(this, SIGNAL(currentTextChanged(QString)),
            this, SLOT(slotTextChanged(QString)));

    d->popup = new DDatePickerPopup(DDatePickerPopup::DatePicker | DDatePickerPopup::Words);
    d->popup->hide();
    d->popup->installEventFilter(this);

    connect(d->popup, SIGNAL(dateChanged(QDate)),
            this, SLOT(dateSelected(QDate)));

    // handle keyword entry

    setupKeywords();
    lineEdit()->installEventFilter(this);

    setValidator(new DateValidator(d->keywordMap.keys(), d->dateFormat, this));

    d->textChanged = false;
}

DDateEdit::~DDateEdit()
{
    delete d->popup;
    d->popup = nullptr;

    delete d;
}

void DDateEdit::setDate(const QDate& _date)
{
    assignDate(_date);
    updateView();
}

QDate DDateEdit::date() const
{
    return d->date;
}

void DDateEdit::setReadOnly(bool readOnly)
{
    d->readOnly = readOnly;
    lineEdit()->setReadOnly(readOnly);
}

bool DDateEdit::isReadOnly() const
{
    return d->readOnly;
}

void DDateEdit::showPopup()
{
    if (d->readOnly)
    {
        return;
    }

    QScreen* screen = qApp->primaryScreen();

    if (QWidget* const widget = nativeParentWidget())
    {
        if (QWindow* const window = widget->windowHandle())
        {
            screen = window->screen();
        }
    }

    QRect desk          = screen->geometry();
    QPoint popupPoint   = mapToGlobal(QPoint(0, 0));
    int dateFrameHeight = d->popup->sizeHint().height();

    if ((popupPoint.y() + height() + dateFrameHeight) > desk.bottom())
    {
        popupPoint.setY(popupPoint.y() - dateFrameHeight);
    }
    else
    {
        popupPoint.setY(popupPoint.y() + height());
    }

    int dateFrameWidth  = d->popup->sizeHint().width();

    if ((popupPoint.x() + dateFrameWidth) > desk.right())
    {
        popupPoint.setX(desk.right() - dateFrameWidth);
    }

    if (popupPoint.x() < desk.left())
    {
        popupPoint.setX(desk.left());
    }

    if (popupPoint.y() < desk.top())
    {
        popupPoint.setY(desk.top());
    }

    if (d->date.isValid())
    {
        d->popup->setDate(d->date);
    }
    else
    {
        d->popup->setDate(QDate::currentDate());
    }

    d->popup->popup(popupPoint);

    // The combo box is now shown pressed. Make it show not pressed again
    // by causing its (invisible) list box to emit a 'selected' signal.
    // First, ensure that the list box contains the date currently displayed.

    QDate dt                    = parseDate();
    assignDate(dt);
    updateView();

    // Now, simulate an Enter to unpress it

    QAbstractItemView* const lb = view();

    if (lb)
    {
        lb->setCurrentIndex(lb->model()->index(0, 0));
        QKeyEvent* const keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
        QApplication::postEvent(lb, keyEvent);
    }
}

void DDateEdit::dateSelected(const QDate& _date)
{
    /// @note: use dynamic binding as this virtual method can be re-implemented in derived classes.

    if (this->assignDate(_date))
    {
        updateView();

        Q_EMIT dateChanged(_date);

        if (_date.isValid())
        {
            d->popup->hide();
        }
    }
}

void DDateEdit::dateEntered(const QDate& _date)
{
    if (assignDate(_date))
    {
        updateView();

        Q_EMIT dateChanged(_date);
    }
}

void DDateEdit::lineEnterPressed()
{
    bool replaced = false;
    QDate dt      = parseDate(&replaced);

    // NOTE: use dynamic binding as this virtual method can be re-implemented in derived classes.

    if (this->assignDate(dt))
    {
        if (replaced)
        {
            updateView();
        }

        Q_EMIT dateChanged(dt);
    }
}

QDate DDateEdit::parseDate(bool* replaced) const
{
    QString text = currentText();
    QDate   result;

    if (replaced)
    {
        (*replaced) = false;
    }

    if      (text.isEmpty())
    {
        result = QDate();
    }
    else if (d->keywordMap.contains(text.toLower()))
    {
        QDate today = QDate::currentDate();
        int i       = d->keywordMap[text.toLower()];

        if (i >= 100)
        {
            /*
             * A day name has been entered. Convert to offset from today.
             * This uses some math tricks to figure out the offset in days
             * to the next date the given day of the week occurs. There
             * are two cases, that the new day is >= the current day, which means
             * the new day has not occurred yet or that the new day < the current day,
             * which means the new day is already passed (so we need to find the
             * day in the next week).
             */
            i             -= 100;
            int currentDay = today.dayOfWeek();

            if (i >= currentDay)
            {
                i -= currentDay;
            }
            else
            {
                i += 7 - currentDay;
            }
        }

        result = today.addDays(i);

        if (replaced)
        {
            (*replaced) = true;
        }
    }
    else
    {
        result = QDate::fromString(text, d->dateFormat);
    }

    return result;
}

bool DDateEdit::eventFilter(QObject* object, QEvent* event)
{
    if (object == lineEdit())
    {
        // We only process the focus out event if the text has changed
        // since we got focus

        if      ((event->type() == QEvent::FocusOut) && d->textChanged)
        {
            lineEnterPressed();
            d->textChanged = false;
        }
        else if (event->type() == QEvent::KeyPress)
        {
            // Up and down arrow keys step the date

            QKeyEvent* const keyEvent = dynamic_cast<QKeyEvent*>(event);

            if (keyEvent && (keyEvent->key() == Qt::Key_Return))
            {
                lineEnterPressed();
                return true;
            }

            int step = 0;

            if      (keyEvent && (keyEvent->key() == Qt::Key_Up))
            {
                step = 1;
            }
            else if (keyEvent && (keyEvent->key() == Qt::Key_Down))
            {
                step = -1;
            }

            if (step && !d->readOnly)
            {
                QDate dt = parseDate();

                if (dt.isValid())
                {
                    dt = dt.addDays(step);

                    if (assignDate(dt))
                    {
                        updateView();

                        Q_EMIT dateChanged(dt);

                        return true;
                    }
                }
            }
        }
    }
    else
    {
        // It's a date picker event

        switch (event->type())
        {
            case QEvent::MouseButtonDblClick:
            case QEvent::MouseButtonPress:
            {
                QMouseEvent* const mouseEvent = dynamic_cast<QMouseEvent*>(event);

                if (mouseEvent && !d->popup->rect().contains(mouseEvent->pos()))
                {
                    QPoint globalPos = d->popup->mapToGlobal(mouseEvent->pos());

                    if (QApplication::widgetAt(globalPos) == this)
                    {
                        // The date picker is being closed by a click on the
                        // DDateEdit widget. Avoid popping it up again immediately.

                        d->discardNextMousePress = true;
                    }
                }

                break;
            }

            default:
            {
                break;
            }
        }
    }

    return false;
}

void DDateEdit::mousePressEvent(QMouseEvent* e)
{
    if ((e->button() == Qt::LeftButton) && d->discardNextMousePress)
    {
        d->discardNextMousePress = false;
        return;
    }

    QComboBox::mousePressEvent(e);
}

void DDateEdit::slotTextChanged(const QString&)
{
    QDate dt = parseDate();

    // NOTE: use dynamic binding as this virtual method can be re-implemented in derived classes.

    if (this->assignDate(dt))
    {
        Q_EMIT dateChanged(dt);
    }

    d->textChanged = true;
}

void DDateEdit::setupKeywords()
{
    // Create the keyword list. This will be used to match against when the user
    // enters information.

    d->keywordMap.insert(i18nc("@item: date keyword", "tomorrow"),   1);
    d->keywordMap.insert(i18nc("@item: date keyword", "today"),      0);
    d->keywordMap.insert(i18nc("@item: date keyword", "yesterday"), -1);

    QString dayName;

    for (int i = 1 ; i <= 7 ; ++i)
    {
        dayName = QLocale().standaloneDayName(i, QLocale::LongFormat).toLower();
        d->keywordMap.insert(dayName, i + 100);
    }
}

bool DDateEdit::assignDate(const QDate& _date)
{
    d->date        = _date;
    d->textChanged = false;

    return true;
}

void DDateEdit::updateView()
{
    QString dateString;

    if (d->date.isValid())
    {
        dateString = d->date.toString(d->dateFormat);
    }

    // We do not want to generate a signal here,
    // since we explicitly setting the date

    bool blocked = signalsBlocked();
    blockSignals(true);
    removeItem(0);
    insertItem(0, dateString);
    blockSignals(blocked);
}

} // namespace Digikam

#include "ddateedit.moc"

#include "moc_ddateedit.cpp"
