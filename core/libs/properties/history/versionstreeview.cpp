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

#include "versionstreeview.h"

// Qt includes

#include <QPaintEvent>

// Local includes

#include "digikam_debug.h"
#include "dimagehistory.h"
#include "itemhistorygraphmodel.h"
#include "versionsdelegate.h"
#include "iteminfo.h"
#include "iteminfolist.h"
#include "itemlistmodel.h"
#include "ditemdelegate.h"
#include "itemviewtooltip.h"
#include "tooltipfiller.h"

namespace Digikam
{

class Q_DECL_HIDDEN VersionsTreeView::ToolTip : public ItemViewToolTip
{
    Q_OBJECT

public:

    enum Mode
    {
        InvalidMode,
        ImageMode,
        FilterActionMode
    };

public:

    explicit ToolTip(QAbstractItemView* const view)
        : ItemViewToolTip(view)
    {
    }

    void show(const QStyleOptionViewItem& option, const QModelIndex& index, Mode mode)
    {
        m_mode = mode;
        ItemViewToolTip::show(option, index);
        m_mode = InvalidMode;
    }

protected:

    QString tipContents() override
    {
        switch (m_mode)
        {
            default:
            case InvalidMode:
            {
                return QString();
            }

            case ImageMode:
            {
                ItemInfo info = ItemModel::retrieveItemInfo(currentIndex());

                return ToolTipFiller::imageInfoTipContents(info);
            }

            case FilterActionMode:
            {
                FilterAction action = currentIndex().data(ItemHistoryGraphModel::FilterActionRole).value<FilterAction>();

                return ToolTipFiller::filterActionTipContents(action);
            }
        }
    }

protected:

    Mode m_mode = InvalidMode;
};

// --------------------------------------------------------------------------------------------------------------------------

VersionsTreeView::VersionsTreeView(QWidget* const parent)
    : QTreeView(parent)
{
    m_delegate = new VersionsDelegate(this);
    setItemDelegate(m_delegate);
    m_delegate->setViewOnAllOverlays(this);
    setMouseTracking(true);
}

/// @note All overlay management code in a sophisticated form can be studied in ItemCategorizedView
VersionsTreeView::~VersionsTreeView()
{
    m_delegate->removeAllOverlays();
}

VersionsDelegate* VersionsTreeView::delegate() const
{
    return m_delegate;
}

void VersionsTreeView::addOverlay(ItemDelegateOverlay* overlay)
{
    m_delegate->installOverlay(overlay);
    overlay->setView(this);
    overlay->setActive(true);
}

void VersionsTreeView::removeOverlay(ItemDelegateOverlay* overlay)
{
    m_delegate->removeOverlay(overlay);
    overlay->setView(nullptr);
}

void VersionsTreeView::setToolTipEnabled(bool on)
{
    if (on == m_showToolTip)
    {
        return;
    }

    m_showToolTip = on;

    if (m_showToolTip && !m_toolTip)
    {
        m_toolTip = new ToolTip(this);
    }
}

void VersionsTreeView::paintEvent(QPaintEvent* e)
{
    static_cast<VersionsDelegate*>(itemDelegate())->beginPainting();
    QTreeView::paintEvent(e);
    static_cast<VersionsDelegate*>(itemDelegate())->finishPainting();
}

QModelIndex VersionsTreeView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    // TODO: Need to find a solution to skip non-vertex items in CombinedTreeMode. Not easy.

    return QTreeView::moveCursor(cursorAction, modifiers);
}

void VersionsTreeView::mouseMoveEvent(QMouseEvent* event)
{
    QTreeView::mouseMoveEvent(event);

    QModelIndex index = indexAt(event->pos());
    QRect indexVisualRect;

    if (index.isValid())
    {
        indexVisualRect = visualRect(index);
    }

    m_delegate->mouseMoved(event, indexVisualRect, index);
}

void VersionsTreeView::setDragDropHandler(AbstractItemDragDropHandler* handler)
{
    m_dragDropHandler = handler;
}

AbstractItemDragDropHandler* VersionsTreeView::dragDropHandler() const
{
    return m_dragDropHandler;
}

QModelIndex VersionsTreeView::mapIndexForDragDrop(const QModelIndex& index) const
{
    return index; // no filter model involved
}

QPixmap VersionsTreeView::pixmapForDrag(const QList<QModelIndex>& indexes) const
{
    QStyleOptionViewItem option;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    initViewItemOption(&option);

#else

    option = viewOptions();

#endif

    option.rect                 = viewport()->rect();
    QPixmap pix;

    if (indexes.count() == 1)
    {
        pix = indexes.first().data(Qt::DecorationRole).value<QPixmap>();
    }

    return DItemDelegate::makeDragPixmap(option, indexes, devicePixelRatio(), pix);
}

bool VersionsTreeView::viewportEvent(QEvent* event)
{
    switch (event->type())
    {
        case QEvent::ToolTip:
        {
            if (!m_showToolTip)
            {
                break;
            }

            QHelpEvent* he          = static_cast<QHelpEvent*>(event);
            const QModelIndex index = indexAt(he->pos());

            if (!index.isValid())
            {
                break;
            }

            ToolTip::Mode mode;

            if (index.data(ItemHistoryGraphModel::IsImageItemRole).toBool())
            {
                mode = ToolTip::ImageMode;
            }
            else if (index.data(ItemHistoryGraphModel::IsFilterActionItemRole).toBool())
            {
                mode = ToolTip::FilterActionMode;
            }
            else
            {
                break;
            }

            QStyleOptionViewItem option;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            initViewItemOption(&option);

#else

            option = viewOptions();

#endif

            option.rect                 = visualRect(index);
            option.state               |= (index == currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);

            m_toolTip->show(option, index, mode);

            return true;
        }

        default:
        {
            break;
        }
    }

    return QTreeView::viewportEvent(event);
}

} // namespace Digikam

#include "versionstreeview.moc"

#include "moc_versionstreeview.cpp"
