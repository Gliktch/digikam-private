/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "GeoDataItemIcon.h"

// Qt includes

#include <QString>
#include <QImage>

// Local includes

#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataItemIconPrivate
{
public:

    GeoDataItemIconPrivate();

    GeoDataItemIcon::ItemIconStates m_state;
    QString                         m_iconPath;
    QImage                          m_icon;
};

GeoDataItemIconPrivate::GeoDataItemIconPrivate()
    : m_state(),
      m_iconPath(),
      m_icon()
{
}

GeoDataItemIcon::GeoDataItemIcon()
    : d(new GeoDataItemIconPrivate)
{
}

GeoDataItemIcon::GeoDataItemIcon(const Marble::GeoDataItemIcon& other)
    : GeoDataObject(), d(new GeoDataItemIconPrivate(*other.d))
{
}

GeoDataItemIcon& GeoDataItemIcon::operator=(const GeoDataItemIcon& other)
{
    GeoDataObject::operator=(other);
    *d = *other.d;
    return *this;
}

bool GeoDataItemIcon::operator==(const GeoDataItemIcon& other) const
{
    return equals(other) &&
           d->m_state == other.d->m_state &&
           d->m_iconPath == other.d->m_iconPath &&
           d->m_icon == other.d->m_icon;
}

bool GeoDataItemIcon::operator!=(const GeoDataItemIcon& other) const
{
    return !this->operator==(other);
}

GeoDataItemIcon::~GeoDataItemIcon()
{
    delete d;
}

const char* GeoDataItemIcon::nodeType() const
{
    return GeoDataTypes::GeoDataItemIconType;
}

GeoDataItemIcon::ItemIconStates GeoDataItemIcon::state() const
{
    return d->m_state;
}

void GeoDataItemIcon::setState(ItemIconStates state)
{
    d->m_state = state;
}

void GeoDataItemIcon::setIcon(const QImage& icon)
{
    d->m_icon = icon;
}

QString GeoDataItemIcon::iconPath() const
{
    return d->m_iconPath;
}

void GeoDataItemIcon::setIconPath(const QString& path)
{
    d->m_iconPath = path;
}

QImage GeoDataItemIcon::icon() const
{
    if (!d->m_icon.isNull())
    {
        return d->m_icon;
    }

    else if (!d->m_iconPath.isEmpty())
    {
        d->m_icon = QImage(resolvePath(d->m_iconPath));
        return d->m_icon;
    }

    else
    {
        return QImage();
    }
}

} // namespace Marble
