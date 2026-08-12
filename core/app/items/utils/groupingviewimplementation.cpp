/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-11-02
 * Description : Implementation of grouping specific functions for views
 *
 * SPDX-FileCopyrightText: 2017 by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "groupingviewimplementation.h"

// Qt includes

#include <QMimeData>

// Local includes

#include "iteminfolist.h"

namespace Digikam
{

bool GroupingViewImplementation::needGroupResolving(OperationType type, const ItemInfoList& infos) const
{
    ApplicationSettings::ApplyToEntireGroup applyAll = ApplicationSettings::instance()->getGroupingOperateOnAll(type);

    if (applyAll == ApplicationSettings::No)
    {
        return false;
    }

    for (const ItemInfo& info : std::as_const(infos))
    {
        if (hasHiddenGroupedImages(info))
        {
            if (applyAll == ApplicationSettings::Yes)
            {
                return true;
            }

            return ApplicationSettings::instance()->askGroupingOperateOnAll(type);
        }
    }

    return false;
}

ItemInfoList GroupingViewImplementation::resolveGrouping(const ItemInfoList& infos) const
{
    ItemInfoList outInfos;

    for (const ItemInfo& info : std::as_const(infos))
    {
        outInfos << info;

        if (hasHiddenGroupedImages(info))
        {
            outInfos << info.groupedImages();
        }
    }

    return outInfos;
}

ItemInfoList GroupingViewImplementation::getHiddenGroupedInfos(const ItemInfoList& infos) const
{
    ItemInfoList outInfos;

    for (const ItemInfo& info : std::as_const(infos))
    {
        if (hasHiddenGroupedImages(info))
        {
            outInfos << info.groupedImages();
        }
    }

    return outInfos;
}

} // namespace Digikam
