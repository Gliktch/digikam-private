/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-01-16
 * Description : Qt item view for images
 *
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "dcategorizedview.h"
#include "dragdropimplementations.h"

class QSortFilterProxyModel;

namespace Digikam
{

class DItemDelegate;
class ItemViewToolTip;

class DIGIKAM_EXPORT ItemViewCategorized : public DCategorizedView,
                                           public DragDropViewImplementation
{
    Q_OBJECT

public:

    explicit ItemViewCategorized(QWidget* const parent = nullptr);
    ~ItemViewCategorized() override;

    DItemDelegate* delegate()                                                       const;
    int            numberOfSelectedIndexes()                                        const;

    /**
     * Selects the index as current and scrolls to it
     */
    void toFirstIndex();
    void toLastIndex();
    void toNextIndex();
    void toPreviousIndex();
    void toIndex(const QModelIndex& index);
    void awayFromSelection();

    /**
     * Ensure a initial selected item.
     */
    void setInitialSelectedItem(bool enabled);

    /**
     * Scroll automatically the current index to center of the view.
     */
    void setScrollCurrentToCenter(bool enabled);

    /**
     * Like scrollTo, but only scrolls if the index is not visible, regardless of hint.
     */
    void scrollToRelaxed(const QModelIndex& index, ScrollHint hint = EnsureVisible);

    void invertSelection();
    void setSelectedIndexes(const QList<QModelIndex>& indexes);

    void setToolTipEnabled(bool enabled);
    bool isToolTipEnabled()                                                         const;

    /**
     * Sets the spacing. Does not use setSpacing()/spacing() from QListView
     */
    void setSpacing(int spacing);

    /**
     * Set if the PointingHand Cursor should be shown over the activation area
     */
    void setUsePointingHandCursor(bool useCursor);

    /**
     * Determine a step size for scrolling: The larger this number,
     * the smaller and more precise is the scrolling. Default is 10.
     */
    void setScrollStepGranularity(int factor);

    virtual QSortFilterProxyModel* filterModel()                                    const = 0;
    void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible)              override;

public Q_SLOTS:

    void showIndexNotification(const QModelIndex& index, const QString& message);
    void hideIndexNotification();

    void cut()   override { DragDropViewImplementation::cut();   }
    void copy()  override { DragDropViewImplementation::copy();  }
    void paste() override { DragDropViewImplementation::paste(); }

Q_SIGNALS:

    /**
     * Emitted when any item selection change occurs. Any of the signals below will be emitted before.
     */
    void itemSelectionChanged();

    /**
     * Emitted when the selection is completely cleared.
     */
    void selectionCleared();

    void zoomOutStep();
    void zoomInStep();

    /**
     * For overlays: Like the respective parent class signals, but with additional info.
     * Do not change the mouse events.
     */
    void signalClicked(const QMouseEvent* e, const QModelIndex& index);
    void signalEntered(const QMouseEvent* e, const QModelIndex& index);

    /**
     * While clicked() is emitted with a valid index, this corresponds to clicking on empty space
     */
    void viewportClicked(const QMouseEvent* e);

    /**
     * Remember you may want to check if the event is accepted or ignored.
     * This signal is emitted after being handled by this widget.
     * You can accept it if ignored.
     */
    void keyPressed(QKeyEvent* e);


protected Q_SLOTS:

    void slotActivated(const QModelIndex& index);
    void slotClicked(const QModelIndex& index);
    void slotEntered(const QModelIndex& index);
    void layoutAboutToBeChanged();
    void layoutWasChanged();

    virtual void slotThemeChanged();
    virtual void slotSetupChanged();

protected:

    void setToolTip(ItemViewToolTip* tip);
    void setItemDelegate(DItemDelegate* delegate);
    void updateDelegateSizes();
    void userInteraction();

    /**
     * Returns an index that is representative for the category at position pos
     */
    QModelIndex indexForCategoryAt(const QPoint& pos) const;

    /// reimplemented from parent class
    void contextMenuEvent(QContextMenuEvent* event)                                       override;
    void keyPressEvent(QKeyEvent* event)                                                  override;
    void leaveEvent(QEvent* event)                                                        override;
    void mouseMoveEvent(QMouseEvent* event)                                               override;
    void mousePressEvent(QMouseEvent* event)                                              override;
    void mouseReleaseEvent(QMouseEvent* event)                                            override;
    void resizeEvent(QResizeEvent* e)                                                     override;
    void reset()                                                                          override;
    void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)              override;
    void rowsInserted(const QModelIndex& parent, int start, int end)                      override;
    void rowsRemoved(const QModelIndex& parent, int start, int end)                       override;
    void selectionChanged(const QItemSelection&, const QItemSelection&)                   override;
    bool viewportEvent(QEvent* event)                                                     override;
    void wheelEvent(QWheelEvent* event)                                                   override;
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)    override;

    /// Reimplement these in a subclass
    virtual void showContextMenuOnIndex(QContextMenuEvent* event, const QModelIndex& index);
    virtual void showContextMenu(QContextMenuEvent* event);
    virtual void indexActivated(const QModelIndex& index, Qt::KeyboardModifiers modifiers);

    /**
     * Provides default behavior, can reimplement in a subclass.
     * Returns true if a tooltip was shown.
     * The help event is optional.
     */
    virtual bool showToolTip(const QModelIndex& index,
                             QStyleOptionViewItem& option,
                             QHelpEvent* e = nullptr);

    DECLARE_VIEW_DRAG_DROP_METHODS(DCategorizedView)

    /**
     * Note: pure virtual dragDropHandler() still open from DragDropViewImplementation
     */
    QModelIndex mapIndexForDragDrop(const QModelIndex& index)                       const override;
    QPixmap     pixmapForDrag(const QList<QModelIndex>& indexes)                    const override;

    /**
     * Assuming the given indexes would be removed (hypothetically!),
     * return the index to be selected instead, starting from anchor.
     * The default implementation returns the next remaining sibling.
     */
    virtual QModelIndex nextIndexHint(const QModelIndex& indexToAnchor,
                                      const QItemSelectionRange& removed)           const;

private Q_SLOTS:

    void slotGridSizeChanged(const QSize&);

private:

    /// @note disabled
    ItemViewCategorized(const ItemViewCategorized&)            = delete;
    ItemViewCategorized& operator=(const ItemViewCategorized&) = delete;

private:

    void ensureSelectionAfterChanges();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
