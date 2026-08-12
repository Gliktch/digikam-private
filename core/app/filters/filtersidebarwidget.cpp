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

FilterSideBarWidget::FilterSideBarWidget(QWidget* const parent, TagModel* const tagFilterModel)
    : DVBox            (parent),
      StateSavingObject(this),
      d                (new Private)
{
    setObjectName(QLatin1String("TagFilter Sidebar"));

    d->tabWidget       = new QTabWidget(this);
    d->tagFilterModel  = tagFilterModel;
    d->faceFilterModel = tagFilterModel;

    populatePropertiesView();
    populateTagsView();
    populateFacesView();
}

FilterSideBarWidget::~FilterSideBarWidget()
{
    delete d;
}

void FilterSideBarWidget::slotResetFilters()
{
    d->textFilter->reset();
    d->mimeFilter->setMimeFilter(MimeFilter::AllFiles);
    d->geolocationFilter->setGeolocationFilter(ItemFilterSettings::GeolocationNoFilter);
    d->tagFilterView->slotResetCheckState();
    d->withoutTagCheckBox->setChecked(false);
    d->withoutFaceCheckBox->setChecked(false);
    d->colorLabelFilter->reset();
    d->pickLabelFilter->reset();
    d->ratingFilter->reset();
    d->tagMatchCond = ItemFilterSettings::OrCondition;
}

void FilterSideBarWidget::checkFilterChanges()
{
    bool showUntagged = d->withoutTagCheckBox->isChecked();

    QList<int> includedTagIds;
    QList<int> excludedTagIds;
    QList<int> clTagIds;
    QList<int> plTagIds;

    if (!showUntagged || (d->tagMatchCond == ItemFilterSettings::OrCondition))
    {
        {
            const auto tags = d->tagFilterView->getCheckedTags();

            for (const TAlbum* const tag : tags)
            {
                if (tag)
                {
                    includedTagIds << tag->id();
                }
            }
        }

        {
            const auto tags = d->tagFilterView->getPartiallyCheckedTags();

            for (const TAlbum* const tag : tags)
            {
                if (tag)
                {
                    excludedTagIds << tag->id();
                }
            }
        }

        if (d->withoutFaceCheckBox->isChecked())
        {
            excludedTagIds << FaceTags::allPersonTags();
        }
    }

    {
        const auto tags = d->colorLabelFilter->getCheckedColorLabelTags();

        for (const TAlbum* const tag : tags)
        {
            if (tag)
            {
                clTagIds << tag->id();
            }
        }
    }

    {
        const auto tags = d->pickLabelFilter->getCheckedPickLabelTags();

        for (const TAlbum* const tag : tags)
        {
            if (tag)
            {
                plTagIds << tag->id();
            }
        }
    }

    Q_EMIT signalTagFilterChanged(includedTagIds,
                                  excludedTagIds,
                                  d->tagMatchCond,
                                  showUntagged,
                                  clTagIds,
                                  plTagIds);
}

void FilterSideBarWidget::setConfigGroup(const KConfigGroup& group)
{
    StateSavingObject::setConfigGroup(group);
    d->tagFilterView->setConfigGroup(group);
}

void FilterSideBarWidget::doLoadState()
{
    /// @todo mime type and geolocation filter states are not loaded/saved

    KConfigGroup group = getConfigGroup();

    d->textFilter->setsearchTextFields((SearchTextFilterSettings::TextFilterFields)
                                       (group.readEntry(entryName(d->configSearchTextFilterFieldsEntry),
                                                                   (int)SearchTextFilterSettings::All)));


    d->ratingFilter->setRatingFilterCondition((ItemFilterSettings::RatingCondition)
                                              (ApplicationSettings::instance()->getRatingFilterCond()));

    d->tagMatchCond = (ItemFilterSettings::MatchingCondition)
                      (group.readEntry(entryName(d->configMatchingConditionEntry),
                                                  (int)ItemFilterSettings::OrCondition));

    d->tagFilterView->loadState();
    d->faceFilterView->loadState();

    if (d->tagFilterView->isRestoreCheckState())
    {
        d->withoutTagCheckBox->setChecked(group.readEntry(entryName(d->configLastShowUntaggedEntry), false));
    }

    if (d->faceFilterView->isRestoreCheckState())
    {
        d->withoutFaceCheckBox->setChecked(group.readEntry(entryName(d->configLastShowWithoutFaceEntry), false));
    }

    checkFilterChanges();
}

void FilterSideBarWidget::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    group.writeEntry(entryName(d->configSearchTextFilterFieldsEntry), (int)d->textFilter->searchTextFields());

    ApplicationSettings::instance()->setRatingFilterCond(d->ratingFilter->ratingFilterCondition());

    group.writeEntry(entryName(d->configMatchingConditionEntry), (int)d->tagMatchCond);

    d->tagFilterView->saveState();
    d->faceFilterView->saveState();

    group.writeEntry(entryName(d->configLastShowUntaggedEntry),    d->withoutTagCheckBox->isChecked());
    group.writeEntry(entryName(d->configLastShowWithoutFaceEntry), d->withoutFaceCheckBox->isChecked());
    group.sync();
}

} // namespace Digikam

#include "moc_filtersidebarwidget.cpp"
