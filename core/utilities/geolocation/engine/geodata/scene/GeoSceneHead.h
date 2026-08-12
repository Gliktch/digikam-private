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

#include "GeoDocument.h"
#include "digikam_export.h"

class QString;

namespace Marble
{

class GeoSceneIcon;
class GeoSceneZoom;
class GeoSceneLicense;
class GeoSceneHeadPrivate;

/**
 * @short General properties and identifiers of a GeoScene document.
 */
class DIGIKAM_EXPORT GeoSceneHead : public GeoNode
{
public:

    GeoSceneHead();
    ~GeoSceneHead()                           override;

    const char* nodeType()              const override;

    QString name()                      const;
    void setName(const QString& name);
    QString target()                    const;
    void setTarget(const QString& target);
    QString theme()                     const;
    void setTheme(const QString& theme);

    /**
     * Planet radius, 0.0 if unknown
     */
    qreal radius()                      const;
    void setRadius(qreal radius);

    QString mapThemeId()                const;

    QString description()               const;
    void setDescription(const QString&);

    bool visible()                      const;
    void setVisible(bool visible);

    const GeoSceneZoom* zoom()          const;
    GeoSceneZoom* zoom();

    const GeoSceneIcon* icon()          const;
    GeoSceneIcon* icon();

    const GeoSceneLicense* license()    const;
    GeoSceneLicense* license();

private:

    Q_DISABLE_COPY(GeoSceneHead)

    GeoSceneHeadPrivate* const d = nullptr;
};

} // namespace Marble
