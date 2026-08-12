/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-06
 * Description : Thumbnail bar for items
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "itemcategorizedview.h"
#include "groupingviewimplementation.h"

namespace Digikam
{

class ItemThumbnailBar : public ItemCategorizedView,
                         public GroupingViewImplementation
{
    Q_OBJECT

public:

    explicit ItemThumbnailBar(QWidget* const parent = nullptr);
    ~ItemThumbnailBar() override;

    /**
     * Sets the policy always for the one scroll bar which is relevant, depending on orientation.
     */
    void setScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setFlow(QListView::Flow newFlow);

    void installOverlays();

    /**
     * This installs a duplicate filter model, if the ItemModel may contain duplicates.
     * Otherwise, just use setModels().
     */
    void setModelsFiltered(ItemModel* model, ImageSortFilterModel* filterModel);

    QModelIndex nextIndex(const QModelIndex& index)     const;
    QModelIndex previousIndex(const QModelIndex& index) const;
    QModelIndex firstIndex()                            const;
    QModelIndex lastIndex()                             const;

public Q_SLOTS:

    void assignRating(const QList<QModelIndex>& index, int rating);
    void slotDockLocationChanged(Qt::DockWidgetArea area);

Q_SIGNALS:

    void signalOpenGeolocationMap();

protected Q_SLOTS:

    void slotOpenGeolocationMap(const QModelIndex& index);
    void slotGroupIndicatorClicked(const QModelIndex& index);

protected:

    bool event(QEvent*)                                       override;
    void slotSetupChanged()                                   override;
    bool hasHiddenGroupedImages(const ItemInfo& info)   const override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
