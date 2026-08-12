/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-08-22
 * Description : Reimplemented QListView for Tags Manager, with support for
 *               drag-n-drop
 *
 * SPDX-FileCopyrightText: 2013      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeView>

class QPoint;

namespace Digikam
{

class TagMngrListView : public QTreeView
{
    Q_OBJECT

public:

    explicit TagMngrListView(QWidget* const parent = nullptr);

    QModelIndexList mySelectedIndexes();

protected:

    /**
     * @brief Reimplemented methods to enable custom drag-n-drop in QListView.
     */
    void startDrag(Qt::DropActions supportedActions)    override;
    void dropEvent(QDropEvent *e)                       override;

    QModelIndex indexVisuallyAt(const QPoint& p);

    /**
     * @brief reimplemented method from QListView to handle custom context menu.
     */
    void contextMenuEvent(QContextMenuEvent* event)     override;


public Q_SLOTS:

    /**
     * @brief delete selected item from Quick Access List.
     */
    void slotDeleteSelected();
};

} // namespace Digikam
