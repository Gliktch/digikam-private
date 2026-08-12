/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * SPDX-FileCopyrightText: 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeView>

// Local includes

#include "albummanager.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "albumpointer.h"
#include "statesavingobject.h"

namespace Digikam
{

class ContextMenuHelper;
class TagModificationHelper;

// NOTE: This structure name can be in conflict with QAbstractItemView::State. FIXME later.
struct State;

/**
 * @brief Base class for all tree views that display Album-based content provided by an
 * AbstractSpecificAlbumModel. This class enables various utility functions like
 * selecting albums on mouse actions or providing an infrastructure for
 * displaying a context menu for albums.
 *
 * Context menu handling is implemented as template methods with hook methods
 * that can be implemented by subclasses to provide a custom behavior. In
 * default mode no context menu is shown at all. It must be enabled via a call
 * to setEnableContextMenu.
 */
class AbstractAlbumTreeView : public QTreeView,
                              public StateSavingObject
{
    Q_OBJECT

public:

    enum Flag
    {
        /**
         * Create a default model. Not supported by abstract classes.
         * Not part of default flags!
         */
        CreateDefaultModel,

        /**
         * Create a default filter model.
         */
        CreateDefaultFilterModel,

        /**
         * Create a delegate which paints according to settings.
         * If not set, the Qt default delegate of the view is used.
         */
        CreateDefaultDelegate,

        /**
         * Show the count according to the settings.
         * If not set, call setShowCount() on the model yourself.
         */
        ShowCountAccordingToSettings,

        /**
         * Always show the inclusive counts.
         * Not part of default flags!
         */
        AlwaysShowInclusiveCounts,

        DefaultFlags = CreateDefaultFilterModel | CreateDefaultDelegate | ShowCountAccordingToSettings
    };
    Q_DECLARE_FLAGS(Flags, Flag)

public:

    /**
     * @brief Constructs an album tree view.
     * If you give 0 for model, call setAlbumModel afterwards.
     * If you supply 0 for filterModel, call setAlbumFilterModel afterwards.
     */
    AbstractAlbumTreeView(QWidget* const parent, Flags flags);
    ~AbstractAlbumTreeView()                                                    override;

    AbstractSpecificAlbumModel* albumModel() const;
    AlbumFilterModel* albumFilterModel()     const;

    /**
     * @brief Enable expanding of tree items on single click on the item (default: off)
     */
    void setExpandOnSingleClick(const bool doThat);

    /**
     * @brief Expand an item when making it the new current item
     */
    void setExpandNewCurrentItem(const bool doThat);

    /**
     * @brief Sets whether to select an album on click via the album manager or not.
     *
     * @param selectOnClick if true, a click on an item automatically sets this
     *                      item as the current album in the album manager
     */
    void setSelectAlbumOnClick(const bool selectOnClick);

    /**
     * @brief Determines the global decision to show a popup menu or not. More detailed
     * decision at which position a menu can be shown and where not can be made
     * by implementing showContextMenuAt.
     *
     * @param enable if true, a context menu can be shown
     */
    void setEnableContextMenu(const bool enable);

    /**
     * @brief Sets whether to select the album under the mouse cursor on a context menu
     * request (so that the album is shown using the album manager) or not
     *
     * Defaults to true.
     *
     * @param select true if a context menu request shall select the album
     */
    void setSelectOnContextMenu(const bool select);

    /**
     * @brief Set the context menu title and icon.
     * This is used by the default implementation of contextMenuIcon()
     * and contextMenuTitle(). You can alternatively reimplement these methods.
     */
    void setContextMenuIcon(const QPixmap& pixmap);
    void setContextMenuTitle(const QString& title);

    /**
     * @brief This is a combination of indexAt() checked with visualRect().
     * @param p must be in the viewport currently. Decoration will not be included.
     * Suitable for mouse click positions.
     */
    QModelIndex indexVisuallyAt(const QPoint& p);

    /**
     * @brief Ensures that every current match is visible by expanding all parent
     * entries.
     *
     * @param index the index to start ensuring expansion state
     * @return <code>true</code> if there was a match under <code>index</code>.
     *         This return value can normally be ignored by the caller because
     *         it is only used for an internal recursion.
     */
    bool expandMatches(const QModelIndex& index);

    //@{

    /**
     * @brief Implements state loading for the album tree view in a somewhat clumsy
     * procedure because the model may not be fully loaded when this method is
     * called. Therefore the config is first parsed into d->statesByAlbumId
     * which holds the state of all tree view entries indexed by album id.
     * Afterwards an initial sync run is done restoring the state of all model
     * entries that are already present at this time. Every processed entry
     * is removed from d->statesByAlbumId. If there are still entries left in
     * this map we assume that the model is not fully loaded at the moment.
     * Therefore the rowsInserted signal is connected to a slot that restores
     * the state of new rows based on the remaining entries in
     * d->statesByAlbumId.
     */
    void doLoadState()                                                          override;
    void doSaveState()                                                          override;

    //@}

    /**
     * @brief Some treeviews shall control the global current album kept by AlbumManager.
     * Other treeview are self-contained and shall not change the current album.
     * Default: false
     */
    void setAlbumManagerCurrentAlbum(const bool setCurrentAlbum);

    /**
     * @brief Add a context menu element which can add actions to the context
     * menu when the menu is generated.
     * First, addCustomContextMenuActions is called, then
     * all elements' addActions method is called in order of addition.
     */
    class ContextMenuElement
    {
    public:

        ContextMenuElement()                    = default;
        virtual ~ContextMenuElement()           = default;

        /**
         * @brief Add actions to the context menu being generated
         *
         * @param view The AbstractAlbumTreeView which generates the menu
         * @param cmh helper object to create the context menu
         * @param album the album on which the context menu will be created. May be null if
         *              it is requested on no tag entry
         */
        virtual void addActions(AbstractAlbumTreeView* view,
                                ContextMenuHelper& cmh,
                                Album* album)   = 0;

    private:

        Q_DISABLE_COPY(ContextMenuElement)
    };

    void addContextMenuElement(ContextMenuElement* const element);
    void removeContextMenuElement(ContextMenuElement* const element);
    QList<ContextMenuElement*> contextMenuElements() const;

    template<class A>
    QList<A*> currentAlbums();

    /**
     * @brief For internal use only.
     */
    bool viewportEvent(QEvent* event)                                           override;

    /**
     * @return selected Items
     */
    QList<Album*> selectedItems();

public Q_SLOTS:

    void setSearchTextSettings(const SearchTextSettings& settings);

    /**
     * @brief Selects the given album.
     *
     * @param albums the albums to select
     * @param selectInAlbumManager the album will be set as current album, if both
     * this parameter is true and setAlbumManagerCurrentAlbum() was set to true.
     */
    void setCurrentAlbums(const QList<Album*>& albums, bool selectInAlbumManager = true);

    /**
     * @brief Adapt the column sizes to the contents of the tree view.
     */
    void adaptColumnsToContent();

    /**
     * @brief Scrolls to the first selected album if there is one.
     */
    void scrollToSelectedAlbum();

    /**
     * @brief Expands the complete tree under the given index.
     *
     * @param index the index to start expanding everything
     */
    void expandEverything(const QModelIndex& index);

    /**
     * @brief slotExpandNode - expands recursively selected nodes
     */
    void slotExpandNode();

    /**
     * @brief slotCollapseNode - collapse recursively selected nodes
     */
    void slotCollapseNode();

    /**
     * @brief slotCollapseAllNodes - collapse all nodes without root node
     */
    void slotCollapseAllNodes();

Q_SIGNALS:

    /**
     * @brief Emitted when the currently selected album changes
     */
    void currentAlbumChanged(Album* currentAlbum);

    /**
     * @brief Emitted when the current selection changes. Use currentChanged unless in multi-selection mode.
     */
    void selectedAlbumsChanged(const QList<Album*>& selectedAlbums);

protected Q_SLOTS:

    /**
     * @note override if implemented behavior is not as intended
     */
    virtual void slotRootAlbumAvailable();

    void slotSearchTextSettingsChanged(bool wasSearching, bool searching);
    void slotSearchTextSettingsAboutToChange(bool searched, bool willSearch);
    void slotCurrentChanged();
    void slotSelectionChanged();

    void albumSettingsChanged();

protected:

    //@{

    // Context menu handling

    /**
     * @brief Hook method to implement that determines if a context menu shall be
     * displayed for the given event at the position coded in the event.
     *
     * @param event context menu event to react on
     * @param albumForEvent the album at the mouse position or null if there is
     *                      no album at that position
     * @return true if a context menu shall be displayed at the event
     *         coordinates, else false
     */
    virtual bool showContextMenuAt(QContextMenuEvent* event, Album* albumForEvent);

    /**
     * @brief Hook method that can be implemented to return a special icon used for the
     * context menu.
     *
     * @return the icon for the context menu
     */
    virtual QPixmap contextMenuIcon()  const;

    /**
     * @brief Hook method to implement that returns the title for the context menu.
     *
     * @return title for the context menu
     */
    virtual QString contextMenuTitle() const;

    /**
     * @brief Hook method to add custom actions to the generated context menu.
     *
     * @param cmh helper object to create the context menu
     * @param album tag on which the context menu will be created. May be null if
     *              it is requested on no tag entry
     */
    virtual void addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album);

