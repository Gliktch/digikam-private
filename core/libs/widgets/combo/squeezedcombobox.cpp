/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-08-21
 * Description : a combo box with a width not depending of text
 *               content size
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2008 by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText:      2005 by Tom Albers <tomalbers at kde dot nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "squeezedcombobox.h"

// Qt includes

#include <QTimer>
#include <QStyle>
#include <QApplication>
#include <QResizeEvent>

namespace Digikam
{

class Q_DECL_HIDDEN SqueezedComboBox::Private
{
public:

    Private() = default;

    QMap<int, QString> originalItems;

    QTimer*            timer = nullptr;
};

SqueezedComboBox::SqueezedComboBox(QWidget* const parent, const char* name)
    : QComboBox(parent),
      d        (new Private)
{
    setObjectName(QString::fromUtf8(name));
    setMinimumWidth(100);

    d->timer = new QTimer(this);
    d->timer->setSingleShot(true);

    connect(d->timer, &QTimer::timeout,
            this, &SqueezedComboBox::slotTimeOut);

    connect(this, static_cast<void (SqueezedComboBox::*)(int)>(&SqueezedComboBox::activated),
            this, &SqueezedComboBox::slotUpdateToolTip);
}

SqueezedComboBox::~SqueezedComboBox()
{
    d->originalItems.clear();

    delete d->timer;
    delete d;
}

bool SqueezedComboBox::contains(const QString& text) const
{
    if (text.isEmpty())
    {
        return false;
    }

    for (QMap<int, QString>::const_iterator it = d->originalItems.constBegin() ;
         it != d->originalItems.constEnd() ; ++it)
    {
        if (it.value() == text)
        {
            return true;
        }
    }

    return false;
}

int SqueezedComboBox::findOriginalText(const QString& text,
                                       Qt::CaseSensitivity cs) const
{
    if (text.isEmpty())
    {
        return -1;
    }

    for (QMap<int, QString>::const_iterator it = d->originalItems.constBegin() ;
         it != d->originalItems.constEnd() ; ++it)
    {
        if (it.value().compare(text, cs) == 0)
        {
            return it.key();
        }
    }

    return -1;
}

QSize SqueezedComboBox::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm = fontMetrics();
    int maxW        = count() ? 18 : 7 * fm.horizontalAdvance(QLatin1Char('x')) + 18;
    int maxH        = qMax(fm.lineSpacing(), 14) + 2;

    QStyleOptionComboBox options;
    options.initFrom(this);

    return style()->sizeFromContents(QStyle::CT_ComboBox, &options, QSize(maxW, maxH), this);
}

void SqueezedComboBox::insertSqueezedItem(const QString& newItem, int index,
                                          const QVariant& userData)
{
    d->originalItems[index] = newItem;
    QComboBox::insertItem(index, squeezeText(newItem), userData);

    // if this is the first item, set the tooltip.

    if (index == 0)
    {
        slotUpdateToolTip(0);
    }
}

void SqueezedComboBox::insertSqueezedList(const QStringList& newItems, int index)
{
    for (QStringList::const_iterator it = newItems.constBegin() ;
         it != newItems.constEnd() ; ++it)
    {
        insertSqueezedItem(*it, index);
        ++index;
    }
}

void SqueezedComboBox::addSqueezedItem(const QString& newItem,
                                       const QVariant& userData)
{
    insertSqueezedItem(newItem, count(), userData);
}

void SqueezedComboBox::setCurrent(const QString& itemText)
{
    int itemIndex = findOriginalText(itemText);

    if (itemIndex >= 0)
    {
        setCurrentIndex(itemIndex);
    }
}

void SqueezedComboBox::resizeEvent(QResizeEvent *)
{
    d->timer->start(200);
}

void SqueezedComboBox::slotTimeOut()
{
    for (QMap<int, QString>::iterator it = d->originalItems.begin() ;
         it != d->originalItems.end() ; ++it)
    {
        setItemText(it.key(), squeezeText(it.value()));
    }
}

QString SqueezedComboBox::squeezeText(const QString& original) const
{
    // not the complete widgetSize is usable. Need to compensate for that.

    int widgetSize = width() - 30;
    QFontMetrics fm(fontMetrics());

    // If we can fit the full text, return that.

    if (fm.horizontalAdvance(original) < widgetSize)
    {
        return (original);
    }

    // We need to squeeze.

    QString sqItem = original; // prevent empty return value;
    widgetSize     = widgetSize-fm.horizontalAdvance(QLatin1String("..."));

    for (int i = 0 ; i != original.length() ; ++i)
    {
        if ((int)fm.horizontalAdvance(original.right(i)) > widgetSize)
        {
            sqItem = QString(original.left(i) + QLatin1String("..."));
            break;
        }
    }

    return sqItem;
}

void SqueezedComboBox::slotUpdateToolTip(int index)
{
     setToolTip(d->originalItems[index]);

     Q_EMIT signalItemActivated(item(index));
}

QString SqueezedComboBox::itemHighlighted() const
{
    int curItem = currentIndex();

    return d->originalItems[curItem];
}

QString SqueezedComboBox::item(int index) const
{
    return d->originalItems[index];
}

} // namespace Digikam

#include "moc_squeezedcombobox.cpp"
