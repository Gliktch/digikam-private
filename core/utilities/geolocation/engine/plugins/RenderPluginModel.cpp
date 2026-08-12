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

#include "RenderPluginModel.h"

// Local includes

#include "DialogConfigurationInterface.h"
#include "PluginInterface.h"
#include "RenderPlugin.h"

namespace Marble
{

class Q_DECL_HIDDEN RenderPluginModel::Private
{
public:

    Private();

    static bool renderPluginGuiStringLessThan(const RenderPlugin* one, const RenderPlugin* two)
    {
        // Sort by gui string ignoring keyboard accelerators

        return (one->guiString().remove(QLatin1Char('&')) < two->guiString().remove(QLatin1Char('&')));
    }

public:

    QList<RenderPlugin*> m_renderPlugins;
};

RenderPluginModel::Private::Private()
    : m_renderPlugins()
{
}

RenderPluginModel::RenderPluginModel(QObject* parent)
    : QStandardItemModel(parent),
      d                 (new Private)
{
}

RenderPluginModel::~RenderPluginModel()
{
    // our model doesn't own the items, so take them away

    while (invisibleRootItem()->hasChildren())
    {
        invisibleRootItem()->takeRow(0);
    }

    delete d;
}

void RenderPluginModel::setRenderPlugins(const QList<RenderPlugin*>& renderPlugins)
{
    // our model doesn't own the items, so take them away

    while (invisibleRootItem()->hasChildren())
    {
        invisibleRootItem()->takeRow(0);
    }

    d->m_renderPlugins = renderPlugins;
    std::sort(d->m_renderPlugins.begin(), d->m_renderPlugins.end(), Private::renderPluginGuiStringLessThan);

    QStandardItem* parentItem = invisibleRootItem();

    for (RenderPlugin* plugin : d->m_renderPlugins)
    {
        parentItem->appendRow(plugin->item());
    }
}

QVector<PluginAuthor> RenderPluginModel::pluginAuthors(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QVector<PluginAuthor>();
    }

    if ((index.row() < 0) || (index.row() >= d->m_renderPlugins.count()))
    {
        return QVector<PluginAuthor>();
    }

    return d->m_renderPlugins.at(index.row())->pluginAuthors();
}

RenderPlugin* RenderPluginModel::plugin(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return nullptr;
    }

    if ((index.row() < 0) || (index.row() >= d->m_renderPlugins.count()))
    {
        return nullptr;
    }

    RenderPlugin* const plugin = d->m_renderPlugins.at(index.row());

    return plugin;
}

DialogConfigurationInterface* RenderPluginModel::pluginDialogConfigurationInterface(const QModelIndex& index)
{
    return qobject_cast<DialogConfigurationInterface*>(plugin(index));
}

PluginInterface* RenderPluginModel::pluginIface(const QModelIndex& index)
{
    return dynamic_cast<PluginInterface*>(plugin(index));
}

void RenderPluginModel::retrievePluginState()
{
    for (RenderPlugin* plugin : d->m_renderPlugins)
    {
        plugin->retrieveItemState();
    }
}

void RenderPluginModel::applyPluginState()
{
    for (RenderPlugin* plugin : d->m_renderPlugins)
    {
        plugin->applyItemState();
    }
}

} // namespace Marble

#include "moc_RenderPluginModel.cpp"
