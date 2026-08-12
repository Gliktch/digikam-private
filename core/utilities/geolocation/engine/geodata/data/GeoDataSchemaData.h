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

#include "GeoDocument.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataExtendedData;
class GeoDataSchemaDataPrivate;
class GeoDataSimpleData;

/**
 */
class DIGIKAM_EXPORT GeoDataSchemaData : public GeoNode
{
public:

    GeoDataSchemaData();
    GeoDataSchemaData(const GeoDataSchemaData& other);
    GeoDataSchemaData& operator=(const GeoDataSchemaData& rhs);
    bool operator==(const GeoDataSchemaData& other) const;
    bool operator!=(const GeoDataSchemaData& other) const;
    ~GeoDataSchemaData() override;

    /*
     * @brief Returns the id for schema which defines custom data
     */
    QString schemaUrl() const;

    /*
     * @brief Set the schemaUrl attribute of SchemaData to @p schemaUrl
     */
    void setSchemaUrl(const QString& schemaUrl);

    /*
     * @brief Returns the SimpleData having name attribute @p name
     */
    GeoDataSimpleData& simpleData(const QString& name) const;

    /*
     * @brief Adds a SimpleData @p simpleData to schemaDataHash
     */
    void addSimpleData(const GeoDataSimpleData& simpleData);

    /*
     * @brief Dump a list containing all SimpleData values stored in schemaDataHash
     */
    QList<GeoDataSimpleData> simpleDataList() const;

    /*
     * @brief Set the parent @parent
     */
    void setParent(GeoDataExtendedData* parent);

    /*
     * @brief Get the parent
     */
    const GeoDataExtendedData* parent() const;
    GeoDataExtendedData* parent();

    /*
     * @brief Provides information for downcasting a GeoNode
     */
    const char* nodeType() const override;

    /*
     * @brief Serialize SchemaData to a stream @p stream
     */
    virtual void pack(QDataStream& stream) const;

    /*
     * @brief Unserialize SchemaData from a stream @p stream
     */
    virtual void unpack(QDataStream& stream);

private:

    GeoDataSchemaDataPrivate* const d = nullptr;
};

}   // namespace Marble