    /**
     * @brief Hook method to handle the custom context menu actions that were added
     * with addCustomContextMenuActions.
     *
     * @param action the action that was chosen by the user, may be null if none
     *               of the custom actions were selected
     * @param album the tag on which the context menu was requested. May be null
     *              if there was no
     */
    virtual void handleCustomContextMenuAction(QAction* action,
                                               const AlbumPointer<Album>& album);

    //@}

    //@{

    /// Other helper methods.

    void mousePressEvent(QMouseEvent* e)                                        override;

    void rowsInserted(const QModelIndex& index, int start, int end)             override;
    void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)    override;
    void startDrag(Qt::DropActions supportedActions)                            override;
    void dragEnterEvent(QDragEnterEvent* e)                                     override;
    void dragMoveEvent(QDragMoveEvent* e)                                       override;
    void dragLeaveEvent(QDragLeaveEvent* e)                                     override;
    void dropEvent(QDropEvent* e)                                               override;

    virtual void middleButtonPressed(Album* a);
    virtual QPixmap pixmapForDrag(const QStyleOptionViewItem& option, QList<QModelIndex> indexes);

    virtual void setAlbumFilterModel(AlbumFilterModel* const filterModel);
    void setAlbumModel(AbstractSpecificAlbumModel* const model);

    //@}

