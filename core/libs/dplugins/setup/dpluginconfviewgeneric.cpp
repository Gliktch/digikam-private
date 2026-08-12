/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : configuration view for external generic plugin
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dpluginconfviewgeneric.h"

// Local includes

#include "dplugingeneric.h"
#include "dpluginloader.h"

namespace Digikam
{

DPluginConfViewGeneric::DPluginConfViewGeneric(QWidget* const parent)
    : DPluginConfView(parent)
{
    this->loadPlugins();
}

void DPluginConfViewGeneric::loadPlugins()
{
    const DPluginLoader* const loader = DPluginLoader::instance();

    if (loader)
    {
        const auto all = loader->allPlugins();

        for (DPlugin* const tool : all)
        {
            DPluginGeneric* const gene = dynamic_cast<DPluginGeneric*>(tool);

            if (gene)
            {
                appendPlugin(gene);
            }
        }
    }

    // Sort items by plugin names.

    sortItems(DPluginConfView::Name, Qt::AscendingOrder);
}

} // namespace Digikam

#include "moc_dpluginconfviewgeneric.cpp"
