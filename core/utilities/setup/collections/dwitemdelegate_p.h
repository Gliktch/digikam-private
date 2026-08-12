/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-15
 * Description : widget item delegate for setup collection view
 *
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2008 by Rafael Fernández López <ereslibre at kde dot org>
 * SPDX-FileCopyrightText: 2008      by Kevin Ottens <ervin at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "dwitemdelegate.h"

// Qt includes

#include <QObject>
#include <QItemSelectionModel>

namespace Digikam
{

class DWItemDelegate;
class DWItemDelegatePool;

class Q_DECL_HIDDEN DWItemDelegatePrivate : public QObject
{
    Q_OBJECT

public:

    explicit DWItemDelegatePrivate(DWItemDelegate* const qq, QObject* const parent = nullptr);
    ~DWItemDelegatePrivate()                            override;

    void updateRowRange(const QModelIndex& parent, int start, int end, bool isRemoving);
    QStyleOptionViewItem optionView(const QModelIndex& index);

public Q_SLOTS:

    void initializeModel(const QModelIndex& parent = QModelIndex());

    void slotDWRowsInserted(const QModelIndex& parent, int start, int end);
    void slotDWRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void slotDWRowsRemoved(const QModelIndex& parent, int start, int end);
    void slotDWDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void slotDWLayoutChanged();
    void slotDWModelReset();
    void slotDWSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

protected:

    bool eventFilter(QObject* watched, QEvent* event)   override;

public:

    QAbstractItemView*   itemView       = nullptr;
    DWItemDelegatePool*  widgetPool     = nullptr;
    QAbstractItemModel*  model          = nullptr;
    QItemSelectionModel* selectionModel = nullptr;
    DWItemDelegate*      q              = nullptr;
};

} // namespace Digikam
