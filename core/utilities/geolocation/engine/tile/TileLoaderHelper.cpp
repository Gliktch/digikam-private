/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "TileLoaderHelper.h"

// C++ includes

#include <cmath>

// Local includes

#include "MarbleGlobal.h"
#include "digikam_debug.h"

namespace Marble
{

int TileLoaderHelper::levelToRow(int levelZeroRows, int level)
{
    if (level < 0)
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG) << QString::fromUtf8("TileLoaderHelper::levelToRow(): Invalid level: %1")
                                    .arg(level);
        return 0;
    }

    return levelZeroRows << level;
}

int TileLoaderHelper::levelToColumn(int levelZeroColumns, int level)
{
    if (level < 0)
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG) << QString::fromUtf8("TileLoaderHelper::levelToColumn(): Invalid level: %1")
                                    .arg(level);
        return 0;
    }

    return levelZeroColumns << level;
}

int TileLoaderHelper::rowToLevel(int levelZeroRows, int row)
{
    if (row < levelZeroRows)
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG) << QString::fromUtf8("TileLoaderHelper::rowToLevel(): Invalid number of rows: %1")
                                    .arg(row);
        return 0;
    }

    return (int)(std::log((qreal)(row / levelZeroRows)) / std::log((qreal)2.0));
}

int TileLoaderHelper::columnToLevel(int levelZeroColumns, int column)
{
    if (column < levelZeroColumns)
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG) << QString::fromUtf8("TileLoaderHelper::columnToLevel(): Invalid number of columns: %1")
                                    .arg(column);
        return 0;
    }

    return (int)(std::log((qreal)(column / levelZeroColumns)) / std::log((qreal)2.0));
}

} // namespace Marble
