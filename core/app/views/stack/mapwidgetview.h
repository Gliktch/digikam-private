/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-15
 * Description : central Map view
 *
 * SPDX-FileCopyrightText: 2010      by Gabriel Voicu <ping dot gabi at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "geomodelhelper.h"
#include "digikam_export.h"
#include "dcategorizedsortfilterproxymodel.h"
#include "statesavingobject.h"
#include "itemalbummodel.h"
#include "thumbnailloadthread.h"
#include "itemfiltermodel.h"
#include "camiteminfo.h"

namespace Digikam
{

class AlbumWidgetStack;
class ImageChangeset;

class DIGIKAM_GUI_EXPORT MapWidgetView : public QWidget,
                                         public StateSavingObject
{
    Q_OBJECT

public:

    enum Application
    {
        ApplicationDigikam  = 1,
        ApplicationImportUI = 2
    };

public:

    explicit MapWidgetView(QItemSelectionModel* const selectionModel,
                           DCategorizedSortFilterProxyModel* const imageFilterModel,
                           QWidget* const parent,
                           const Application application);
    ~MapWidgetView()                        override;

    void setActive(const bool state);
    bool getActiveState()            const;

    ItemInfo   currentItemInfo()     const;
    CamItemInfo currentCamItemInfo() const;

private Q_SLOTS:

    void slotModelChanged();
    void slotAllTrackFilesReady();
    void slotLoadTracksFromAlbums();

protected:

    void doLoadState()                      override;
    void doSaveState()                      override;

private:

    class Private;
    const QScopedPointer<Private> d;
};

// ----------------------------------------------------------------------------------------------

class MapViewModelHelper : public GeoModelHelper
{
    Q_OBJECT

public:

    explicit MapViewModelHelper(QItemSelectionModel* const selection,
                                DCategorizedSortFilterProxyModel* const filterModel,
                                QObject* const parent,
                                const MapWidgetView::Application application);
    ~MapViewModelHelper()                                                     override;

    QAbstractItemModel* model()                                         const override;
    QItemSelectionModel* selectionModel()                               const override;
    bool itemCoordinates(const QModelIndex& index,
                         GeoCoordinates* const coordinates)             const override;

    QPixmap pixmapFromRepresentativeIndex(const QPersistentModelIndex& index,
                                          const QSize& size)                  override;
    QPersistentModelIndex bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list,
                                                          const int sortKey)  override;

    void onIndicesClicked(const QList<QPersistentModelIndex>& clickedIndices) override;

Q_SIGNALS:

    void signalFilteredImages(const QList<qlonglong>& idList);

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);
    void slotThumbnailLoaded(const CamItemInfo& info);
    void slotImageChange(const ImageChangeset& changeset);

private:

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam
