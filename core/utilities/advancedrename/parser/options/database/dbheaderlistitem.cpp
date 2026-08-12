/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-22
 * Description : header list view item
 *
 * SPDX-FileCopyrightText: 2010-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbheaderlistitem.h"

// Qt includes

#include <QPalette>
#include <QFont>
#include <QPainter>
#include <QApplication>

// Local includes

#include "thememanager.h"

namespace Digikam
{

DbHeaderListItem::DbHeaderListItem(QTreeWidget* parent, const QString& key)
    : QObject        (parent),
      QTreeWidgetItem(parent)
{
    // Reset all item flags: item is not selectable.

    setFlags(Qt::ItemIsEnabled);

    setDisabled(false);
    setExpanded(true);

    setFirstColumnSpanned(true);
    setTextAlignment(0, Qt::AlignCenter);
    QFont fn0(font(0));
    fn0.setBold(true);
    fn0.setItalic(false);
    setFont(0, fn0);
    QFont fn1(font(1));
    fn1.setBold(true);
    fn1.setItalic(false);
    setFont(1, fn1);
    setText(0, key);
    slotThemeChanged();

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

void DbHeaderListItem::slotThemeChanged()
{
    setBackground(0, QBrush(qApp->palette().color(QPalette::Highlight)));
    setBackground(1, QBrush(qApp->palette().color(QPalette::Highlight)));
    setForeground(0, QBrush(qApp->palette().color(QPalette::HighlightedText)));
    setForeground(1, QBrush(qApp->palette().color(QPalette::HighlightedText)));
}

} // namespace Digikam

#include "moc_dbheaderlistitem.cpp"