protected:

    AbstractSpecificAlbumModel* m_albumModel            = nullptr;
    AlbumFilterModel*           m_albumFilterModel      = nullptr;
    AlbumModelDragDropHandler*  m_dragDropHandler       = nullptr;

    int                         m_lastScrollBarValue    = 0;
    bool                        m_checkOnMiddleClick    = false;
    bool                        m_restoreCheckState     = false;
    Flags                       m_flags                 = DefaultFlags;

private:

    void saveStateRecursive(const QModelIndex& index,
                            QList<int>& selection, QList<int>& expansion);

    /**
     * @brief Restores the state of the index and all sub-indexes if there is an entry
     * for this index in stateStore. Every album that is restored is removed
     * from the stateStore.
     *
     * @param index the index to start restoring
     * @param stateStore states indexed by album id
     */
    void restoreStateForHierarchy(const QModelIndex& index,
                                  const QMap<int, Digikam::State>& stateStore);

    /**
     * @brief Restore the state for this index.
     */
    void restoreState(const QModelIndex& index,
                      const QMap<int, Digikam::State>& stateStore);

    /**
     * @brief Creates the context menu.
     *
     * @param event the event that requested the menu
     */
    void contextMenuEvent(QContextMenuEvent* event)                             override;

private Q_SLOTS:

    /**
     * @brief Adapts the columns in between the given model indices to the content
     * size. This can be connected to dataChanged.
     *
     * @param topLeft top left index of changed data
     * @param bottomRight index of changed data
     */
    void adaptColumnsOnDataChange(const QModelIndex& topLeft, const QModelIndex& bottomRight);

    /**
     * @brief Adapt the column sizes to new contents. This can be connected to all
     * signals indicating row changes.
     *
     * @param parent the parent index of changed rows
     * @param start the start row changed under the parent
     * @param end the end row changed under the parent
     */
    void adaptColumnsOnRowChange(const QModelIndex& parent, int start, int end);

    /**
     * @brief Adapts the column sizes if the layout changes.
     */
    void adaptColumnsOnLayoutChange();

    /**
     * @brief This slot is used to ensure that after searching for entries the correct
     * album is selected again. Therefore it tracks new selections.
     */
    void currentAlbumChangedForBackupSelection(Album* currentAlbum);

    /**
     * @note This slots is used to fix bug 400960.
     */
    void slotScrollBarValueChanged(int value);
    void slotScrollBarActionTriggered(int action);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::AbstractAlbumTreeView::Flags)
