/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-04-18
 * Description : ExifTool metadata list view item.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "exiftoollistviewitem.h"

// Qt includes

#include <QHeaderView>
#include <QFont>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ditemtooltip.h"
#include "exiftoollistviewgroup.h"

namespace Digikam
{

class Q_DECL_HIDDEN ExifToolListViewItem::Private
{
public:

    Private() = default;

    QString key;
    QString desc;
    QString value;
};

ExifToolListViewItem::ExifToolListViewItem(ExifToolListViewGroup* const parent,
                                           const QString& key,
                                           const QString& value,
                                           const QString& desc)
    : QTreeWidgetItem(parent),
      d              (new Private)
{
    d->key   = key;
    d->desc  = desc;
    d->value = value;


    setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
    setDisabled(false);
    setSelected(false);

    QString name = d->key.section(QLatin1Char('.'), -1);
    setText(0, name);
    setToolTip(0, name);

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

ExifToolListViewItem::ExifToolListViewItem(ExifToolListViewGroup* const parent,
                                           const QString& key)
    : QTreeWidgetItem(parent),
      d              (new Private)
{
    d->key = key;

    setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

    QString name = d->key.section(QLatin1Char('.'), -1);
    setText(0, name);
    setToolTip(0, name);

    setDisabled(true);
    setText(1, i18n("Unavailable"));
    QFont fnt = font(1);
    fnt.setItalic(true);
    setFont(1, fnt);
}

ExifToolListViewItem::~ExifToolListViewItem()
{
    delete d;
}

QString ExifToolListViewItem::getKey() const
{
    return d->key;
}

QString ExifToolListViewItem::getTitle() const
{
    return text(0);
}

QString ExifToolListViewItem::getValue() const
{
    return text(1);
}

QString ExifToolListViewItem::getFullValue() const
{
    return d->value;
}

QString ExifToolListViewItem::getDescription() const
{
    return d->desc;
}

} // namespace Digikam
