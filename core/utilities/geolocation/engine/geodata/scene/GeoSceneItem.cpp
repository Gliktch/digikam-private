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

#include "GeoSceneItem.h"

// Local includes

#include "GeoSceneIcon.h"
#include "GeoSceneTypes.h"

namespace Marble
{

GeoSceneItem::GeoSceneItem(const QString& name)
    : m_icon(new GeoSceneIcon),
      m_name(name),
      m_text(),
      m_connectTo(),
      m_checkable(false),
      m_spacing(12)
{
}

GeoSceneItem::~GeoSceneItem()
{
    delete m_icon;
}

const char* GeoSceneItem::nodeType() const
{
    return GeoSceneTypes::GeoSceneItemType;
}

const GeoSceneIcon* GeoSceneItem::icon() const
{
    return m_icon;
}

GeoSceneIcon* GeoSceneItem::icon()
{
    return m_icon;
}

QString GeoSceneItem::name() const
{
    return m_name;
}

QString GeoSceneItem::text() const
{
    return m_text;
}

void GeoSceneItem::setText(const QString& text)
{
    m_text = text;
}

bool GeoSceneItem::checkable() const
{
    return m_checkable;
}

void GeoSceneItem::setCheckable(bool checkable)
{
    m_checkable = checkable;
}

QString GeoSceneItem::connectTo() const
{
    return m_connectTo;
}

void GeoSceneItem::setConnectTo(const QString& connectTo)
{
    m_connectTo = connectTo;
}

int  GeoSceneItem::spacing() const
{
    return m_spacing;
}

void GeoSceneItem::setSpacing(int spacing)
{
    m_spacing = spacing;
}

} // namespace Marble
