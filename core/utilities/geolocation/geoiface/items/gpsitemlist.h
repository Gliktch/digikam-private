/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-22
 * Description : A view to display a list of items with GPS info.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeView>

// Local includes

#include "gpsitemmodel.h"
#include "gpsitemsortproxymodel.h"
#include "digikam_export.h"

class QWheelEvent;
class KConfigGroup;

namespace Digikam
{

class ItemListDragDropHandler;

class DIGIKAM_EXPORT GPSItemList : public QTreeView
{
    Q_OBJECT

public:

    explicit GPSItemList(QWidget* const parent = nullptr);
    ~GPSItemList()                                    override;

    void setModelAndSelectionModel(GPSItemModel* const model, QItemSelectionModel* const selectionModel);
    GPSItemModel* getModel()                    const;
    QItemSelectionModel* getSelectionModel()    const;
    void setDragDropHandler(ItemListDragDropHandler* const dragDropHandler);
    void setThumbnailSize(const int size);
    GPSItemSortProxyModel* getSortProxyModel()  const;

    void saveSettingsToGroup(KConfigGroup* const group);
    void readSettingsFromGroup(const KConfigGroup* const group);
    void setEditEnabled(const bool state);
    void setDragEnabled(const bool state);

Q_SIGNALS:

    void signalImageActivated(const QModelIndex& index);

public Q_SLOTS:

    void slotIncreaseThumbnailSize();
    void slotDecreaseThumbnailSize();
    void slotUpdateActionsEnabled();

private Q_SLOTS:

    void slotInternalTreeViewImageActivated(const QModelIndex& current, const QModelIndex& previous);
    void slotThumbnailFromModel(const QPersistentModelIndex& index, const QPixmap& pixmap);
    void slotColumnVisibilityActionTriggered(QAction* action);

protected:

    bool eventFilter(QObject* watched, QEvent* event) override;
    void startDrag(Qt::DropActions supportedActions)  override;
    void wheelEvent(QWheelEvent* we)                  override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
