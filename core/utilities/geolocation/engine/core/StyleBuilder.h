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

#include <QHash>
#include <QPair>

// Local includes

#include "GeoDataPlacemark.h"
#include "GeoDataStyle.h"
#include "GeoDataRelation.h"
#include "digikam_export.h"

class QFont;
class QColor;

namespace Marble
{

class OsmPlacemarkData;

class DIGIKAM_EXPORT StyleParameters
{
public:

    explicit StyleParameters(const GeoDataPlacemark* placemark_ = nullptr, int tileLevel = 0);

    const GeoDataPlacemark* placemark;
    int                     tileLevel = 0;
    const GeoDataRelation*  relation;
};

enum StyleEffect
{
    NoEffect,
    InvertedEffect,
    GrayscaleEffect,
    RedModeEffect
};

class DIGIKAM_EXPORT StyleBuilder
{
public:

    typedef QPair<QString, QString> OsmTag;

public:

    StyleBuilder();
    ~StyleBuilder();

public:

    /**
     * Return the label font of the placemark.
     */
    QFont defaultFont()                                                     const;
    void setDefaultFont(const QFont& font);

    QColor defaultLabelColor()                                              const;
    void setDefaultLabelColor(const QColor& color);

    GeoDataStyle::ConstPtr createStyle(const StyleParameters& parameters)   const;

    /**
     * @brief Returns the order in which the visual categories used in the theme shall be painted on the map.
     * @return order in which the visual categories shall be painted on the map
     */
    QStringList renderOrder()                                               const;

    void reset();

    /**
     * @brief Returns the zoom level from which on the given visual category will be visible.
     * @param placemark the placemark for which the minimum zoom level shall be returned
     * @return zoom level from which on the given visual category will be visible
     */
    int minimumZoomLevel(const GeoDataPlacemark& placemark)                 const;

    static int minimumZoomLevel(const GeoDataPlacemark::GeoDataVisualCategory& visualCategory);

    static qint64 popularity(const GeoDataPlacemark* placemark);

    /**
     * @brief Returns the maximum zoom level in the theme.
     * @return maximum zoom level in the theme
     */
    int maximumZoomLevel()                                                  const;

    static QString visualCategoryName(GeoDataPlacemark::GeoDataVisualCategory category);

    static QColor effectColor(const QColor& color);
    static StyleEffect styleEffect();
    static void setStyleEffect(StyleEffect effect);

    /**
     * @brief Mapping between osm key=value pairs and visual categories
     */
    static QHash<OsmTag, GeoDataPlacemark::GeoDataVisualCategory> osmTagMapping();

    static QStringList shopValues();
    static QSet<OsmTag> buildingTags();

    static GeoDataPlacemark::GeoDataVisualCategory determineVisualCategory(const OsmPlacemarkData& osmData);

private:

    Q_DISABLE_COPY(StyleBuilder)

private:

    class Private;
    Private* const d = nullptr;
};

} // Namespace Marble
