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
#include <QList>

// Local includes

#include "GeoDataObject.h"
#include "digikam_export.h"

class QDataStream;

namespace Marble
{

class GeoDataSchemaPrivate;
class GeoDataSimpleField;

/**
 */
class DIGIKAM_EXPORT GeoDataSchema : public GeoDataObject
{
public:

    GeoDataSchema();
    explicit GeoDataSchema(const QHash<QString, GeoDataSimpleField>& simpleFields);
    GeoDataSchema(const GeoDataSchema& other);
    GeoDataSchema& operator=(const GeoDataSchema& other);
    bool operator==(const GeoDataSchema& other) const;
    bool operator!=(const GeoDataSchema& other) const;
    ~GeoDataSchema() override;

    /*
     * @brief Returns the name attribute of schema
     */
    QString schemaName() const;

    /*
     * @brief Sets the name attribute of the schema
     * @param name  The name to be set
     */
    void setSchemaName(const QString& name);

    /*
     * @brief Returns the SimpleField child of schema
     * @param name  The value of name attribute of SimpleField which is to be returned
     */
    GeoDataSimpleField& simpleField(const QString& name) const;

    /*
     * @brief Adds a SimpleField to schema
     * @param value  The SimpleField to be added
     */
    void addSimpleField(const GeoDataSimpleField& value);

    /*
     * @brief dump a vector containing all simple fields of schema
     */
    QList<GeoDataSimpleField> simpleFields() const;

    const char* nodeType() const override;

    void pack(QDataStream& stream) const override;

    void unpack(QDataStream& stream) override;

private:

    GeoDataSchemaPrivate* const d = nullptr;
};

} // namespace Marble
