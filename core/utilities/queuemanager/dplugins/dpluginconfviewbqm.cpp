/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : configuration view for external BQM plugin
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dpluginconfviewbqm.h"

// Local includes

#include "dpluginbqm.h"
#include "dpluginloader.h"

namespace Digikam
{

DPluginConfViewBqm::DPluginConfViewBqm(QWidget* const parent)
    : DPluginConfView(parent)
{
    this->loadPlugins();
}

void DPluginConfViewBqm::loadPlugins()
{
    const DPluginLoader* const loader = DPluginLoader::instance();

    if (loader)
    {
        const auto tools = loader->allPlugins();

        for (DPlugin* const tool : tools)
        {
            DPluginBqm* const bqm = dynamic_cast<DPluginBqm*>(tool);

            if (bqm)
            {
                appendPlugin(bqm);
            }
        }
    }

    // Sort items by plugin names.

    sortItems(DPluginConfView::Name, Qt::AscendingOrder);
}

} // namespace Digikam

#include "moc_dpluginconfviewbqm.cpp"
