/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-05
 * Description : simple text labels to display item
 *               properties meta infos
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itempropertiestxtlabel.h"

// Qt includes

#include <QMargins>
#include <QPalette>
#include <QColor>
#include <QFontMetrics>
#include <QFontDatabase>

namespace Digikam
{

DTextLabelName::DTextLabelName(const QString& name, QWidget* const parent)
    : DAdjustableLabel(parent)
{
    setAdjustedText(name);
    QFont fnt;
    fnt.setItalic(true);
    setFont(fnt);

    if (layoutDirection() == Qt::RightToLeft)
    {
        setAlignment(Qt::AlignRight | Qt::AlignTop);
    }
    else
    {
        setAlignment(Qt::AlignLeft | Qt::AlignTop);
    }

    setWordWrap(false);
}

DTextLabelName::~DTextLabelName()
{
}

// -------------------------------------------------------------------

DTextLabelValue::DTextLabelValue(const QString& value, QWidget* const parent)
    : DAdjustableLabel(parent)
{
    setAdjustedText(value);

    if (layoutDirection() == Qt::RightToLeft)
    {
        setAlignment(Qt::AlignLeft | Qt::AlignTop);
        setElideMode(Qt::ElideRight);
    }
    else
    {
        setAlignment(Qt::AlignRight | Qt::AlignTop);
        setElideMode(Qt::ElideLeft);
    }

    setWordWrap(false);
}

DTextLabelValue::~DTextLabelValue()
{
}

// -------------------------------------------------------------------

DTextBrowser::DTextBrowser(const QString& text, QWidget* const parent)
    : QTextBrowser(parent)
{
    setOpenExternalLinks(false);
    setOpenLinks(false);
    setText(text);
    setLinesNumber(3);
    setFocusPolicy(Qt::NoFocus);
}

DTextBrowser::~DTextBrowser()
{
}

void DTextBrowser::setLinesNumber(int l)
{
    QFont fnt;
    document()->setDefaultFont(fnt);
    QMargins m = contentsMargins();
    setFixedHeight(m.top() + m.bottom() + frameWidth() + fontMetrics().lineSpacing()*l);
}

// -------------------------------------------------------------------

DTextList::DTextList(const QStringList& list, QWidget* const parent)
    : QListWidget(parent)
{
    addItems(list);
    setLinesNumber(6);
    setFocusPolicy(Qt::NoFocus);
    sortItems();
}

DTextList::~DTextList()
{
}

void DTextList::setLinesNumber(int l)
{
    QFont fnt;
    setFont(fnt);
    QMargins m = contentsMargins();
    setFixedHeight(m.top() + m.bottom() + frameWidth() + fontMetrics().lineSpacing()*l);
}

} // namespace Digikam

#include "moc_itempropertiestxtlabel.cpp"
