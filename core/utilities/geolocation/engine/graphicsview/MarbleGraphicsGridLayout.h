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

#include <Qt>

// Local includes

#include "AbstractMarbleGraphicsLayout.h"
#include "digikam_export.h"

namespace Marble
{

class ScreenGraphicsItem;
class MarbleGraphicsGridLayoutPrivate;

class DIGIKAM_EXPORT MarbleGraphicsGridLayout : public AbstractMarbleGraphicsLayout
{
public:

    MarbleGraphicsGridLayout(int rows, int columns);
    ~MarbleGraphicsGridLayout()                                       override;

    void addItem(ScreenGraphicsItem* item, int row, int column);

    /**
     * This updates the positions of all items in the layout.
     * Calling this will be done on every repainting, so you won't have to do it yourself.
     */
    void updatePositions(MarbleGraphicsItem* parent)                  override;

    Qt::Alignment alignment()                                   const;

    Qt::Alignment alignment(ScreenGraphicsItem* item)           const;

    void setAlignment(Qt::Alignment align);

    void setAlignment(ScreenGraphicsItem* item, Qt::Alignment);

    /**
     * Returns the spacing between the items inside the layout.
     */
    int spacing()                                               const;

    /**
     * Set the spacing between the items inside the layout. By default the spacing is 0.
     */
    void setSpacing(int spacing);

private:

    MarbleGraphicsGridLayout(const MarbleGraphicsGridLayout&)            = delete;
    MarbleGraphicsGridLayout& operator=(const MarbleGraphicsGridLayout&) = delete;

private:

    MarbleGraphicsGridLayoutPrivate* const d = nullptr;
};

} // namespace Marble
