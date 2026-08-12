/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-04-11
 * Description : A list view to show grouped items
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPixmap>
#include <QTreeWidget>
#include <QTreeWidgetItem>

// Local includes

#include "iteminfo.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class ItemFilterModel;

class ItemsGroupedView : public QTreeWidget
{
    Q_OBJECT

public:

    explicit ItemsGroupedView(QWidget* const parent);
    ~ItemsGroupedView()                                                  override;

    void setGroups(const ItemInfoList& items);
    void setEnableToolTips(bool val);

    void setItemFilterModel(ItemFilterModel* const model);
    ItemFilterModel* itemFilterModel()                              const;

private Q_SLOTS:

    void slotGotThumbnail(const LoadingDescription&, const QPixmap&);
    void slotSettingsChanged();
    void slotToolTip();

private:

    void hideToolTip();
    bool acceptToolTip(const QPoint& pos)                          const;

    void hideToolBar();
    bool acceptToolBar(const QPoint& pos)                          const;

    void mouseMoveEvent(QMouseEvent*)                                    override;
    void wheelEvent(QWheelEvent*)                                        override;
    void keyPressEvent(QKeyEvent*)                                       override;
    void focusOutEvent(QFocusEvent*)                                     override;
    void leaveEvent(QEvent*)                                             override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
