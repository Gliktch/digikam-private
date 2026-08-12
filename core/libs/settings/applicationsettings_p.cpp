/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : application settings interface
 *
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "applicationsettings_p.h"

namespace Digikam
{

ApplicationSettings::OperationStrings ApplicationSettings::Private::createConfigGroupingOperateOnAll()
{
    ApplicationSettings::OperationStrings out;
    out.insert(MetadataOps,     QLatin1String("Do metadata operations on all"));
    out.insert(ImportExportOps, QLatin1String("Do Import Export operations on all"));
    out.insert(BQMOps,          QLatin1String("Do BQM operations on all"));
    out.insert(LightTableOps,   QLatin1String("Do light table operations on all"));
    out.insert(SlideshowOps,    QLatin1String("Do slideshow operations on all"));
    out.insert(RenameOps,       QLatin1String("Rename all"));
    out.insert(ToolsOps,        QLatin1String("Operate tools on all"));

    return out;
}

ApplicationSettings::Private::Private(ApplicationSettings* const qq)
    : q(qq)
{
    for (int i = 0 ; i != UnspecifiedOps ; ++i)
    {
        groupingOperateOnAll.insert((OperationType)i,
                ApplicationSettings::Ask);
    }
}

void ApplicationSettings::Private::init()
{
    albumCategoryNames.clear();
    albumCategoryNames.append(i18nc("@item: album categories", "Category"));
    albumCategoryNames.append(i18nc("@item: album categories", "Travel"));
    albumCategoryNames.append(i18nc("@item: album categories", "Holidays"));
    albumCategoryNames.append(i18nc("@item: album categories", "Friends"));
    albumCategoryNames.append(i18nc("@item: album categories", "Nature"));
    albumCategoryNames.append(i18nc("@item: album categories", "Party"));
    albumCategoryNames.append(i18nc("@item: album categories", "Todo"));
    albumCategoryNames.append(i18nc("@item: album categories", "Miscellaneous"));
    albumCategoryNames.sort();

    for (int i = 0 ; i != UnspecifiedOps ; ++i)
    {
        groupingOperateOnAll.insert((OperationType)i,
                                    ApplicationSettings::Ask);
    }

    q->connect(q, SIGNAL(balooSettingsChanged()),
               q, SLOT(applyBalooSettings()));
}

} // namespace Digikam
