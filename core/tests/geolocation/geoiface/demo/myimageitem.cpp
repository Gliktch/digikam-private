/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-20
 * Description : sub class of QStandardItem to represent the images
 *
 * SPDX-FileCopyrightText: 2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "myimageitem.h"

MyImageItem::MyImageItem(const QUrl& url, const GeoCoordinates& itemCoordinates)
    : QTreeWidgetItem(),
      coordinates    (itemCoordinates),
      imageUrl       (url)
{
}

MyImageItem::~MyImageItem()
{
}

QVariant MyImageItem::data(int column, int role) const
{
    if (role == RoleCoordinates)
    {
        return QVariant::fromValue(coordinates);
    }
    else if (role == Qt::DisplayRole)
    {
        switch (column)
        {
            case 0:
            {
                return imageUrl.fileName();
            }

            case 1:
            {
                return coordinates.geoUrl();
            }

            default:
            {
                return QVariant();
            }
        }
    }

    return QTreeWidgetItem::data(column, role);
}

void MyImageItem::setData(int column, int role, const QVariant& value)
{
    if (role == RoleCoordinates)
    {
        if (value.canConvert<GeoCoordinates>())
        {
            coordinates = value.value<GeoCoordinates>();
            emitDataChanged();
        }

        return;
    }

    QTreeWidgetItem::setData(column, role, value);
}
