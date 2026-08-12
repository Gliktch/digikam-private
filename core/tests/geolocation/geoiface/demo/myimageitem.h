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

#pragma once

// Qt includes

#include <QTreeWidgetItem>
#include <QPersistentModelIndex>
#include <QUrl>

// Local includes

#include "geoifacetypes.h"
#include "geocoordinates.h"

using namespace Digikam;

const int RoleMyData      = Qt::UserRole+0;
const int RoleCoordinates = Qt::UserRole+1;

class MyImageItem : public QTreeWidgetItem
{
public:

    MyImageItem(const QUrl& url, const GeoCoordinates& itemCoordinates);
    ~MyImageItem() override;

    QVariant data(int column, int role) const override;
    void setData(int column, int role, const QVariant& value) override;

private:

    GeoCoordinates coordinates;
    QUrl           imageUrl;

private:

    Q_DISABLE_COPY(MyImageItem)
};
