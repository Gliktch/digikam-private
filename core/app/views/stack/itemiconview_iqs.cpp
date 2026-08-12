/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Item icon view interface - Image Quality Sorting.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemiconview_p.h"

// Local includes

#include "imagequalitysorter.h"

namespace Digikam
{

void ItemIconView::slotImageQualitySorter()
{
    QStringList paths;
    const auto urls = selectedUrls();

    for (const auto& url : urls)
    {
        paths << url.toLocalFile();
    }

    ImageQualitySettings settings;
    settings.readFromConfig();

    ImageQualitySorter* const tool = new ImageQualitySorter(paths, settings);
    tool->start();
}

} // namespace Digikam
