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

#include <QList>

// Local includes

#include "GeoDataObject.h"
#include "GeoDataTourPrimitive.h"

namespace Marble
{

class DIGIKAM_EXPORT GeoDataPlaylist : public GeoDataObject
{
public:

    bool operator==(const GeoDataPlaylist& other)                       const;
    bool operator!=(const GeoDataPlaylist& other)                       const;
    const char* nodeType()                                              const override;

    GeoDataTourPrimitive* primitive(int index);
    const GeoDataTourPrimitive* primitive(int index)                    const;
    void addPrimitive(GeoDataTourPrimitive* primitive);
    void insertPrimitive(int index, GeoDataTourPrimitive* primitive);
    void removePrimitiveAt(int index);
    void swapPrimitives(int indexA, int indexB);

    int size()                                                          const;

private:

    QList<GeoDataTourPrimitive*> m_primitives;
};

} // namespace Marble
