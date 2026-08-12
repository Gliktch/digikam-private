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

#include "GeoDataObject.h"

class QVariant;

namespace Marble
{

class GeoDataSimpleArrayDataPrivate;

class DIGIKAM_EXPORT GeoDataSimpleArrayData : public GeoDataObject
{

public:

    GeoDataSimpleArrayData();
    GeoDataSimpleArrayData(const GeoDataSimpleArrayData& other);
    bool operator==(const GeoDataSimpleArrayData& other) const;
    bool operator!=(const GeoDataSimpleArrayData& other) const;
    ~GeoDataSimpleArrayData() override;

    /**
     * Returns the number of value in the array
     */
    int size() const;

    /**
     * Returns the value at index @p index
     */
    QVariant valueAt(int index) const;

    /**
     * Returns all values in the array
     */
    QList< QVariant > valuesList() const;

    /**
     * Append a value to the array
     */
    void append(const QVariant& value);

    const char* nodeType() const override;
    void pack(QDataStream& stream) const override;      // cppcheck-suppress uselessOverride
    void unpack(QDataStream& stream) override;          // cppcheck-suppress uselessOverride

private:

    GeoDataSimpleArrayData& operator=(const GeoDataSimpleArrayData&) = delete;

private:

    GeoDataSimpleArrayDataPrivate* d = nullptr;
};

} // namespace Marble
