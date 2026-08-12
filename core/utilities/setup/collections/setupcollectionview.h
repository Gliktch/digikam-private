/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-15
 * Description : collections setup tab model/view
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractItemModel>
#include <QAbstractItemDelegate>
#include <QList>
#include <QTreeView>
#include <QStyledItemDelegate>
#include <QPushButton>
#include <QToolButton>

// Local includes

#include "collectionlocation.h"
#include "dwitemdelegate.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT SetupCollectionModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    /**
     * SetupCollectionModel is a model specialized for use in
     * SetupCollectionTreeView. It provides a reads the current collections
     * from CollectionManager, displays them in three categories,
     * and supports adding and removing collections
     */

    enum SetupCollectionDataRole
    {
        /// Returns true if the model index is the index of a category
        IsCategoryRole             = Qt::UserRole,

        /// The text for the category button
        CategoryButtonDisplayRole  = Qt::UserRole + 1,
        CategoryButtonMapId        = Qt::UserRole + 2,

        /// Returns true if the model index is the index of a button
        IsAppendRole               = Qt::UserRole + 3,

        /// The pixmap of the button
        AppendDecorationRole       = Qt::UserRole + 4,
        AppendMapId                = Qt::UserRole + 5,

        /// Returns true if the model index is the index of a button
        IsUpdateRole               = Qt::UserRole + 6,

        /// The pixmap of the button
        UpdateDecorationRole       = Qt::UserRole + 7,
        UpdateMapId                = Qt::UserRole + 8,

        /// Returns true if the model index is the index of a button
        IsDeleteRole               = Qt::UserRole + 9,

        /// The pixmap of the button
        DeleteDecorationRole       = Qt::UserRole + 10,
        DeleteMapId                = Qt::UserRole + 11
    };

    enum Columns
    {
        ColumnStatus       = 0,
        ColumnName         = 1,
        ColumnPath         = 2,
        ColumnAppendButton = 3,
        ColumnUpdateButton = 4,
        ColumnDeleteButton = 5,
        NumberOfColumns
    };

    enum Category
    {
        CategoryLocal      = 0,
        CategoryRemovable  = 1,
        CategoryRemote     = 2,
        NumberOfCategories
    };

public:

    explicit SetupCollectionModel(QObject* const parent = nullptr);
    ~SetupCollectionModel()                                                                         override;

public:

    /// Read collections from CollectionManager
    void loadCollections();

    /// Set a widget used as parent for dialogs and message boxes
    void setParentWidgetForDialogs(QWidget* const widget);

    /// Apply the changed settings to CollectionManager
    void apply();

    QModelIndex indexForCategory(Category category)                                           const;
    QList<QModelIndex> categoryIndexes()                                                      const;

    /// QAbstractItemModel implementation

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)                       const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex())                                   const override;
    int columnCount(const QModelIndex& parent = QModelIndex())                                const override;
    Qt::ItemFlags flags(const QModelIndex& index)                                             const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole)          override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex())         const override;
    QModelIndex parent(const QModelIndex& index)                                              const override;

Q_SIGNALS:

    /// Emitted when all collections were loaded and the model reset in loadCollections

    void collectionsLoaded();

public Q_SLOTS:

    /**
     * Forward category button clicked signals to this slot.
     * mappedId is retrieved with the CategoryButtonMapId role
     * for the model index of the button
     */
    void slotCategoryButtonPressed(int mappedId);

    /**
     * Forward button clicked signals to this slot.
     * mappedId is retrieved with the ButtonMapId role
     * for the model index of the button
     */
    void slotAppendPressed(int mappedId);
    void slotUpdatePressed(int mappedId);
    void slotDeletePressed(int mappedId);

public:

    QString lastAddedCollectionPath;

protected Q_SLOTS:

    void addCollection(int category);
    void updateCollection(int internalId);
    void deleteCollection(int internalId);

protected:

    QModelIndex indexForId(int id, int column)                                                const;

    bool askForNewCollectionPath(bool adding,
                                 int category,
                                 QString* const newPath,
                                 QString* const newLabel);
    bool askForNewCollectionCategory(int* const category);

    int categoryButtonMapId(const QModelIndex& index)                                         const;
    int buttonMapId(const QModelIndex& index)                                                 const;

    static Category typeToCategory(CollectionLocation::Type type);

protected:

    class Item
    {
    public:

        Item();
        explicit Item(const CollectionLocation& location);
        Item(const QString& path, const QString& label, SetupCollectionModel::Category category);

    public:

        CollectionLocation location;
        QStringList        childs;
        QString            label;
        QString            path;
        int                parentId  = 0;
        int                orgIndex  = 0;
        bool               appended  = false;
        bool               updated   = false;
        bool               deleted   = false;
    };

protected:

    QList<Item> m_collections;
    QWidget*    m_dialogParentWidget = nullptr;

private Q_SLOTS:

    void slotHelp();

private:

    static const int INTERNALID = 65535;
};

// -----------------------------------------------------------------------

class SetupCollectionTreeView : public QTreeView
{
    Q_OBJECT

public:

    explicit SetupCollectionTreeView(QWidget* const parent = nullptr);

    void setModel(SetupCollectionModel* model);

protected Q_SLOTS:

    void modelLoadedCollections();

private:

    void setModel(QAbstractItemModel* model)                                      override
    {
        setModel(static_cast<SetupCollectionModel*>(model));
    }
};

// -----------------------------------------------------------------------

class SetupCollectionDelegate : public DWItemDelegate
{
    Q_OBJECT

public:

    explicit SetupCollectionDelegate(QAbstractItemView* const view,
                                     QObject* const parent = nullptr);
    ~SetupCollectionDelegate()                                                    override;

    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index)                         const override;

    bool     editorEvent(QEvent* event,
                         QAbstractItemModel* model,
                         const QStyleOptionViewItem& option,
                         const QModelIndex& index)                                override;

    void     paint(QPainter* painter,
                   const QStyleOptionViewItem& option,
                   const QModelIndex& index)                                const override;

    void     setEditorData(QWidget* editor,
                           const QModelIndex& index)                        const override;

    void     setModelData(QWidget* editor,
                          QAbstractItemModel* model,
                          const QModelIndex& index)                         const override;

    QSize    sizeHint(const QStyleOptionViewItem& option,
                      const QModelIndex& index)                             const override;

    void     updateEditorGeometry(QWidget* editor,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index)                 const override;

    QList<QWidget*> createItemWidgets(const QModelIndex& index)             const override;
    void            updateItemWidgets(const QList<QWidget*>& widgets,
                                      const QStyleOptionViewItem& option,
                                      const QPersistentModelIndex& index)   const override;

Q_SIGNALS:

    void categoryButtonPressed(int mappedId)                                const;     // clazy:exclude=const-signal-or-slot
    void appendPressed(int mappedId)                                        const;     // clazy:exclude=const-signal-or-slot
    void updatePressed(int mappedId)                                        const;     // clazy:exclude=const-signal-or-slot
    void deletePressed(int mappedId)                                        const;     // clazy:exclude=const-signal-or-slot

protected:

    QStyledItemDelegate* m_styledDelegate           = nullptr;

    QPushButton*         m_samplePushButton         = nullptr;
    QToolButton*         m_sampleAppendButton       = nullptr;
    QToolButton*         m_sampleUpdateButton       = nullptr;
    QToolButton*         m_sampleDeleteButton       = nullptr;
    int                  m_categoryMaxStyledWidth   = 0;
};

} // namespace Digikam
