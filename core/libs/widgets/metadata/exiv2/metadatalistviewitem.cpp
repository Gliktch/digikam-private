/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-21
 * Description : a generic list view item widget to
 *               display metadata
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metadatalistviewitem.h"

// Qt includes

#include <QPalette>
#include <QFont>
#include <QPainter>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ditemtooltip.h"

namespace Digikam
{

MetadataListViewItem::MetadataListViewItem(QTreeWidgetItem* const parent, const QString& key,
                                           const QString& title, const QString& value)
    : QTreeWidgetItem(parent),
      m_key          (key),
      m_value        (value)
{
    setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
    setText(0, title);
    setToolTip(0, title);
    setDisabled(false);
    QString tagVal = value.simplified();

    if (tagVal.length() > 512)
    {
        tagVal.truncate(512);
        tagVal.append(QLatin1String("..."));
    }

    setText(1, tagVal);

    DToolTipStyleSheet cnt;
    setToolTip(1, QLatin1String("<qt><p>") + cnt.breakString(tagVal) + QLatin1String("</p></qt>"));
}

MetadataListViewItem::MetadataListViewItem(QTreeWidgetItem* const parent, const QString& key,
                                           const QString& title)
    : QTreeWidgetItem(parent),
      m_key          (key)
{
    setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
    setText(0, title);
    setToolTip(0, title);
    setDisabled(true);
    setText(1, i18n("Unavailable"));
    m_value   = text(1);
    QFont fnt = font(1);
    fnt.setItalic(true);
    setFont(1, fnt);
}

QString MetadataListViewItem::getKey() const
{
    return m_key;
}

QString MetadataListViewItem::getTitle() const
{
    return text(0);
}

QString MetadataListViewItem::getValue() const
{
    return text(1);
}

QString MetadataListViewItem::getFullValue() const
{
    return m_value;
}

} // namespace Digikam
