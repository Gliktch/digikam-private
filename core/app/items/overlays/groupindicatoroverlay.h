/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-28
 * Description : overlay for extra functionality of the group indicator
 *
 * SPDX-FileCopyrightText: 2011      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractButton>
#include <QAbstractItemView>

// Local includes

#include "itemdelegateoverlay.h"
#include "itemviewdelegate.h"

namespace Digikam
{

class GroupIndicatorOverlayWidget : public QAbstractButton
{
    Q_OBJECT

public:

    explicit GroupIndicatorOverlayWidget(QWidget* const parent = nullptr);
    void contextMenuEvent(QContextMenuEvent* event) override;

protected:

    void paintEvent(QPaintEvent*)                   override;

Q_SIGNALS:

    void contextMenu(QContextMenuEvent* event);
};

// ----------------------------------------------------------------------------------

class GroupIndicatorOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewDelegate)

public:

    explicit GroupIndicatorOverlay(QObject* const parent);

    GroupIndicatorOverlayWidget* buttonWidget() const;

Q_SIGNALS:

    void toggleGroupOpen(const QModelIndex& index);
    void showButtonContextMenu(const QModelIndex& index, QContextMenuEvent* event);

protected Q_SLOTS:

    void slotButtonClicked();
    void slotButtonContextMenu(QContextMenuEvent* event);

protected:

    void updatePosition();

    QWidget* createWidget()                           override;
    void setActive(bool)                              override;
    void visualChange()                               override;
    void slotEntered(const QModelIndex& index)        override;
    bool checkIndex(const QModelIndex& index)   const override;

protected:

    QPersistentModelIndex m_index;
};

} // namespace Digikam
