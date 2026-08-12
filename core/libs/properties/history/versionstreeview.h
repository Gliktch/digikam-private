/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-26
 * Description : images versions QTreeView
 *
 * SPDX-FileCopyrightText: 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
 * SPDX-FileCopyrightText: 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeView>

// Local includes

#include "digikam_export.h"
#include "dragdropimplementations.h"

namespace Digikam
{

class VersionsDelegate;
class ItemDelegateOverlay;

class VersionsTreeView : public QTreeView,
                         public DragDropViewImplementation
{
    Q_OBJECT

public:

    explicit VersionsTreeView(QWidget* const parent = nullptr);
    ~VersionsTreeView()                                                override;

    void setToolTipEnabled(bool on);

    void addOverlay(ItemDelegateOverlay* overlay);
    void removeOverlay(ItemDelegateOverlay* overlay);

    VersionsDelegate* delegate()                                 const;

protected:

    void paintEvent(QPaintEvent* e)                                    override;
    void mouseMoveEvent(QMouseEvent* event)                            override;
    bool viewportEvent(QEvent* event)                                  override;
    QModelIndex moveCursor(CursorAction cursorAction,
                           Qt::KeyboardModifiers modifiers)            override;

    DECLARE_VIEW_DRAG_DROP_METHODS(QTreeView)
    QModelIndex mapIndexForDragDrop(const QModelIndex& index)    const override;
    QPixmap     pixmapForDrag(const QList<QModelIndex>& indexes) const override;
    AbstractItemDragDropHandler* dragDropHandler()               const override;
    virtual void setDragDropHandler(AbstractItemDragDropHandler* handler);

protected:

    class ToolTip;

    VersionsDelegate*            m_delegate         = nullptr;
    AbstractItemDragDropHandler* m_dragDropHandler  = nullptr;
    bool                         m_showToolTip      = false;
    ToolTip*                     m_toolTip          = nullptr;

private:

    /// @note disabled
    VersionsTreeView(const VersionsTreeView&)            = delete;
    VersionsTreeView& operator=(const VersionsTreeView&) = delete;
};

} // namespace Digikam
