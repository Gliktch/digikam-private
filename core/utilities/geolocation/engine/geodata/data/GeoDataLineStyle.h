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

#include "GeoDataColorStyle.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataLineStylePrivate;

/**
 * @short specifies the style how lines are drawn
 *
 * A GeoDataLineStyle specifies how the <b>name</b> of a
 * GeoDataFeature is drawn in the viewer. A custom color, color mode
 * (both inherited from GeoDataColorStyle) and width for the width
 * of the line.
 */
class DIGIKAM_EXPORT GeoDataLineStyle : public GeoDataColorStyle
{
public:

    /// Construct a new GeoDataLineStyle
    GeoDataLineStyle();
    GeoDataLineStyle(const GeoDataLineStyle& other);
    /**
     * @brief Construct a new GeoDataLineStyle
     * @param  color  the color to use when showing the name @see GeoDataColorStyle
     */
    explicit GeoDataLineStyle(const QColor& color);

    ~GeoDataLineStyle() override;

    /**
    * @brief assignment operator
    */
    GeoDataLineStyle& operator=(const GeoDataLineStyle& other);

    bool operator==(const GeoDataLineStyle& other) const;
    bool operator!=(const GeoDataLineStyle& other) const;

    /// Provides type information for downcasting a GeoData
    const char* nodeType() const override;

    /**
     * @brief Set the width of the line
     * @param  width  the new width
     */
    void setWidth(float width);
    /**
     * @brief Return the current width of the line
     * @return the current width
     */
    float width() const;

    /**
     * @brief Set the physical width of the line (in meters)
     * @param  realWidth  the new width
     */
    void setPhysicalWidth(float realWidth);
    /**
     * @brief Return the current physical width of the line
     * @return the current width
     */
    float physicalWidth() const;

    /**
     * @brief Set whether the line has a cosmetic 1 pixel outline
     */
    void setCosmeticOutline(bool enabled);
    /**
     * @brief Return whether the line has a cosmetic 1 pixel outline
     */
    bool cosmeticOutline() const;


    /**
     * @brief Set pen cap style
     * @param  style cap style
     */
    void setCapStyle(Qt::PenCapStyle style);

    /**
     * @brief Return the current pen cap style
     * @return the current pen cap style
     */
    Qt::PenCapStyle capStyle() const;

    /**
     * @brief Set pen cap style
     * @param  style cap style
     */
    void setPenStyle(Qt::PenStyle style);

    /**
     * @brief Return the current pen cap style
     * @return the current pen cap style
     */
    Qt::PenStyle penStyle() const;

    /**
    * @brief Set whether to draw the solid background
    * @param background @c true if the background should be solid
    */
    void setBackground(bool background);

    /**
     * @brief Return true if background get drawn
     * @return
     */
    bool background() const;

    /**
     * @brief Sets the dash pattern
     * @param pattern dash pattern
     */
    void setDashPattern(const QVector<qreal>& pattern);

    /**
     * @brief Return the current dash pattern
     * @return the current dash pattern
     */
    QVector<qreal> dashPattern() const;

    /**
     * @brief  Serialize the style to a stream.
     * @param  stream  the stream
     */
    void pack(QDataStream& stream) const override;
    /**
     * @brief  Unserialize the style from a stream
     * @param  stream  the stream
     */
    void unpack(QDataStream& stream) override;

private:

    GeoDataLineStylePrivate* const d = nullptr;
};

} // namespace Marble
