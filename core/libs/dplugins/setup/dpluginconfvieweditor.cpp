/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : configuration view for external editor plugin
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dpluginconfvieweditor.h"

// Local includes

#include "dplugineditor.h"
#include "dpluginloader.h"

namespace Digikam
{

DPluginConfViewEditor::DPluginConfViewEditor(QWidget* const parent)
    : DPluginConfView(parent)
{
    this->loadPlugins();
}

void DPluginConfViewEditor::loadPlugins()
{
    const DPluginLoader* const loader = DPluginLoader::instance();

    if (loader)
    {
        const auto all = loader->allPlugins();

        for (DPlugin* const tool : all)
        {
            DPluginEditor* const edit = dynamic_cast<DPluginEditor*>(tool);

            if (edit)
            {
                appendPlugin(edit);
            }
        }
    }

    // Sort items by plugin names.

    sortItems(DPluginConfView::Name, Qt::AscendingOrder);
}

} // namespace Digikam

#include "moc_dpluginconfvieweditor.cpp"
