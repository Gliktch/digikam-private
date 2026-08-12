/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-15
 * Description : Model for central Map view
 *
 * SPDX-FileCopyrightText: 2010      by Gabriel Voicu <ping dot gabi at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QStandardItemModel>
#include <QPixmap>

// Local includes

#include "geomodelhelper.h"
#include "thumbnailloadthread.h"
#include "gpsiteminfosorter.h"

namespace Digikam
{

const int RoleGPSItemInfo = Qt::UserRole + 1;

class ItemGPSModelHelper : public GeoModelHelper
{
    Q_OBJECT

public:

    explicit ItemGPSModelHelper(QStandardItemModel* const itemModel,
                                QObject* const parent = nullptr);
    ~ItemGPSModelHelper()                                                                  override;

    QAbstractItemModel* model()                                                      const override;
    QItemSelectionModel* selectionModel()                                            const override;

    bool itemCoordinates(const QModelIndex& index,
                         GeoCoordinates* const coordinates)                          const override;

    QPixmap pixmapFromRepresentativeIndex(const QPersistentModelIndex& index,
                                          const QSize& size)                               override;

    QPersistentModelIndex bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list,
                                                          const int sortKey)               override;

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
