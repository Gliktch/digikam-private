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

#include <QVector>

// Local includes

#include "GeoDocument.h"
#include "digikam_export.h"

namespace Marble
{

class GeoSceneSection;
class GeoSceneLegendPrivate;

/**
 * @short Legend of a GeoScene document.
 */
class DIGIKAM_EXPORT GeoSceneLegend : public GeoNode
{
public:

    GeoSceneLegend();
    ~GeoSceneLegend() override;

    const char* nodeType() const override;

    /**
     * @brief  Add a section to the legend
     * @param  section  the new section
     */
    void addSection(const GeoSceneSection*);
    QVector<const GeoSceneSection*> sections() const;

private:

    Q_DISABLE_COPY(GeoSceneLegend)

private:

    GeoSceneLegendPrivate* const d = nullptr;
};

} // namespace Marble
