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

#pragma once

// Local includes

#include "digikam_export.h"

namespace Marble
{

namespace TileLoaderHelper
{
/**
 * @brief Get the maximum number of tile rows for a given tile level.
 * @param levelZeroRows  the number of rows in level zero
 * @param level  the tile level
 * @return       the maximum number of rows that a map level was tiled into.
 *               If the tile level number is invalid then "-1" gets
 *               returned so this case of wrong input data can get caught
 *               by the code which makes use of it.
 */
DIGIKAM_EXPORT int levelToRow(int levelZeroRows, int level);

/**
 * @brief Get the maximum number of tile columns for a given tile level.
 * @param levelZeroColumns  the number of columns in level zero
 * @param level  the tile level
 * @return       the maximum number of columns that a map level was tiled into.
 *               If the tile level number is invalid then "-1" gets
 *               returned so this case of wrong input data can get caught
 *               by the code which makes use of it.
 */
DIGIKAM_EXPORT int levelToColumn(int levelZeroColumns, int level);

/**
 * @brief Get the tile level for the given maximum number of tile columns.
 * @param levelZeroRows  the number of rows in level zero
 * @param row    the maximum number of rows that a map level was tiled into.
 * @return       the corresponding tile level.
 *               If the number of rows is invalid then "-1" gets
 *               returned so this case of wrong input data can get caught
 *               by the code which makes use of it.
 */
DIGIKAM_EXPORT int rowToLevel(int levelZeroRows, int row);

/**
 * @brief Get the tile level for the given maximum number of tile columns.
 * @param levelZeroColumns  the number of columns in level zero
 * @param column the maximum number of columns that a map level was tiled into.
 * @return       the corresponding tile level.
 *               If the number of columns is invalid then "-1" gets
 *               returned so this case of wrong input data can get caught
 *               by the code which makes use of it.
 */
DIGIKAM_EXPORT int columnToLevel(int levelZeroColumns, int column);
}

} // namespace Marble
