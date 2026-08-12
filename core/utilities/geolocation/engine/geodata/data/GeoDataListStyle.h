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

// qt includes

#include <QColor>
#include <QVector>

// Local includes

#include "MarbleGlobal.h"
#include "GeoDataObject.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataListStylePrivate;
class GeoDataItemIcon;

class DIGIKAM_EXPORT GeoDataListStyle : public GeoDataObject
{
public:

    GeoDataListStyle();

    GeoDataListStyle(const GeoDataListStyle& other);

    GeoDataListStyle& operator=(const GeoDataListStyle& other);

    bool operator==(const GeoDataListStyle& other) const;
    bool operator!=(const GeoDataListStyle& other) const;

    ~GeoDataListStyle() override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    enum ListItemType
    {
        Check,
        RadioFolder,
        CheckOffOnly,
        CheckHideChildren
    };

    ListItemType listItemType() const;
    void setListItemType(ListItemType type);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor& color);

    QVector<GeoDataItemIcon*> itemIconList() const;

    GeoDataItemIcon* child(int);

    const GeoDataItemIcon* child(int) const;

    int childPosition(const GeoDataItemIcon* child) const;

    void append(GeoDataItemIcon* other);

    void remove(int index);

    int size() const;

    GeoDataItemIcon& at(int pos);
    const GeoDataItemIcon& at(int pos) const;

    GeoDataItemIcon& last();
    const GeoDataItemIcon& last() const;

    GeoDataItemIcon& first();
    const GeoDataItemIcon& first() const;

    QVector<GeoDataItemIcon*>::Iterator begin();
    QVector<GeoDataItemIcon*>::Iterator end();
    QVector<GeoDataItemIcon*>::ConstIterator constBegin() const;
    QVector<GeoDataItemIcon*>::ConstIterator constEnd() const;
    void clear();

    void pack(QDataStream& stream) const override;

    void unpack(QDataStream& stream) override;

private:

    friend class GeoDataItemIcon;

    GeoDataListStylePrivate* const d = nullptr;
};

} // namespace Marble
