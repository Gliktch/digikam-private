/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-23
 * Description : Qt Model for Albums
 *
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractItemModel>
#include <QHash>
#include <QPixmap>
#include <QSet>

// Local includes

#include "album.h"
#include "digikam_export.h"

namespace Digikam
{

class Album;
class AlbumManager;
class AlbumModelDragDropHandler;

class DIGIKAM_GUI_EXPORT AbstractAlbumModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    /**
     *  AbstractAlbumModel is the abstract base class for all models that
     *  present Album objects as managed by AlbumManager.
     *  You will want to create an instance of the base classes.
     */
    enum RootAlbumBehavior
    {
        /**
         * The root album will be included as a single parent item
         * with all top-level album as children
         */
        IncludeRootAlbum,
        /**
         * The root album will not be included, but all top-level album
         * are represented as top-level items in this view
         */
        IgnoreRootAlbum
    };

    enum AlbumDataRole
    {
        /// Returns the album title. Principally the same as display role, but without any additions.
        AlbumTitleRole    = Qt::UserRole,
        /// Returns the Album::Type of the associated album
        AlbumTypeRole     = Qt::UserRole + 1,
        /// Returns a pointer to the associated Album object
        AlbumPointerRole  = Qt::UserRole + 2,
        /// Returns the id of the associated Album object
        AlbumIdRole       = Qt::UserRole + 3,
        /// Returns the global id (unique across all album types)
        AlbumGlobalIdRole = Qt::UserRole + 4,
        /// Returns the data to sort on
        AlbumSortRole     = Qt::UserRole + 5
    };

public:

    /**
     * Create an AbstractAlbumModel object for albums with the given type.
     * Pass the root album if it is already available.
     * Do not use this class directly, but one of the subclasses.
     */
    explicit AbstractAlbumModel(Album::Type albumType,
                                Album* const rootAlbum,
                                RootAlbumBehavior rootBehavior = IncludeRootAlbum,
                                QObject* const parent = nullptr);
    ~AbstractAlbumModel() override;

    /**
     * Set current index from QDragMoveEvent
     */
    void setDropIndex(const QModelIndex& index);

    /**
     * Set a drag drop handler
     */
    void setDragDropHandler(AlbumModelDragDropHandler* handler);

    /**
     * Returns the drag drop handler, or 0 if none is installed
     */
    AlbumModelDragDropHandler* dragDropHandler()    const;

    /**
     * Returns the album object associated with the given model index
     */
    Album* albumForIndex(const QModelIndex& index)  const;

    /**
     * Return the QModelIndex for the given album, or an invalid index if
     * the album is not contained in this model.
     */
    QModelIndex indexForAlbum(Album* album)         const;

    /**
     * Returns the album represented by the index. In contrast to albumForIndex(),
     * the index can be from any proxy model, as long as an AbstractAlbumModel is at the end.
     */
    static Album* retrieveAlbum(const QModelIndex& index);

    Album* rootAlbum()                              const;

    /**
     * Return the index corresponding to the root album. If the policy is IgnoreRootAlbum, this is an invalid index.
     */
    QModelIndex rootAlbumIndex()                    const;

    /**
     * Returns the root album behavior set for this model
     */
    RootAlbumBehavior rootAlbumBehavior()           const;

    /**
     * Returns the Album::Type of the contained albums
     */
    Album::Type albumType()                         const;

    /**
     * Returns true if the album model a face tag model
     */
    bool isFaceTagModel()                           const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)                                             const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole)                       const override;
    int rowCount(const QModelIndex& parent = QModelIndex())                                                         const override;
    int columnCount(const QModelIndex& parent = QModelIndex())                                                      const override;
    Qt::ItemFlags flags(const QModelIndex& index)                                                                   const override;
    bool hasChildren(const QModelIndex& parent = QModelIndex())                                                     const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex())                               const override;
    QModelIndex parent(const QModelIndex& index)                                                                    const override;

    Qt::DropActions supportedDropActions()                                                                          const override;
    QStringList mimeTypes()                                                                                         const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)       override;
    QMimeData* mimeData(const QModelIndexList& indexes)                                                             const override;

Q_SIGNALS:

    /**
     * This is initialized once after creation, if the root album becomes available,
     * if it was not already available at time of construction.
     * This is emitted regardless of root album policy.
     */
    void rootAlbumAvailable();

