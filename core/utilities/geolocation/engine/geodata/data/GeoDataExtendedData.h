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

// Local includes

#include "GeoDocument.h"
#include "Serializable.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataSimpleArrayData;
class GeoDataSchemaData;
class GeoDataData;

class GeoDataExtendedDataPrivate;

/**
 * @short a class which allows to add custom data to KML Feature.
 *
 * @see GeoDataData
 */
class DIGIKAM_EXPORT GeoDataExtendedData : public GeoNode, public Serializable
{
public:
    GeoDataExtendedData();
    GeoDataExtendedData(const GeoDataExtendedData& other);
    ~GeoDataExtendedData() override;

    /// Provides type information for downcasting a GeoNode
    const char* nodeType() const override;

    /**
     * @brief assignment operator
     */
    GeoDataExtendedData& operator=(const GeoDataExtendedData& other);

    bool operator==(const GeoDataExtendedData& other) const;
    bool operator!=(const GeoDataExtendedData& other) const;

    /**
     * @brief return the value of GeoDataExtendedData associated with the given @p key
     */
    GeoDataData value(const QString& key) const;

    /**
     * @brief add a data object to the GeoDataExtendedData with the @p key
     */
    void addValue(const GeoDataData& data);

    /**
     * @since 0.26.0
     */
    void removeKey(const QString& key);

    /**
      * @brief return const Begin iterator for QHash
      */
    QHash< QString, GeoDataData >::const_iterator constBegin() const;

    /**
     * @brief return const End iterator for QHash
     */

    QHash< QString, GeoDataData >::const_iterator constEnd() const;

    /**
     * @brief return size of QHash
     */

    int size() const;

    /**
      * @brief return whether QHash is empty or not
      */
    bool isEmpty() const;

    /**
      * @brief Returns true if there exists a value for the given key
      */
    bool contains(const QString& key) const;

    /**
     * @brief return value of GeoDataExtendedData object associated with the given @p key as a modifiable reference
     */
    GeoDataData& valueRef(const QString& key) const;

    /**
     * @brief set SimpleArrayData for given @p key
     *
     * ExtendedData takes ownership of SimpleArrayData
     */
    void setSimpleArrayData(const QString& key, GeoDataSimpleArrayData* values);

    /**
     * @brief return SimpleArrayData for given @p key, 0 pointer if none is set
     */
    GeoDataSimpleArrayData* simpleArrayData(const QString& key) const;

    /**
     * @brief Adds a SchemaData @p schemaData element to schemaDataHash
     */
    GeoDataSchemaData& schemaData(const QString& schemaUrl) const;

    /**
     * @brief Adds a SchemaData @p schemaData element to schemaDataHash
     */
    void addSchemaData(const GeoDataSchemaData& schemaData);

    /**
     * @brief Removes a SchemaData element with schema url @p schemaUrl from schemaDataHash
     */
    void removeSchemaData(const QString& schemaUrl);

    /**
     * @brief Dump a vector containing all SchemaData element
     */
    QList< GeoDataSchemaData > schemaDataList() const;

    /**
     * @brief Serialize the ExtendedData to a stream
     * @param  stream  the stream
     */
    void pack(QDataStream& stream) const override;

    /**
     * @brief  Unserialize the ExtendedData from a stream
     * @param  stream  the stream
     */
    void unpack(QDataStream& stream) override;

private:

    GeoDataExtendedDataPrivate* const d = nullptr;
};

} // namespace Marble
