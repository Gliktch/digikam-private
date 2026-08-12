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

#include <QImage>

// Local includes

#include "Tile.h"

namespace Marble
{

class Blending;
class TileId;

/*!
  @class TextureTile
  @brief A class that resembles an image tile (extends Tile).

  This tile provides a bitmap image for a certain (geographic) area and
  for a given zoom level. Each Tile can be identified via a unique
  TileId.

  A stack of Tiles that cover the same area and the same
  zoom level can be stored (and painted) layer by layer in a StackedTile object.
  For this purpose each Tile specifies a blending type.

  Usually the tiles are organized in so called quad tiles: This means that
  with increasing zoom level four other tiles cover the same area as a
  single "parent" tile in the previous zoom level. These four tiles have
  the same pixel size as the "parent" tile.

  The process of "filling the tile with data is done in stages: The
  State describes the current progress of loading the data (Empty, Scaled,
  Expired, StateUptodate).

  The life time cycle of a Tile can also be influenced by its
  expiration time which will trigger a reload of the tile data.
 */
class TextureTile : public Tile
{
public:

    TextureTile(TileId const& tileId, QImage const& image, const Blending* blending);
    ~TextureTile() override;

    /*!
     @brief Returns the QImage that describes the look of the Tile
     \return A non-zero pointer to a QImage associated with the tile.
    */
    QImage const* image() const;

    /*!
     @brief Returns the kind of blending used for the texture tile.
     \return A pointer to the blending object used for painting/merging the Tile.

     If no blending is set the pointer returned will be zero.
    */
    Blending const* blending() const;

    int byteCount() const;

private:

    Q_DISABLE_COPY(TextureTile)

private:

    QImage const           m_image;
    Blending const* const m_blending = nullptr;

};

inline QImage const* TextureTile::image() const
{
    return &m_image;
}

inline Blending const* TextureTile::blending() const
{
    return m_blending;
}

inline int TextureTile::byteCount() const
{
    return m_image.sizeInBytes();
}

} // namespace Marble
