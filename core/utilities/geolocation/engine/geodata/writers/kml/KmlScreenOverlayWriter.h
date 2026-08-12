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

#include "GeoDataVec2.h"
#include "KmlOverlayTagWriter.h"
#include "MarbleGlobal.h"

namespace Marble
{

class KmlScreenOverlayWriter : public KmlOverlayTagWriter
{
public:

    KmlScreenOverlayWriter();
    bool writeMid(const GeoNode* node, GeoWriter& writer) const override;

private:

    static void writeVec2(const QString& element, const GeoDataVec2& vec2, GeoWriter& writer);
    static QString unitToString(GeoDataVec2::Unit unit);
};

} // namespace Marble
