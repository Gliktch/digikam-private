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

#include "AbstractDataPlugin.h"

// Qt includes

#include <QTimer>
#include <QRegion>

// Local includes

#include "AbstractDataPluginModel.h"
#include "AbstractDataPluginItem.h"
#include "GeoPainter.h"
#include "GeoSceneLayer.h"
#include "MarbleModel.h"
#include "ViewportParams.h"
#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN AbstractDataPluginPrivate
{
public:

    AbstractDataPluginPrivate()
    {
        m_updateTimer.setSingleShot(true);
    }

    ~AbstractDataPluginPrivate()
    {
        delete m_model;
    }

public:

    AbstractDataPluginModel* m_model         = nullptr;
    quint32                  m_numberOfItems = 10;
    QTimer                   m_updateTimer;

private:

    AbstractDataPluginPrivate(const AbstractDataPluginPrivate&)            = delete;
    AbstractDataPluginPrivate& operator=(const AbstractDataPluginPrivate&) = delete;
};

AbstractDataPlugin::AbstractDataPlugin(const MarbleModel* marbleModel)
    : RenderPlugin(marbleModel),
      d           (new AbstractDataPluginPrivate)
{
    connect(&d->m_updateTimer, SIGNAL(timeout()),
            this, SIGNAL(repaintNeeded()));
}

AbstractDataPlugin::~AbstractDataPlugin()
{
    delete d;
}

bool AbstractDataPlugin::isInitialized() const
{
    return (model() != nullptr);
}

QStringList AbstractDataPlugin::backendTypes() const
{
    return QStringList(name());
}

QString AbstractDataPlugin::renderPolicy() const
{
    return QString::fromUtf8("ALWAYS");
}

QStringList AbstractDataPlugin::renderPosition() const
{
    return QStringList(QString::fromUtf8("ALWAYS_ON_TOP"));
}

bool AbstractDataPlugin::render(GeoPainter* painter, ViewportParams* viewport,
                                const QString& renderPos, GeoSceneLayer* layer)
{
    Q_UNUSED(renderPos);
    Q_UNUSED(layer);

    QList<AbstractDataPluginItem*> items = d->m_model->items(viewport, numberOfItems());
    painter->save();

    // Paint the most important item at last

    for (int i = items.size() - 1 ; i >= 0 ; --i)
    {
        items.at(i)->paintEvent(painter, viewport);
    }

    painter->restore();

    return true;
}

AbstractDataPluginModel* AbstractDataPlugin::model()
{
    return d->m_model;
}

const AbstractDataPluginModel* AbstractDataPlugin::model() const
{
    return d->m_model;
}

void AbstractDataPlugin::setModel(AbstractDataPluginModel* model)
{
    if (d->m_model)
    {
        disconnect(d->m_model, SIGNAL(itemsUpdated()),
                   this, SLOT(delayedUpdate()));

        delete d->m_model;
    }

    d->m_model = model;

    connect(d->m_model, SIGNAL(itemsUpdated()),
            this, SLOT(delayedUpdate()));

    connect(d->m_model, SIGNAL(favoriteItemsChanged(QStringList)),
            this, SLOT(favoriteItemsChanged(QStringList)));

    connect(d->m_model, SIGNAL(favoriteItemsOnlyChanged()),
            this, SIGNAL(favoriteItemsOnlyChanged()));

    Q_EMIT favoritesModelChanged();
}

quint32 AbstractDataPlugin::numberOfItems() const
{
    return d->m_numberOfItems;
}

void AbstractDataPlugin::setNumberOfItems(quint32 number)
{
    bool changed       = (number != d->m_numberOfItems);
    d->m_numberOfItems = number;

    if (changed)
    {
        Q_EMIT changedNumberOfItems(number);
    }
}

QList<AbstractDataPluginItem*> AbstractDataPlugin::whichItemAt(const QPoint& curpos)
{
    if (d->m_model && enabled() && visible())
    {
        return d->m_model->whichItemAt(curpos);
    }
    else
    {
        return QList<AbstractDataPluginItem*>();
    }
}

RenderPlugin::RenderType AbstractDataPlugin::renderType() const
{
    return OnlineRenderType;
}

void AbstractDataPlugin::setFavoriteItemsOnly(bool favoriteOnly)
{
    if (d->m_model && (d->m_model->isFavoriteItemsOnly() != favoriteOnly))
    {
        d->m_model->setFavoriteItemsOnly(favoriteOnly);
    }
}

bool AbstractDataPlugin::isFavoriteItemsOnly() const
{
    return (d->m_model && d->m_model->isFavoriteItemsOnly());
}

QObject* AbstractDataPlugin::favoritesModel()
{
    return (d->m_model ? d->m_model->favoritesModel() : nullptr);
}

void AbstractDataPlugin::favoriteItemsChanged(const QStringList& favoriteItems)
{
    Q_UNUSED(favoriteItems)
}

void AbstractDataPlugin::delayedUpdate()
{
    if (!d->m_updateTimer.isActive())
    {
        d->m_updateTimer.start(500);
    }
}

} // namespace Marble

#include "moc_AbstractDataPlugin.cpp"
