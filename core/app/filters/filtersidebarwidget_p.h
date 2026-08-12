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

#include "filtersidebarwidget.h"

// Qt includes

#include <QActionGroup>
#include <QLayout>
#include <QCheckBox>
#include <QGridLayout>
#include <QToolButton>
#include <QMenu>
#include <QIcon>
#include <QLabel>
#include <QScrollArea>
#include <QTabWidget>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "applicationsettings.h"
#include "colorlabelfilter.h"
#include "geolocationfilter.h"
#include "picklabelfilter.h"
#include "ratingfilter.h"
#include "mimefilter.h"
#include "tagfilterview.h"
#include "searchtextbardb.h"
#include "facetags.h"

namespace Digikam
{

class Q_DECL_HIDDEN FilterSideBarWidget::Private
{
public:

    enum PropertiesFilter
    {
        TEXT = 0,
        MIME,
        GEOLOCATION,
        LABELS
    };

    enum FilterTabs
    {
        PropertiesTab = 0,
        TagsTab,
        FacesTab
    };

public:

    Private() = default;

public:

    const QString                          configSearchTextFilterFieldsEntry    = QLatin1String("Search Text Filter Fields");
    const QString                          configLastShowUntaggedEntry          = QLatin1String("Show Untagged");
    const QString                          configLastShowWithoutFaceEntry       = QLatin1String("Show Without Face");
    const QString                          configMatchingConditionEntry         = QLatin1String("Matching Condition");

    TagFilterView*                         tagFilterView                        = nullptr;
    SearchTextBarDb*                       tagFilterSearchBar                   = nullptr;
    QToolButton*                           tagOptionsBtn                        = nullptr;
    QMenu*                                 tagOptionsMenu                       = nullptr;
    TagModel*                              tagFilterModel                       = nullptr;
    QAction*                               tagOrCondAction                      = nullptr;
    QAction*                               tagAndCondAction                     = nullptr;
    ItemFilterSettings::MatchingCondition  tagMatchCond                         = ItemFilterSettings::OrCondition;

    TagFilterView*                         faceFilterView                       = nullptr;
    SearchTextBarDb*                       faceFilterSearchBar                  = nullptr;
    QToolButton*                           faceOptionsBtn                       = nullptr;
    QMenu*                                 faceOptionsMenu                      = nullptr;
    TagModel*                              faceFilterModel                      = nullptr;

    ColorLabelFilter*                      colorLabelFilter                     = nullptr;
    GeolocationFilter*                     geolocationFilter                    = nullptr;
    PickLabelFilter*                       pickLabelFilter                      = nullptr;
    RatingFilter*                          ratingFilter                         = nullptr;
    MimeFilter*                            mimeFilter                           = nullptr;
    TextFilter*                            textFilter                           = nullptr;

    QCheckBox*                             withoutTagCheckBox                   = nullptr;
    QCheckBox*                             withoutFaceCheckBox                  = nullptr;

    QTabWidget*                            tabWidget                            = nullptr;
};

} // namespace Digikam
