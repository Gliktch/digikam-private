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

#include <QString>
#include <QMap>

// Local includes

#include "GeoDataStyleSelector.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataStyleMapPrivate;

/**
 * @short a class to map different styles to one style
 *
 * A GeoDataStyleMap connects styles for highlighted and normal
 * context. Besides GeoDataStyleSelector it derives from
 * QMap<QString, QString>.
 *
 * @see GeoDataStyle
 * @see GeoDataStyleSelector
 */
class DIGIKAM_EXPORT GeoDataStyleMap : public GeoDataStyleSelector,
    public QMap<QString, QString>
{
public:

    /// Provides type information for downcasting a GeoNode
    const char* nodeType() const override;

    /**
    * @brief return the last key
    */
    QString lastKey() const;
    /**
    * @brief Set the last key
    * this property is needed to set an entry in the kml parser
    * after the parser has set the last key, it will read the value
    * and add both to this map
    * @param key the last key
    */
    void setLastKey(const QString& key);

    /**
    * @brief assignment operator
    * @param other the styleMap which gets duplicated.
    */
    GeoDataStyleMap& operator=(const GeoDataStyleMap& other);

    bool operator==(const GeoDataStyleMap& other) const;
    bool operator!=(const GeoDataStyleMap& other) const;

    /**
     * @brief Serialize the stylemap to a stream
     * @param  stream  the stream
     */
    void pack(QDataStream& stream) const override;
    /**
     * @brief  Unserialize the stylemap from a stream
     * @param  stream  the stream
     */
    void unpack(QDataStream& stream) override;

    GeoDataStyleMap();
    GeoDataStyleMap(const GeoDataStyleMap& other);
    ~GeoDataStyleMap() override;

private:

    GeoDataStyleMapPrivate* const d = nullptr;
};

} // namespace Marble
