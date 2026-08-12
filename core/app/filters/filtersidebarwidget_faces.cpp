/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : filters view for the right sidebar
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2011 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText:      2014 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "filtersidebarwidget_p.h"

namespace Digikam
{

void FilterSideBarWidget::populateFacesView()
{
    QWidget* const box5 = new QWidget(d->tabWidget);
    d->faceFilterView   = new TagFilterView(box5, d->faceFilterModel);
    d->faceFilterView->setObjectName(QLatin1String("ItemIconViewFaceTagFilterView"));

    d->faceFilterView->filteredModel()->listOnlyTagsWithProperty(TagPropertyName::person());
    d->faceFilterView->filteredModel()->setFilterBehavior(AlbumFilterModel::StrictFiltering);

    d->faceFilterSearchBar = new SearchTextBarDb(box5, QLatin1String("ItemIconViewFaceTagFilterSearchBar"));
    d->faceFilterSearchBar->setModel(d->faceFilterView->filteredModel(),
                                     AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->faceFilterSearchBar->setFilterModel(d->faceFilterView->albumFilterModel());

    const QString notfaceTaggedTitle = i18n("Images Without Face tags");
    d->withoutFaceCheckBox           = new QCheckBox(notfaceTaggedTitle, box5);
    d->withoutFaceCheckBox->setWhatsThis(i18n("Show images without a face tag."));

    d->faceOptionsBtn = new QToolButton(box5);
    d->faceOptionsBtn->setToolTip(i18n("Face tags Matching Condition"));
    d->faceOptionsBtn->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    d->faceOptionsBtn->setPopupMode(QToolButton::InstantPopup);
    d->faceOptionsBtn->setWhatsThis(i18n("Defines in which way the selected tags are combined "
                                         "to filter the images. This also includes the '%1' check box.",
                                         notfaceTaggedTitle));

    d->faceOptionsMenu = new QMenu(d->faceOptionsBtn);
    d->faceOptionsMenu->addAction(d->tagOrCondAction);
    d->faceOptionsMenu->addAction(d->tagAndCondAction);
    d->faceOptionsBtn->setMenu(d->faceOptionsMenu);

    QGridLayout* const lay5 = new QGridLayout(box5);
    lay5->addWidget(d->faceFilterView,      0, 0, 1, 3);
    lay5->addWidget(d->faceFilterSearchBar, 1, 0, 1, 3);
    lay5->addWidget(d->withoutFaceCheckBox, 2, 0, 1, 1);
    lay5->addWidget(d->faceOptionsBtn,      2, 2, 1, 1);
    lay5->setRowStretch(0, 100);
    lay5->setColumnStretch(1, 10);
    lay5->setContentsMargins(QMargins());
    lay5->setSpacing(0);

    d->tabWidget->insertTab(Private::FacesTab, box5, i18nc("@title", "People"));

    // ---

    connect(d->withoutFaceCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(slotWithoutTagChanged(int)));

    connect(d->faceOptionsMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotTagOptionsTriggered(QAction*)));

    connect(d->faceOptionsMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotTagOptionsMenu()));
}

} // namespace Digikam
