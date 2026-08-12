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

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "itemfiltersettings.h"
#include "statesavingobject.h"
#include "textfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class TagModel;
class TAlbum;

/**
 * Sidebar widget containing the all filter widgets.
 *
 * @author jwienke
 */
class FilterSideBarWidget : public DVBox,
                            public StateSavingObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent the parent for qt parent child mechanism
     * @param tagFilterModel tag model to work on
     */
    explicit FilterSideBarWidget(QWidget* const parent, TagModel* const tagFilterModel);

    /**
     * Destructor.
     */
    ~FilterSideBarWidget()                         override;

    void setConfigGroup(const KConfigGroup& group) override;
    void doLoadState()                             override;
    void doSaveState()                             override;

    void setFocusToTextFilter();

Q_SIGNALS:

    /**
     * Emitted if the selected filter has changed.
     *
     * @param includedTags a list of included tag ids
     * @param excludedTags a list of excluded tag ids
     * @param matchingCond condition to join the selected tags
     * @param showUnTagged if this is true, only photos without a tag shall be shown
     * @param clTagIds     a list of color label tag ids
     * @param plTagIds     a list of pick label tag ids
     */
    void signalTagFilterChanged(const QList<int>& includedTags,
                                const QList<int>& excludedTags,
                                ItemFilterSettings::MatchingCondition matchingCond,
                                bool showUnTagged,
                                const QList<int>& clTagIds,
                                const QList<int>& plTagIds);

    void signalRatingFilterChanged(int, ItemFilterSettings::RatingCondition, bool);
    void signalMimeTypeFilterChanged(int);
    void signalGeolocationFilterChanged(ItemFilterSettings::GeolocationCondition);
    void signalSearchTextFilterChanged(const SearchTextFilterSettings&);

public Q_SLOTS:

    /**
     * Resets all selected filters.
     */
    void slotResetFilters();

    void slotFilterMatchesForText(bool);

private Q_SLOTS:

    void slotCheckedTagsChanged(const QList<TAlbum*>& includedTags,
                                const QList<TAlbum*>& excludedTags);
    void slotColorLabelFilterChanged(const QList<ColorLabel>&);
    void slotPickLabelFilterChanged(const QList<PickLabel>&);
    void slotWithoutTagChanged(int newState);
    void slotTagOptionsTriggered(QAction*);
    void slotTagOptionsMenu();

private:

    void populatePropertiesView();
    void populateTagsView();
    void populateFacesView();
    void checkFilterChanges();

private:

    class Private;
    Private* const d = nullptr;
};

} // nameSpace Digikam
