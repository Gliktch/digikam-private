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

void FilterSideBarWidget::populateTagsView()
{
    QWidget* const box3   = new QWidget(d->tabWidget);
    d->tagFilterView      = new TagFilterView(box3, d->tagFilterModel);
    d->tagFilterView->setObjectName(QLatin1String("ItemIconViewTagFilterView"));

    d->tagFilterSearchBar = new SearchTextBarDb(box3, QLatin1String("ItemIconViewTagFilterSearchBar"));
    d->tagFilterSearchBar->setModel(d->tagFilterView->filteredModel(),
                                    AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->tagFilterSearchBar->setFilterModel(d->tagFilterView->albumFilterModel());

    const QString notTaggedTitle = i18n("Images Without Tags");
    d->withoutTagCheckBox        = new QCheckBox(notTaggedTitle, box3);
    d->withoutTagCheckBox->setWhatsThis(i18n("Show images without a tag."));

    d->tagOptionsBtn = new QToolButton(box3);
    d->tagOptionsBtn->setToolTip(i18n("Tags Matching Condition"));
    d->tagOptionsBtn->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    d->tagOptionsBtn->setPopupMode(QToolButton::InstantPopup);
    d->tagOptionsBtn->setWhatsThis(i18n("Defines in which way the selected tags are combined "
                                        "to filter the images. This also includes the '%1' check box.",
                                        notTaggedTitle));

    d->tagOptionsMenu                = new QMenu(d->tagOptionsBtn);
    QActionGroup* const tagCondGroup = new QActionGroup(d->tagOptionsMenu);
    tagCondGroup->setExclusive(true);
    d->tagOrCondAction  = d->tagOptionsMenu->addAction(i18n("OR"));
    d->tagOrCondAction->setActionGroup(tagCondGroup);
    d->tagOrCondAction->setCheckable(true);
    d->tagAndCondAction = d->tagOptionsMenu->addAction(i18n("AND"));
    d->tagAndCondAction->setActionGroup(tagCondGroup);
    d->tagAndCondAction->setCheckable(true);
    d->tagOptionsBtn->setMenu(d->tagOptionsMenu);

    QGridLayout* const lay3 = new QGridLayout(box3);
    lay3->addWidget(d->tagFilterView,      0, 0, 1, 3);
    lay3->addWidget(d->tagFilterSearchBar, 1, 0, 1, 3);
    lay3->addWidget(d->withoutTagCheckBox, 2, 0, 1, 1);
    lay3->addWidget(d->tagOptionsBtn,      2, 2, 1, 1);
    lay3->setRowStretch(0, 100);
    lay3->setColumnStretch(1, 10);
    lay3->setContentsMargins(QMargins());
    lay3->setSpacing(0);

    d->tabWidget->insertTab(Private::TagsTab, box3, i18nc("@title", "Tags"));

    // ---

    connect(d->tagFilterView, SIGNAL(checkedTagsChanged(QList<TAlbum*>,QList<TAlbum*>)),
            this, SLOT(slotCheckedTagsChanged(QList<TAlbum*>,QList<TAlbum*>)));

    connect(d->withoutTagCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(slotWithoutTagChanged(int)));

    connect(d->tagOptionsMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotTagOptionsMenu()));

    connect(d->tagOptionsMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotTagOptionsTriggered(QAction*)));
}

void FilterSideBarWidget::slotTagOptionsMenu()
{
    d->tagOrCondAction->setChecked(false);
    d->tagAndCondAction->setChecked(false);

    switch (d->tagMatchCond)
    {
        case ItemFilterSettings::OrCondition:
        {
            d->tagOrCondAction->setChecked(true);
            break;
        }

        case ItemFilterSettings::AndCondition:
        {
            d->tagAndCondAction->setChecked(true);
            break;
        }
    }
}

void FilterSideBarWidget::slotTagOptionsTriggered(QAction* action)
{
    if (action)
    {
        if      (action == d->tagOrCondAction)
        {
            d->tagMatchCond = ItemFilterSettings::OrCondition;
        }
        else if (action == d->tagAndCondAction)
        {
            d->tagMatchCond = ItemFilterSettings::AndCondition;
        }
    }

    checkFilterChanges();
}

void FilterSideBarWidget::slotCheckedTagsChanged(const QList<TAlbum*>& includedTags,
                                                 const QList<TAlbum*>& excludedTags)
{
    Q_UNUSED(includedTags);
    Q_UNUSED(excludedTags);

    checkFilterChanges();
}

void FilterSideBarWidget::slotWithoutTagChanged(int newState)
{
    Q_UNUSED(newState);

    checkFilterChanges();
}

} // namespace Digikam
