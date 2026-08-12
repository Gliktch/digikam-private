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

#include "GeoDataFeature.h"
#include "digikam_export.h"

class QColor;
class QImage;

namespace Marble
{

class GeoDataOverlayPrivate;

class DIGIKAM_EXPORT GeoDataOverlay: public GeoDataFeature
{
public:

    /** Constructor */
    GeoDataOverlay();

    ~GeoDataOverlay() override;

    GeoDataOverlay(const GeoDataOverlay& other);

    GeoDataOverlay& operator=(const GeoDataOverlay& other);

    QColor color() const;

    void setColor(const QColor& color);

    int drawOrder() const;

    void setDrawOrder(int order);

    QImage icon() const;

    void setIcon(const QImage& icon);

    void setIconFile(const QString& path);

    QString iconFile() const;

    /**
      * Returns the #iconFile as an absolute filename. Relative files are
      * resolved relative to the directory of the GeoDataDocument this overlay
      * is part of (@see fileName of GeoDataDocument)
      */
    QString absoluteIconFile() const;

protected:

    explicit GeoDataOverlay(GeoDataOverlayPrivate* priv);
    GeoDataOverlay(const GeoDataOverlay& other, GeoDataOverlayPrivate* priv);

    bool equals(const GeoDataOverlay& other) const;
    using GeoDataFeature::equals;

private:

    Q_DECLARE_PRIVATE(GeoDataOverlay)
};

} // namespace Marble
