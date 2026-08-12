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

void FilterSideBarWidget::populatePropertiesView()
{
    QScrollArea* const sv         = new QScrollArea(d->tabWidget);
    sv->setFrameStyle(QFrame::NoFrame);
    sv->setWidgetResizable(true);

    QWidget* const propertiesView = new QWidget(sv->viewport());
    QGridLayout* const grid       = new QGridLayout(propertiesView);
    sv->setWidget(propertiesView);

    // ---

    QLabel* const textLbl = new QLabel(i18n("Text Filter:"), propertiesView);
    d->textFilter         = new TextFilter(propertiesView);

    // ---

    QLabel* const mimeLbl = new QLabel(i18n("MIME Type Filter:"), propertiesView);
    d->mimeFilter         = new MimeFilter(propertiesView);

    // ---

    QLabel* const geoLbl  = new QLabel(i18n("Geolocation Filter:"), propertiesView);
    d->geolocationFilter  = new GeolocationFilter(propertiesView);

    // ---

    QLabel* const labelsLbl = new QLabel(i18n("Labels Filter:"), propertiesView);
    d->colorLabelFilter     = new ColorLabelFilter(propertiesView);
    d->pickLabelFilter      = new PickLabelFilter(propertiesView);
    d->ratingFilter         = new RatingFilter(propertiesView);

    // ---

    grid->addWidget(textLbl,              0, 0, 1, 2);
    grid->addWidget(d->textFilter,        1, 0, 1, 2);
    grid->addWidget(mimeLbl,              2, 0, 1, 2);
    grid->addWidget(d->mimeFilter,        3, 0, 1, 2);
    grid->addWidget(geoLbl,               4, 0, 1, 2);
    grid->addWidget(d->geolocationFilter, 5, 0, 1, 2);
    grid->addWidget(labelsLbl,            6, 0, 1, 2);
    grid->addWidget(d->colorLabelFilter,  7, 0, 1, 2, Qt::AlignRight);
    grid->addWidget(d->pickLabelFilter,   8, 0, 1, 2, Qt::AlignRight);
    grid->addWidget(d->ratingFilter,      9, 0, 1, 2, Qt::AlignRight);
    grid->setRowStretch(10, 10);
    grid->setContentsMargins(layoutSpacing(), layoutSpacing(), layoutSpacing(), layoutSpacing());
    grid->setSpacing(layoutSpacing());

    d->tabWidget->insertTab(Private::PropertiesTab, sv, i18nc("@title", "Properties"));

    // ---

    connect(d->mimeFilter, SIGNAL(activated(int)),
            this, SIGNAL(signalMimeTypeFilterChanged(int)));

    connect(d->geolocationFilter, SIGNAL(signalFilterChanged(ItemFilterSettings::GeolocationCondition)),
            this, SIGNAL(signalGeolocationFilterChanged(ItemFilterSettings::GeolocationCondition)));

    connect(d->textFilter, SIGNAL(signalSearchTextFilterSettings(SearchTextFilterSettings)),
            this, SIGNAL(signalSearchTextFilterChanged(SearchTextFilterSettings)));

    connect(d->colorLabelFilter, SIGNAL(signalColorLabelSelectionChanged(QList<ColorLabel>)),
            this, SLOT(slotColorLabelFilterChanged(QList<ColorLabel>)));

    connect(d->pickLabelFilter, SIGNAL(signalPickLabelSelectionChanged(QList<PickLabel>)),
            this, SLOT(slotPickLabelFilterChanged(QList<PickLabel>)));

    connect(d->ratingFilter, SIGNAL(signalRatingFilterChanged(int,ItemFilterSettings::RatingCondition,bool)),
            this, SIGNAL(signalRatingFilterChanged(int,ItemFilterSettings::RatingCondition,bool)));
}

void FilterSideBarWidget::setFocusToTextFilter()
{
    d->textFilter->searchTextBar()->setFocus();
}

void FilterSideBarWidget::slotFilterMatchesForText(bool match)
{
    d->textFilter->searchTextBar()->slotSearchResult(match);
}

void FilterSideBarWidget::slotColorLabelFilterChanged(const QList<ColorLabel>& list)
{
    Q_UNUSED(list);

    checkFilterChanges();
}

void FilterSideBarWidget::slotPickLabelFilterChanged(const QList<PickLabel>& list)
{
    Q_UNUSED(list);

    checkFilterChanges();
}

} // namespace Digikam
