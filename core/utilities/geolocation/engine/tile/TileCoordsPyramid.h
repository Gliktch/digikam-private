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

// Qt includes

#include <QtGlobal>

// Local includes

#include "digikam_export.h"

class QRect;

namespace Marble
{

class DIGIKAM_EXPORT TileCoordsPyramid
{
public:

    TileCoordsPyramid(int const topLevel, int const bottomLevel);
    explicit TileCoordsPyramid(TileCoordsPyramid const& other);
    TileCoordsPyramid();
    TileCoordsPyramid& operator=(TileCoordsPyramid const& rhs);
    ~TileCoordsPyramid();

    int topLevel() const;
    int bottomLevel() const;
    void setBottomLevelCoords(QRect const& coords);
    QRect coords(int const level) const;

    void setValidTileLevels(const QVector<int>& validLevels);
    QVector<int> validTileLevels();

    /**
      * @brief returns the number of tiles covered by one pyramid
      **/
    qint64 tilesCount() const;

private:

    void swap(TileCoordsPyramid& other);

private:

    class Private;
    Private* d = nullptr; // not const, needs to be swapable
};

} // namespace Marble
