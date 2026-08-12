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

#include <QObject>

// Local includes

#include "GeoDocument.h"
#include "digikam_export.h"

namespace Marble
{

class GeoSceneHead;
class GeoSceneLegend;
class GeoSceneMap;
class GeoSceneSettings;
class GeoSceneDocumentPrivate;

/**
 * @short A container for features parsed from the DGML file.
 */
class DIGIKAM_EXPORT GeoSceneDocument : public QObject,
                                        public GeoDocument,
                                        public GeoNode
{
    Q_OBJECT

public:

    GeoSceneDocument();
    ~GeoSceneDocument()                               override;

    const char* nodeType()                      const override;

    bool isGeoSceneDocument()                   const override
    {
        return true;
    }

    const GeoSceneHead* head()                  const;
    GeoSceneHead* head();

    const GeoSceneMap* map()                    const;
    GeoSceneMap* map();

    const GeoSceneSettings* settings()          const;
    GeoSceneSettings* settings();

    const GeoSceneLegend* legend()              const;
    GeoSceneLegend* legend();

Q_SIGNALS:

    void valueChanged(const QString&, bool);

private:

    Q_DISABLE_COPY(GeoSceneDocument)

    GeoSceneDocumentPrivate* const d = nullptr;
};

} // namespace Marble
