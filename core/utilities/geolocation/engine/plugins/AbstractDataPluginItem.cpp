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

#include "AbstractDataPluginItem.h"

// Qt includes

#include <QAction>
#include <QRect>
#include <QSize>

// Local includes

#include "GeoDataCoordinates.h"
#include "ViewportParams.h"
#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN AbstractDataPluginItemPrivate
{
public:

    QString m_id;
    QString m_toolTip;
    bool    m_favorite                  = false;
    bool    m_sticky                    = false;
    qreal   m_addedAngularResolution    = 0.0;

    AbstractDataPluginItemPrivate();
};

AbstractDataPluginItemPrivate::AbstractDataPluginItemPrivate()
    : m_favorite              (false),
      m_sticky                (false),
      m_addedAngularResolution(0.0)
{
    // nothing to do
}

AbstractDataPluginItem::AbstractDataPluginItem(QObject* parent)
    : QObject              (parent),
      BillboardGraphicsItem(),
      d                    (new AbstractDataPluginItemPrivate)
{
    // nothing to do
}

AbstractDataPluginItem::~AbstractDataPluginItem()
{
    delete d;
}

QString AbstractDataPluginItem::toolTip() const
{
    return d->m_toolTip;
}

void AbstractDataPluginItem::setToolTip(const QString& toolTip)
{
    d->m_toolTip = toolTip;
}

QString AbstractDataPluginItem::id() const
{
    return d->m_id;
}

void AbstractDataPluginItem::setId(const QString& id)
{
    d->m_id = id;
}

bool AbstractDataPluginItem::isFavorite() const
{
    return d->m_favorite;
}

void AbstractDataPluginItem::setFavorite(bool favorite)
{
    if (isFavorite() != favorite)
    {
        d->m_favorite = favorite;

        Q_EMIT favoriteChanged(id(), favorite);
    }
}

bool AbstractDataPluginItem::isSticky() const
{
    return d->m_sticky;
}

void AbstractDataPluginItem::setSticky(bool sticky)
{
    if (d->m_sticky != sticky)
    {
        d->m_sticky = sticky;

        Q_EMIT stickyChanged();
    }
}

void AbstractDataPluginItem::toggleFavorite()
{
    setFavorite(!isFavorite());
}

qreal AbstractDataPluginItem::addedAngularResolution() const
{
    return d->m_addedAngularResolution;
}

void AbstractDataPluginItem::setAddedAngularResolution(qreal resolution)
{
    d->m_addedAngularResolution = resolution;
}

void AbstractDataPluginItem::setSettings(const QHash<QString, QVariant>& settings)
{
    Q_UNUSED(settings)
}

QAction* AbstractDataPluginItem::action()
{
    return nullptr;
}

void AbstractDataPluginItem::addDownloadedFile(const QString& url, const QString& type)
{
    Q_UNUSED(url)
    Q_UNUSED(type)
}

QList<QAction*> AbstractDataPluginItem::actions()
{
    QList<QAction*> result;

    // cppcheck-suppress constVariablePointer
    QAction* pluginAction = action();

    if (pluginAction)
    {
        result << pluginAction;
    }

    return result;
}

} // Marble namespace

#include "moc_AbstractDataPluginItem.cpp"