protected:

    /**
     * Switch on drag and drop globally for all items. Default is true.
     * For per-item cases reimplement itemFlags().
     */
    void setEnableDrag(bool enable);
    void setEnableDrop(bool enable);
    void setFaceTagModel(bool enable);

    /**
     * @note these can be reimplemented in a subclass
     */

    /// For subclassing convenience: A part of the implementation of data()
    virtual QVariant albumData(Album* a, int role)  const;

    /// For subclassing convenience: A part of the implementation of data()
    virtual QVariant decorationRoleData(Album* a)   const;

    /// For subclassing convenience: A part of the implementation of data()
    virtual QVariant fontRoleData(Album* a)         const;

    /// For subclassing convenience: A part of the implementation of data()
    virtual QVariant sortRoleData(Album* a)         const;

    /// For subclassing convenience: A part of the implementation of headerData()
    virtual QString columnHeader()                  const;

    /// For subclassing convenience: A part of the implementation of itemFlags()
    virtual Qt::ItemFlags itemFlags(Album* album)   const;

    /**
     * Returns true for those and only those albums that shall be contained in this model.
     * They must have a common root album, which is set in the constructor.
     */
    virtual bool filterAlbum(Album* album)          const;

    /// Notification when an entry is removed
    virtual void albumCleared(Album* /*album*/) {};

    /// Notification when all entries are removed
    virtual void allAlbumsCleared()             {};

protected Q_SLOTS:

    void slotAlbumAboutToBeAdded(Album* album, Album* parent, Album* prev);
    void slotAlbumAdded(Album*);
    void slotAlbumAboutToBeDeleted(Album* album);
    void slotAlbumHasBeenDeleted(Album* album);
    void slotAlbumsCleared();
    void slotAlbumIconChanged(Album* album);
    void slotAlbumRenamed(Album* album);

private:

    class Private;
    Private* const d = nullptr;
};

// ------------------------------------------------------------------

class DIGIKAM_GUI_EXPORT AbstractSpecificAlbumModel : public AbstractAlbumModel
{
    Q_OBJECT

public:

    /// Abstract base class, do not instantiate.
    explicit AbstractSpecificAlbumModel(Album::Type albumType,
                                        Album* const rootAlbum,
                                        RootAlbumBehavior rootBehavior = IncludeRootAlbum,
                                        QObject* const parent = nullptr);

protected:

    QString  columnHeader() const override;
    virtual void setColumnHeader(const QString& header);

    /// You need to call this from your constructor if you intend to load the thumbnail facilities of this class
    void setupThumbnailLoading();
    void emitDataChangedForChildren(Album* album);

protected Q_SLOTS:

    void slotGotThumbnailFromIcon(Album* album, const QPixmap& thumbnail);
    void slotThumbnailLost(Album* album);
    void slotReloadThumbnails();

protected:

    QString m_columnHeader;
};

// ------------------------------------------------------------------

class DIGIKAM_GUI_EXPORT AbstractCountingAlbumModel : public AbstractSpecificAlbumModel
{
    Q_OBJECT

public:

    /// Supports displaying a count alongside the album name in DisplayRole

    explicit AbstractCountingAlbumModel(Album::Type albumType,
                                        Album* const rootAlbum,
                                        RootAlbumBehavior rootBehavior = IncludeRootAlbum,
                                        QObject* const parent = nullptr);
    ~AbstractCountingAlbumModel() override;

    bool showCount()                        const;

    /**
     * Returns the number of included items for this album.
     *
     * @return positive value or -1 if unknown
     */
    virtual int albumCount(Album* album)    const;

protected:

    /**
     * Call this method in children class constructors to init signal/slots connections.
     */
    void setup();

public Q_SLOTS:

    /// Call to enable or disable showing the count. Default is false.
    void setShowCount(bool show);

    /**
     * Enable displaying the count. Set a map of album id -> count (excluding children).
     * If an album is not contained, no count is displayed. To display a count of 0,
     * there must be an entry album id -> 0.
     */
    void setCountHash(const QHash<int, int>& idCountHash);

    /**
     * Displays only the count of the album, without adding child albums' counts.
     * This is the default.
     * Can connect to QTreeView's expanded() signal.
     */
    void excludeChildrenCount(const QModelIndex& index);

    /**
     * Displays sum of the count of the album and child albums' counts.
     * Can connect to QTreeView's collapsed() signal.
     */
    void includeChildrenCount(const QModelIndex& index);

protected:

    /// If you do not use setCountHash, excludeChildrenCount and includeChildrenCount, you can set a count here.
    void setCount(Album* album, int count);

    /// need to implement in subclass
    virtual Album* albumForId(int id)       const = 0;

    /// Can reimplement in subclass
    virtual QString albumName(Album* a)     const;

    /// Reimplemented from parent classes
    QVariant albumData(Album* a, int role)  const override;
    void albumCleared(Album* album) override;
    void allAlbumsCleared() override;

protected Q_SLOTS:

