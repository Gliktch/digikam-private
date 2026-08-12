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
#include <QDataStream>

// Local includes

#include "GeoDocument.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataSimpleDataPrivate;

class DIGIKAM_EXPORT GeoDataSimpleData : public GeoNode
{
public:

    GeoDataSimpleData();
    GeoDataSimpleData(const GeoDataSimpleData& other);
    ~GeoDataSimpleData() override;

    /*
     * @brief Returns the value of name attribute of SimpleData tag
     */
    QString name() const;

    /*
     * @brief Set the name of SimpleData to @p name
     */
    void setName(const QString& name);

    /*
     * @brief Returns the data defined by SimpleData
     */
    QString data() const;

    /*
     * @brief Set the value of data defined by SimpleData to @p data
     */
    void setData(const QString& data);

    /*
     * Assignment operator
     */
    GeoDataSimpleData& operator=(const GeoDataSimpleData& rhs);

    /*
     * @brief Equality operator
     */
    bool operator==(const GeoDataSimpleData& other) const;
    bool operator!=(const GeoDataSimpleData& other) const;

    /*
     * Provides information for downcasting a GeoNode
     */
    const char* nodeType() const override;

    /*
     * Seriliaze SimpleData to stream @p stream
     */
    virtual void pack(QDataStream& stream) const;

    /*
     * Unseriliaze SimpleData from stream @p stream
     */
    virtual void unpack(QDataStream& stream);

private:

    GeoDataSimpleDataPrivate* const d = nullptr;
};

} // namespace Marble
