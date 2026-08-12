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

#include "FloatItemsLayer.h"

// Local includes

#include "AbstractFloatItem.h"
#include "GeoPainter.h"
#include "ViewportParams.h"

namespace Marble
{

FloatItemsLayer::FloatItemsLayer(QObject* parent) :
    QObject     (parent),
    m_floatItems()
{
}

QStringList FloatItemsLayer::renderPosition() const
{
    return QStringList(QStringLiteral("FLOAT_ITEM"));
}

bool FloatItemsLayer::render(GeoPainter* painter,
                             ViewportParams* viewport,
                             const QString& renderPos,
                             GeoSceneLayer* layer)
{
    Q_UNUSED(renderPos)
    Q_UNUSED(layer)

    for (AbstractFloatItem* item : m_floatItems)
    {
        if (!item->enabled())
        {
            continue;
        }

        if (!item->isInitialized())
        {
            item->initialize();
            Q_EMIT renderPluginInitialized(item);
        }

        if (item->visible())
        {
            item->paintEvent(painter, viewport);
        }
    }

    return true;
}

void FloatItemsLayer::addFloatItem(AbstractFloatItem* floatItem)
{
    Q_ASSERT(floatItem && "must not add a null float item to FloatItemsLayer");

    connect(floatItem, SIGNAL(settingsChanged(QString)),
            this,      SIGNAL(pluginSettingsChanged()));

    connect(floatItem, SIGNAL(repaintNeeded(QRegion)),
            this,      SIGNAL(repaintNeeded(QRegion)));

    connect(floatItem, SIGNAL(visibilityChanged(bool,QString)),
            this,      SLOT(updateVisibility(bool,QString)));

    m_floatItems.append(floatItem);
}

QList<AbstractFloatItem*> FloatItemsLayer::floatItems() const
{
    return m_floatItems;
}

QString FloatItemsLayer::runtimeTrace() const
{
    return QStringLiteral("Float Items: %1").arg(m_floatItems.size());
}

void FloatItemsLayer::updateVisibility(bool visible, const QString& nameId)
{
    Q_EMIT visibilityChanged(nameId, visible);
}

} // namespace Marble

#include "moc_FloatItemsLayer.cpp"
