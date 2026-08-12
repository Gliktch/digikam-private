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

#include "GeoDataGeometry.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataMultiGeometryPrivate;

/**
 * @short A class that can contain other GeoDataGeometry objects
 *
 * GeoDataMultiGeometry is a collection of other GeoDataGeometry objects.
 * As one can add GeoDataMultiGeometry to itself, you can make up a collection
 * of different objects to form one Placemark.
 */
class DIGIKAM_EXPORT GeoDataMultiGeometry : public GeoDataGeometry
{
public:

    GeoDataMultiGeometry();
    explicit GeoDataMultiGeometry(const GeoDataGeometry& other);

    ~GeoDataMultiGeometry() override;

    const char* nodeType() const override;

    EnumGeometryId geometryId() const override;

    GeoDataGeometry* copy() const override;

    bool operator==(const GeoDataMultiGeometry& other) const;
    bool operator!=(const GeoDataMultiGeometry& other) const
    {
        return !(*this == other);
    }

    const GeoDataLatLonAltBox& latLonAltBox() const override;

    int size() const;
    GeoDataGeometry& at(int pos);
    const GeoDataGeometry& at(int pos) const;
    GeoDataGeometry& operator[](int pos);
    const GeoDataGeometry& operator[](int pos) const;

    GeoDataGeometry& first();
    const GeoDataGeometry& first() const;
    GeoDataGeometry& last();
    const GeoDataGeometry& last() const;

    /**
     * @brief  returns the requested child item
     */
    GeoDataGeometry* child(int);

    /**
     * @brief  returns the requested child item
     */
    const GeoDataGeometry* child(int) const;

    /**
     * @brief returns the position of an item in the list
     */
    int childPosition(const GeoDataGeometry* child) const;

    /**
    * @brief add an element
    */
    void append(GeoDataGeometry* other);

    GeoDataMultiGeometry& operator << (const GeoDataGeometry& value);

    QVector<GeoDataGeometry*>::Iterator begin();
    QVector<GeoDataGeometry*>::Iterator end();
    QVector<GeoDataGeometry*>::ConstIterator constBegin() const;
    QVector<GeoDataGeometry*>::ConstIterator constEnd() const;
    void clear();
    QVector<GeoDataGeometry*> vector();

    QVector<GeoDataGeometry*>::Iterator erase(QVector<GeoDataGeometry*>::Iterator pos);
    QVector<GeoDataGeometry*>::Iterator erase(QVector<GeoDataGeometry*>::Iterator begin,
                                              QVector<GeoDataGeometry*>::Iterator end);

    // Serialize the Placemark to @p stream
    void pack(QDataStream& stream) const override;
    // Unserialize the Placemark from @p stream
    void unpack(QDataStream& stream) override;

private:

    Q_DECLARE_PRIVATE(GeoDataMultiGeometry)
};

} // namespace Marble