    void slotAlbumMoved(Album* album);

private:

    void updateCount(Album* const album);

private:

    class Private;
    Private* const d = nullptr;
};

// ------------------------------------------------------------------

class DIGIKAM_GUI_EXPORT AbstractCheckableAlbumModel : public AbstractCountingAlbumModel
{
    Q_OBJECT

public:

    /**
     * Abstract base class that manages the check state of Albums.
     * Call setCheckable(true) to enable checkable albums.
     */

    explicit AbstractCheckableAlbumModel(Album::Type albumType,
                                         Album* const rootAlbum,
                                         RootAlbumBehavior rootBehavior = IncludeRootAlbum,
                                         QObject* const parent = nullptr);
    ~AbstractCheckableAlbumModel() override;

    /// Triggers if the albums in this model are checkable
    void setCheckable(bool isCheckable);
    bool isCheckable()                      const;

    /**
     * Triggers if the root album is checkable.
     * Only applicable if the root album is contained at all, and if isCheckable() is true.
     */
    void setRootCheckable(bool rootIsCheckable);
    bool rootIsCheckable()                  const;

    /**
     * Triggers if the albums in this model are tristate.
     * Used to allow the user to actively set a third state,
     * don't use if you only want to display a third state.
     * Note that you want to set setCheckable(true) before.
     */
    void setTristate(bool isTristate);
    bool isTristate()                       const;

    /**
     * Sets a special tristate mode, which offers the
     * three modes "unchecked", "added" and "excluded",
     * where "excluded" corresponds to partially checked internally,
     * but is reflected in the treeview through the decoration only.
     */
    void setAddExcludeTristate(bool b);
    bool isAddExcludeTristate()             const;

    /// Returns if the given album has the check state Checked
    bool isChecked(Album* album)            const;

    /// Returns the check state of the album
    Qt::CheckState checkState(Album* album) const;

    /// Returns a list of album with check state Checked
    QList<Album*> checkedAlbums()           const;

    /// Returns a list of album with partially check state Checked
    QList<Album*> partiallyCheckedAlbums()  const;

    /**
     * If an item gets checked, all childs get checked as well,
     * If an item gets unchecked, all childs get unchecked as well
     */
    void setRecursive(bool recursive);

public Q_SLOTS:

    /// Sets the check state of album to Checked or Unchecked
    void setChecked(Album* album, bool isChecked);

    /// Sets the check state of the album
    void setCheckState(Album* album, Qt::CheckState state);

    /// Toggles the check state of album between Checked or Unchecked
    void toggleChecked(Album* album);

    /// Resets the checked state of all albums to Qt::Unchecked
    void resetAllCheckedAlbums();

    /// Resets the checked state of all albums under the given parent
    void resetCheckedAlbums(const QModelIndex& parent = QModelIndex());

    /// Resets the checked state of all parents of the child including it.
    void resetCheckedParentAlbums(const QModelIndex& child);

    /// Checks all albums beneath the given parent
    void checkAllAlbums(const QModelIndex& parent = QModelIndex());

    /// Checks all parent albums starting at the child, including it.
    void checkAllParentAlbums(const QModelIndex& child);

    /// Inverts the checked state of all albums under the given parent.
    void invertCheckedAlbums(const QModelIndex& parent = QModelIndex());

    /// Sets the checked state recursively for all children of but not for the given album.
    void setCheckStateForChildren(Album* album, Qt::CheckState state);

    /// Sets the checked state recursively for all parents of but not for the given album.
    void setCheckStateForParents(Album* album, Qt::CheckState state);

Q_SIGNALS:

    /**
     * Emitted when the check state of an album changes.
     * checkState contains the new Qt::CheckState of album
     */
    void checkStateChanged(Album* album, Qt::CheckState checkState);

protected:

    /**
     * If in AddExcludeTristate mode, changes the icon as to indicate the state.
     */
    void prepareAddExcludeDecoration(Album* a, QPixmap& icon)                              const;

    QVariant albumData(Album* a, int role)                                                 const override;
    Qt::ItemFlags flags(const QModelIndex& index)                                          const override;

    /**
     * @note Do not call this function directly, use the setData(..., bool recursive)
     */
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole)       override;
    bool setData(const QModelIndex& index, const QVariant& value, int role, bool recursive);

    void albumCleared(Album* album)                                                              override;
    void allAlbumsCleared()                                                                      override;

private:

    void setDataForParents(const QModelIndex& child, const QVariant& value, int role = Qt::EditRole);
    void setDataForChildren(const QModelIndex& parent, const QVariant& value, int role = Qt::EditRole);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
