/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-22
 * Description : Reimplemented QListView for metadata setup, with support for
 *               drag-n-drop
 *
 * SPDX-FileCopyrightText: 2013-2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QListView>

class QPoint;

namespace Digikam
{

class NamespaceListView : public QListView
{
    Q_OBJECT

public:

    explicit NamespaceListView(QWidget* const parent = nullptr);
    QModelIndexList mySelectedIndexes();

protected:

    /**
     * Reimplemented methods to enable custom drag-n-drop in QListView
     */
    void startDrag(Qt::DropActions supportedActions) override;
    void dropEvent(QDropEvent* e) override;

    QModelIndex indexVisuallyAt(const QPoint& p);

    /**
     * @brief contextMenuEvent - reimplemented method from QListView
     *                           to handle custom context menu
     */
/*
    void contextMenuEvent(QContextMenuEvent* event);
*/

Q_SIGNALS:

    void signalItemsChanged();

public Q_SLOTS:

    /**
     * @brief slotDeleteSelected - delete selected item from Quick Access List
     */
    void slotDeleteSelected();

    void slotMoveItemDown();

    void slotMoveItemUp();
};

} // namespace Digikam
