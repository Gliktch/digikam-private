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

#include "GeoSceneHead.h"

// Local includes

#include "GeoSceneTypes.h"
#include "GeoSceneIcon.h"
#include "GeoSceneZoom.h"
#include "GeoSceneLicense.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoSceneHeadPrivate
{
public:

    GeoSceneHeadPrivate()
        : m_zoom       (new GeoSceneZoom),
          m_icon       (new GeoSceneIcon),
          m_license    (new GeoSceneLicense),
          m_name       (),
          m_target     (),
          m_theme      (),
          m_description(),
          m_radius     (0.0),
          m_visible    (true)
    {
    }

    ~GeoSceneHeadPrivate()
    {
        delete m_icon;
        delete m_zoom;
        delete m_license;
    }

public:

    GeoSceneZoom*       m_zoom          = nullptr;
    GeoSceneIcon*       m_icon          = nullptr;
    GeoSceneLicense*    m_license       = nullptr;

    QString             m_name;
    QString             m_target;
    QString             m_theme;
    QString             m_description;
    qreal               m_radius        = 0.0;

    bool                m_visible       = true;

private:

    GeoSceneHeadPrivate(const GeoSceneHeadPrivate&)            = delete;
    GeoSceneHeadPrivate& operator=(const GeoSceneHeadPrivate&) = delete;
};

GeoSceneHead::GeoSceneHead()
    : d(new GeoSceneHeadPrivate)
{
}

GeoSceneHead::~GeoSceneHead()
{
    delete d;
}

const char* GeoSceneHead::nodeType() const
{
    return GeoSceneTypes::GeoSceneHeadType;
}

QString GeoSceneHead::name() const
{
    return d->m_name;
}

void GeoSceneHead::setName(const QString& name)
{
    d->m_name = name;
}

QString GeoSceneHead::target() const
{
    return d->m_target;
}

void GeoSceneHead::setTarget(const QString& target)
{
    d->m_target = target;
}

QString GeoSceneHead::theme() const
{
    return d->m_theme;
}

QString GeoSceneHead::mapThemeId() const
{
    return (d->m_target + QLatin1Char('/') + d->m_theme + QLatin1Char('/') + d->m_theme + QLatin1String(".dgml"));
}

void GeoSceneHead::setTheme(const QString& theme)
{
    d->m_theme = theme;
}

QString GeoSceneHead::description() const
{
    return d->m_description;
}

void GeoSceneHead::setDescription(const QString& description)
{
    d->m_description = description;
}

qreal GeoSceneHead::radius() const
{
    return d->m_radius;
}

void GeoSceneHead::setRadius(qreal radius)
{
    d->m_radius = radius;
}

bool GeoSceneHead::visible() const
{
    return d->m_visible;
}

void GeoSceneHead::setVisible(bool visible)
{
    d->m_visible = visible;
}

const GeoSceneIcon* GeoSceneHead::icon() const
{
    return d->m_icon;
}

GeoSceneIcon* GeoSceneHead::icon()
{
    return d->m_icon;
}

const GeoSceneZoom* GeoSceneHead::zoom() const
{
    return d->m_zoom;
}

GeoSceneZoom* GeoSceneHead::zoom()
{
    return d->m_zoom;
}

const GeoSceneLicense* GeoSceneHead::license() const
{
    return d->m_license;
}

GeoSceneLicense* GeoSceneHead::license()
{
    return d->m_license;
}

} // namespace Marble
