/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-02-01
 * Description : album view configuration setup tab
 *
 * SPDX-FileCopyrightText: 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupalbumview_p.h"

namespace Digikam
{

void SetupAlbumView::setupTreeView()
{
    QWidget* const fwpanel           = new QWidget(d->tab);
    QGridLayout* const grid2         = new QGridLayout(fwpanel);

    QLabel* const iconTreeThumbLabel = new QLabel(i18n("Tree View icon size:"), fwpanel);
    d->iconTreeThumbSize             = new QComboBox(fwpanel);
    d->iconTreeThumbSize->addItem(QLatin1String("16"), 16);
    d->iconTreeThumbSize->addItem(QLatin1String("22"), 22);
    d->iconTreeThumbSize->addItem(QLatin1String("32"), 32);
    d->iconTreeThumbSize->addItem(QLatin1String("48"), 48);
    d->iconTreeThumbSize->addItem(QLatin1String("64"), 64);
    d->iconTreeThumbSize->setToolTip(i18n("Set this option to configure the size in pixels of "
                                          "the Tree View icons in digiKam's sidebars."));

    QLabel* const iconTreeFaceLabel  = new QLabel(i18n("People Tree View icon size:"), fwpanel);
    d->iconTreeFaceSize              = new QComboBox(fwpanel);
    d->iconTreeFaceSize->addItem(QLatin1String("16"),   16);
    d->iconTreeFaceSize->addItem(QLatin1String("22"),   22);
    d->iconTreeFaceSize->addItem(QLatin1String("32"),   32);
    d->iconTreeFaceSize->addItem(QLatin1String("48"),   48);
    d->iconTreeFaceSize->addItem(QLatin1String("64"),   64);
    d->iconTreeFaceSize->addItem(QLatin1String("128"), 128);
    d->iconTreeFaceSize->setToolTip(i18n("Set this option to configure the size in pixels of "
                                         "the Tree View icons in digiKam's people sidebar."));

    d->treeViewFontSelect            = new DFontSelect(i18n("Tree View font:"), fwpanel);
    d->treeViewFontSelect->setToolTip(i18n("Select here the font used to display text in Tree Views."));

    d->showFolderTreeViewItemsCount  = new QCheckBox(i18n("Show a count of items in Tree Views"), fwpanel);
    d->showFolderTreeViewItemsCount->setToolTip(i18n("Set this option to display along the album name the number of icon-view items inside."));

    grid2->addWidget(iconTreeThumbLabel,              0, 0, 1, 1);
    grid2->addWidget(d->iconTreeThumbSize,            0, 1, 1, 1);
    grid2->addWidget(iconTreeFaceLabel,               1, 0, 1, 1);
    grid2->addWidget(d->iconTreeFaceSize,             1, 1, 1, 1);
    grid2->addWidget(d->treeViewFontSelect,           2, 0, 1, 2);
    grid2->addWidget(d->showFolderTreeViewItemsCount, 3, 0, 1, 2);
    grid2->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    grid2->setSpacing(d->spacing);
    grid2->setRowStretch(4, 10);

    d->tab->insertTab(FolderView, fwpanel, i18nc("@title:tab", "Tree-Views"));
}

} // namespace Digikam
