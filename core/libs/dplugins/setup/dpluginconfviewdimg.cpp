/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-09-22
 * Description : configuration view for external DImg plugin
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dpluginconfviewdimg.h"

// Qt includes

#include <QHeaderView>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "itempropertiestxtlabel.h"
#include "dplugindimg.h"
#include "dpluginloader.h"

namespace Digikam
{

DPluginConfViewDImg::DPluginConfViewDImg(QWidget* const parent)
    : DPluginConfView(parent)
{
    setColumnHidden(DPluginConfView::Categories, true);
    headerItem()->setText(DPluginConfView::Tools, i18n("Type-Mimes"));
    header()->setSectionResizeMode(DPluginConfView::Tools, QHeaderView::Stretch);
    this->loadPlugins();
}

void DPluginConfViewDImg::loadPlugins()
{
    const DPluginLoader* const loader = DPluginLoader::instance();

    if (loader)
    {
        const auto tools = loader->allPlugins();

        for (DPlugin* const tool : tools)
        {
            DPluginDImg* const plug = dynamic_cast<DPluginDImg*>(tool);

            if (plug)
            {
                QTreeWidgetItem* const item = appendPlugin(plug);
                DTextBrowser* const tview   = new DTextBrowser(plug->typeMimes(), this);
                setItemWidget(item, DPluginConfView::Tools, tview);
            }
        }
    }

    // Sort items by plugin names.

    sortItems(DPluginConfView::Name, Qt::AscendingOrder);
}

} // namespace Digikam

#include "moc_dpluginconfviewdimg.cpp"
