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
 * SPDX-FileCopyrightText:      2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupcollectionview.h"

// Qt includes

#include <QGroupBox>
#include <QLabel>
#include <QDir>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QStandardPaths>
#include <QComboBox>
#include <QUrlQuery>
#include <QUrl>
#include <QIcon>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmessagebox.h"
#include "dfiledialog.h"
#include "applicationsettings.h"
#include "collectionmanager.h"
#include "newitemsfinder.h"
#include "dtextedit.h"
#include "digikam_globals.h"

namespace Digikam
{

SetupCollectionDelegate::SetupCollectionDelegate(QAbstractItemView* const view, QObject* const parent)
    : DWItemDelegate(view, parent)
{
    // We keep a standard delegate that does all the normal drawing work for us
    // DWItemDelegate handles the widgets, for the rest of the work we act as a proxy to m_styledDelegate

    m_styledDelegate = new QStyledItemDelegate(parent);

    // forward all signals

    connect(m_styledDelegate, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
            this, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)));

    connect(m_styledDelegate, SIGNAL(commitData(QWidget*)),
            this, SIGNAL(commitData(QWidget*)));

    connect(m_styledDelegate, SIGNAL(sizeHintChanged(QModelIndex)),
            this, SIGNAL(sizeHintChanged(QModelIndex)));

    // For size hint. To get a valid size hint, the widgets seem to need a parent widget

    m_samplePushButton   = new QPushButton(view);
    m_samplePushButton->hide();
    m_sampleAppendButton = new QToolButton(view);
    m_sampleAppendButton->hide();
    m_sampleUpdateButton = new QToolButton(view);
    m_sampleUpdateButton->hide();
    m_sampleDeleteButton = new QToolButton(view);
    m_sampleDeleteButton->hide();
}

SetupCollectionDelegate::~SetupCollectionDelegate()
{
}

QList<QWidget*> SetupCollectionDelegate::createItemWidgets(const QModelIndex& /*index*/) const
{
    // We only need a push button for certain indexes and two tool button for others,
    // but we have no index here, but need to provide the widgets for each index

    QList<QWidget*> list;
    QPushButton* const pushButton   = new QPushButton();
    list << pushButton;

    connect(pushButton, &QPushButton::clicked,
            this, [this, pushButton]()
        {
            Q_EMIT categoryButtonPressed(pushButton->property("id").toInt());
        }
    );

    QToolButton* const appendButton = new QToolButton();
    appendButton->setToolTip(i18nc("@info:tooltip", "Append a path to the collection."));
    appendButton->setAutoRaise(true);
    list << appendButton;

    connect(appendButton, &QToolButton::clicked,
            this, [this, appendButton]()
        {
            Q_EMIT appendPressed(appendButton->property("id").toInt());
        }
    );

    QToolButton* const updateButton = new QToolButton();
    updateButton->setToolTip(i18nc("@info:tooltip", "Updates the path of the collection."));
    updateButton->setAutoRaise(true);
    list << updateButton;

    connect(updateButton, &QToolButton::clicked,
            this, [this, updateButton]()
        {
            Q_EMIT updatePressed(updateButton->property("id").toInt());
        }
    );

    QToolButton* const deleteButton = new QToolButton();
    deleteButton->setToolTip(i18nc("@info:tooltip", "Removes the collection from digiKam."));
    deleteButton->setAutoRaise(true);
    list << deleteButton;

    connect(deleteButton, &QToolButton::clicked,
            this, [this, deleteButton]()
        {
            Q_EMIT deletePressed(deleteButton->property("id").toInt());
        }
    );

    return list;
}

QSize SetupCollectionDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // get default size hint

    QSize hint = m_styledDelegate->sizeHint(option, index);

    // We need to handle those two cases where we display widgets

    if      (index.data(SetupCollectionModel::IsCategoryRole).toBool())
    {
        // get the largest size hint for the icon/text of all category entries

        int maxStyledWidth = 0;
        const auto idx     = static_cast<const SetupCollectionModel*>(index.model())->categoryIndexes();

        for (const QModelIndex& catIndex : idx)
        {
            maxStyledWidth = qMax(maxStyledWidth, m_styledDelegate->sizeHint(option, catIndex).width());
        }

        const_cast<SetupCollectionDelegate*>(this)->m_categoryMaxStyledWidth = maxStyledWidth;

        // set real text on sample button to compute correct size hint

        m_samplePushButton->setText(index.data(SetupCollectionModel::CategoryButtonDisplayRole).toString());
        QSize widgetHint = m_samplePushButton->sizeHint();

        // add largest of the icon/text sizes (so that all buttons are aligned) and our button size hint

        hint.setWidth(m_categoryMaxStyledWidth + widgetHint.width());
        hint.setHeight(qMax(hint.height(), widgetHint.height()));
    }
    else if (index.data(SetupCollectionModel::IsAppendRole).toBool())
    {
        // set real pixmap on sample button to compute correct size hint

        m_sampleAppendButton->setIcon(index.data(SetupCollectionModel::AppendDecorationRole).value<QIcon>());
        QSize widgetHint = m_sampleAppendButton->sizeHint();

        // combine hints

        hint.setWidth(hint.width() + widgetHint.width());
        hint.setHeight(qMax(hint.height(), widgetHint.height()));
    }
    else if (index.data(SetupCollectionModel::IsUpdateRole).toBool())
    {
        // set real pixmap on sample button to compute correct size hint

        m_sampleUpdateButton->setIcon(index.data(SetupCollectionModel::UpdateDecorationRole).value<QIcon>());
        QSize widgetHint = m_sampleUpdateButton->sizeHint();

        // combine hints

        hint.setWidth(hint.width() + widgetHint.width());
        hint.setHeight(qMax(hint.height(), widgetHint.height()));
    }
    else if (index.data(SetupCollectionModel::IsDeleteRole).toBool())
    {
        // set real pixmap on sample button to compute correct size hint

        m_sampleDeleteButton->setIcon(index.data(SetupCollectionModel::DeleteDecorationRole).value<QIcon>());
        QSize widgetHint = m_sampleDeleteButton->sizeHint();

        // combine hints

        hint.setWidth(hint.width() + widgetHint.width());
        hint.setHeight(qMax(hint.height(), widgetHint.height()));
    }

    return hint;
}

void SetupCollectionDelegate::updateItemWidgets(const QList<QWidget*>& widgets,
                                                const QStyleOptionViewItem& option,
                                                const QPersistentModelIndex& index) const
{
    QPushButton* const pushButton   = static_cast<QPushButton*>(widgets.at(0));
    QToolButton* const appendButton = static_cast<QToolButton*>(widgets.at(1));
    QToolButton* const updateButton = static_cast<QToolButton*>(widgets.at(2));
    QToolButton* const deleteButton = static_cast<QToolButton*>(widgets.at(3));

    if      (index.data(SetupCollectionModel::IsCategoryRole).toBool())
    {
        // set text from model

        pushButton->setText(index.data(SetupCollectionModel::CategoryButtonDisplayRole).toString());

        // resize according to size hint

        pushButton->resize(pushButton->sizeHint());

        // move to position in line. We have cached the icon/text size hint from sizeHint()

        pushButton->move(m_categoryMaxStyledWidth, (option.rect.height() - pushButton->height()) / 2);
        pushButton->show();
        appendButton->hide();
        updateButton->hide();
        deleteButton->hide();

        pushButton->setEnabled(itemView()->isEnabled());
        pushButton->setProperty("id", index.data(SetupCollectionModel::CategoryButtonMapId));
    }
    else if (index.data(SetupCollectionModel::IsAppendRole).toBool())
    {
        appendButton->setIcon(index.data(SetupCollectionModel::AppendDecorationRole).value<QIcon>());
        appendButton->resize(appendButton->sizeHint());
        appendButton->move(0, (option.rect.height() - appendButton->height()) / 2);
        appendButton->show();
        pushButton->hide();

        appendButton->setEnabled(itemView()->isEnabled());
        appendButton->setProperty("id", index.data(SetupCollectionModel::AppendMapId));
    }
    else if (index.data(SetupCollectionModel::IsUpdateRole).toBool())
    {
        updateButton->setIcon(index.data(SetupCollectionModel::UpdateDecorationRole).value<QIcon>());
        updateButton->resize(updateButton->sizeHint());
        updateButton->move(0, (option.rect.height() - updateButton->height()) / 2);
        updateButton->show();
        pushButton->hide();

        updateButton->setEnabled(itemView()->isEnabled());
        updateButton->setProperty("id", index.data(SetupCollectionModel::UpdateMapId));
    }
    else if (index.data(SetupCollectionModel::IsDeleteRole).toBool())
    {
        deleteButton->setIcon(index.data(SetupCollectionModel::DeleteDecorationRole).value<QIcon>());
        deleteButton->resize(deleteButton->sizeHint());
        deleteButton->move(0, (option.rect.height() - deleteButton->height()) / 2);
        deleteButton->show();
        pushButton->hide();

        deleteButton->setEnabled(itemView()->isEnabled());
        deleteButton->setProperty("id", index.data(SetupCollectionModel::DeleteMapId));
    }
    else
    {
        pushButton->hide();
        appendButton->hide();
        updateButton->hide();
        deleteButton->hide();
    }
}

QWidget* SetupCollectionDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return m_styledDelegate->createEditor(parent, option, index);
}

bool SetupCollectionDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                          const QStyleOptionViewItem& option, const QModelIndex& index)
{
    return static_cast<QAbstractItemDelegate*>(m_styledDelegate)->editorEvent(event, model, option, index);
}

void SetupCollectionDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    m_styledDelegate->paint(painter, option, index);
}

void SetupCollectionDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    m_styledDelegate->setEditorData(editor, index);
}

void SetupCollectionDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    m_styledDelegate->setModelData(editor, model, index);
}

void SetupCollectionDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    m_styledDelegate->updateEditorGeometry(editor, option, index);
}

// ------------- View ----------------- //

SetupCollectionTreeView::SetupCollectionTreeView(QWidget* const parent)
    : QTreeView(parent)
{
    setHeaderHidden(true);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setExpandsOnDoubleClick(false);

    // Set custom delegate

    setItemDelegate(new SetupCollectionDelegate(this, this));
}

void SetupCollectionTreeView::setModel(SetupCollectionModel* collectionModel)
{
    if (model())
    {
        disconnect(model(), nullptr, this, nullptr);
    }

    // we need to do some things after the model has loaded its data

    connect(collectionModel, SIGNAL(collectionsLoaded()),
            this, SLOT(modelLoadedCollections()));

    // connect button click signals from the delegate to the model

    connect(static_cast<SetupCollectionDelegate*>(itemDelegate()), SIGNAL(categoryButtonPressed(int)),
            collectionModel, SLOT(slotCategoryButtonPressed(int)));

    connect(static_cast<SetupCollectionDelegate*>(itemDelegate()), SIGNAL(appendPressed(int)),
            collectionModel, SLOT(slotAppendPressed(int)));

    connect(static_cast<SetupCollectionDelegate*>(itemDelegate()), SIGNAL(updatePressed(int)),
            collectionModel, SLOT(slotUpdatePressed(int)));

    connect(static_cast<SetupCollectionDelegate*>(itemDelegate()), SIGNAL(deletePressed(int)),
            collectionModel, SLOT(slotDeletePressed(int)));

    // give model a widget to use as parent for message boxes

    collectionModel->setParentWidgetForDialogs(this);

    QTreeView::setModel(collectionModel);
}

void SetupCollectionTreeView::modelLoadedCollections()
{
    // make category entries span the whole line

    for (int i = 0 ; i < model()->rowCount(QModelIndex()) ; ++i)
    {
        setFirstColumnSpanned(i, QModelIndex(), true);
    }

    // show all entries

    expandAll();

    // Resize name and path column

    header()->setSectionResizeMode(SetupCollectionModel::ColumnName, QHeaderView::Stretch);
    header()->setSectionResizeMode(SetupCollectionModel::ColumnPath, QHeaderView::Stretch);

    // Resize last column, so that delete button is always rightbound

    header()->setStretchLastSection(false); // defaults to true
    header()->setSectionResizeMode(SetupCollectionModel::ColumnAppendButton, QHeaderView::Fixed);
    resizeColumnToContents(SetupCollectionModel::ColumnAppendButton);
    header()->setSectionResizeMode(SetupCollectionModel::ColumnUpdateButton, QHeaderView::Fixed);
    resizeColumnToContents(SetupCollectionModel::ColumnUpdateButton);
    header()->setSectionResizeMode(SetupCollectionModel::ColumnDeleteButton, QHeaderView::Fixed);
    resizeColumnToContents(SetupCollectionModel::ColumnDeleteButton);

    // Resize first column
    // This is more difficult because we need to ignore the width of the category entries,
    // which are formally location in the first column (although spanning the whole line).
    // resizeColumnToContents fails therefore.

    const SetupCollectionModel* const collectionModel = static_cast<SetupCollectionModel*>(model());
    QModelIndex categoryIndex                         = collectionModel->indexForCategory(SetupCollectionModel::CategoryLocal);
    QModelIndex firstChildOfFirstCategory             = collectionModel->index(0, SetupCollectionModel::ColumnStatus, categoryIndex);
    QSize hint                                        = sizeHintForIndex(firstChildOfFirstCategory);
    setColumnWidth(SetupCollectionModel::ColumnStatus, hint.width() + indentation());
}

// ------------- Model ----------------- //

SetupCollectionModel::Item::Item()
    : parentId(INTERNALID)
{
}

SetupCollectionModel::Item::Item(const CollectionLocation& location)
    : location(location)
{
    parentId = SetupCollectionModel::typeToCategory(location.type());
}

SetupCollectionModel::Item::Item(const QString& path, const QString& label, SetupCollectionModel::Category category)
    : label   (label),
      path    (path),
      parentId(category)
{
}

/**
 * Internal data structure:
 *
 * The category entries get a model index with INTERNALID and are identified by their row().
 * The item entries get the index in m_collections as INTERNALID.
 * No item is ever removed from m_collections, deleted entries are only marked as such.
 *
 * Items have a location, a parentId, and a name and label field.
 * parentId always contains the category, needed to implement parent().
 * The location is the location if it exists, or is null if the item was added.
 * Name and label are null if unchanged, then the values from location are used.
 * They are valid if edited (label) or the location was added (both valid, location null).
 */

SetupCollectionModel::SetupCollectionModel(QObject* const parent)
    : QAbstractItemModel(parent)
{
}

SetupCollectionModel::~SetupCollectionModel()
{
}

void SetupCollectionModel::loadCollections()
{
    beginResetModel();

    m_collections.clear();
    QList<CollectionLocation> locations = CollectionManager::instance()->allLocations();

    for (const CollectionLocation& location : std::as_const(locations))
    {
        m_collections << Item(location);
        int idx = m_collections.size() - 1;

        if (location.type() == CollectionLocation::Network)
        {
            QUrl url(location.identifier);

            if (url.scheme() == QLatin1String("networkshareid"))
            {
                QUrlQuery q(url);
                const auto paths = q.allQueryItemValues(QLatin1String("mountpath"));

                for (const QString& path : paths)
                {
                    if (location.albumRootPath() != path)
                    {
                        Item item(location);
                        item.orgIndex = idx;
                        item.appended = true;
                        item.path     = path;
                        m_collections << item;
                        m_collections[idx].childs << path;
                    }
                }
            }
        }
    }

    endResetModel();

    Q_EMIT collectionsLoaded();
}

void SetupCollectionModel::apply()
{
    QList<int> newItems, deletedItems, updatedItems, renamedItems;

    for (int i = 0 ; i < m_collections.count() ; ++i)
    {
        const Item& item = m_collections.at(i);

        if      (item.appended)
        {
            continue;
        }
        else if (item.deleted && !item.location.isNull())
        {
            // if item was deleted and had a valid location, i.e. exists in DB

            deletedItems << i;
        }
        else if (!item.deleted && item.location.isNull())
        {
            // if item has no valid location, i.e. does not yet exist in db

            newItems << i;
        }
        else if (!item.deleted && !item.location.isNull())
        {
            // if item has a valid location, is updated or has changed its label

            if      (item.updated)
            {
                updatedItems << i;
            }
            else if (!item.label.isNull() && item.label != item.location.label())
            {
                renamedItems << i;
            }
        }
    }

    // Delete deleted items

    for (int i : std::as_const(deletedItems))
    {
        Item& item    = m_collections[i];
        CollectionManager::instance()->removeLocation(item.location);
        item.location = CollectionLocation();
    }

    // Add added items

    QList<Item> failedItems;

    for (int i : std::as_const(newItems))
    {
        Item& item = m_collections[i];
        CollectionLocation location;

        if (item.parentId == CategoryRemote)
        {
            location = CollectionManager::instance()->addNetworkLocation(QUrl::fromLocalFile(item.path), item.label);
        }
        else
        {
            location = CollectionManager::instance()->addLocation(QUrl::fromLocalFile(item.path), item.label);
        }

        if (location.isNull())
        {
            failedItems << item;
        }
        else
        {
            item.location = location;
            item.path.clear();
            item.label.clear();
        }
    }

    // Update collections

    for (int i : std::as_const(updatedItems))
    {
        Item& item  = m_collections[i];
        CollectionLocation location;

        int newType = CollectionLocation::VolumeHardWired;

        if      (item.parentId == CategoryRemovable)
        {
            newType = CollectionLocation::VolumeRemovable;
        }
        else if (item.parentId == CategoryRemote)
        {
            newType = CollectionLocation::Network;
        }

        QStringList pathList;
        pathList << item.path << item.childs;
        location    = CollectionManager::instance()->refreshLocation(item.location, newType,
                                                                     pathList, item.label);

        if (location.isNull())
        {
            failedItems << item;
        }
        else
        {
            item.location = location;
            item.path.clear();
            item.label.clear();
        }
    }

    // Rename collections

    for (int i : std::as_const(renamedItems))
    {
        Item& item = m_collections[i];
        CollectionManager::instance()->setLabel(item.location, item.label);
        item.label.clear();
    }

    // Handle any errors

    if (!failedItems.isEmpty())
    {
        QStringList failedPaths;

        for (const Item& item : std::as_const(failedItems))
        {
            failedPaths << QDir::toNativeSeparators(item.path);
        }

        DMessageBox::showInformationList(QMessageBox::Critical,
                                         m_dialogParentWidget,
                                         qApp->applicationName(),
                                         i18n("It was not possible to add a collection for the following paths:"),
                                         failedPaths);
    }

    // Trigger collection scan

    if (!newItems.isEmpty() || !updatedItems.isEmpty() || !deletedItems.isEmpty())
    {
        NewItemsFinder* const tool = new NewItemsFinder();
        tool->start();
    }
}

void SetupCollectionModel::setParentWidgetForDialogs(QWidget* widget)
{
    m_dialogParentWidget = widget;
}

void SetupCollectionModel::slotCategoryButtonPressed(int mappedId)
{
    addCollection(mappedId);
}

void SetupCollectionModel::slotAppendPressed(int mappedId)
{
    QString picPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    QUrl curl       = DFileDialog::getExistingDirectoryUrl(m_dialogParentWidget,
                                                           i18nc("@title:window", "Choose the Folder Containing your Collection"),
                                                           QUrl::fromLocalFile(picPath));

    curl            = curl.adjusted(QUrl::StripTrailingSlash);

    if (curl.isEmpty())
    {
        return;
    }

    bool foundPath = false;

    for (const Item& item : std::as_const(m_collections))
    {
        if (!item.deleted)
        {
            QStringList possiblePaths;

            possiblePaths << item.childs;

            if      (!item.path.isEmpty())
            {
                possiblePaths << item.path;
            }
            else if (!item.location.isNull())
            {
                possiblePaths << item.location.albumRootPath();
            }

            if (possiblePaths.contains(curl.toLocalFile()))
            {
                foundPath = true;
                break;
            }
        }
    }

    if (foundPath)
    {
        QMessageBox::warning(m_dialogParentWidget, i18nc("@title:window", "Problem Appending Collection"),
                                                   i18n("A collection with the path \"%1\" already exists.",
                                                        QDir::toNativeSeparators(curl.toLocalFile())));

        return;
    }

    QModelIndex ind = indexForId(mappedId, (int)ColumnStatus);

    if (!ind.isValid() || (mappedId >= m_collections.count()))
    {
        return;
    }

    Item& orgItem   = m_collections[ind.internalId()];

    Item item(curl.toLocalFile(), orgItem.label, (Category)orgItem.parentId);
    orgItem.path    = !orgItem.updated ? orgItem.location.albumRootPath()
                                       : orgItem.path;
    orgItem.label   = orgItem.location.label();
    orgItem.childs << curl.toLocalFile();
    orgItem.updated = true;

    item.orgIndex   = ind.internalId();
    item.location   = orgItem.location;
    item.appended   = true;

    int row         = rowCount(ind);

    beginInsertRows(ind, row, row);
    m_collections << item;
    endInsertRows();

    // only workaround for bug 182753

    Q_EMIT layoutChanged();
}

void SetupCollectionModel::slotUpdatePressed(int mappedId)
{
    updateCollection(mappedId);
}

void SetupCollectionModel::slotDeletePressed(int mappedId)
{
    deleteCollection(mappedId);
}

void SetupCollectionModel::addCollection(int category)
{
    if ((category < 0) || (category >= NumberOfCategories))
    {
        return;
    }

    QString label;
    QString path = lastAddedCollectionPath;

    if (askForNewCollectionPath(true, category, &path, &label))
    {
        // Add new item to model. Adding to CollectionManager is done in apply()!

        QModelIndex prnt = indexForCategory((Category)category);
        int row          = rowCount(prnt);

        beginInsertRows(prnt, row, row);
        m_collections << Item(path, label, (Category)category);
        endInsertRows();

        // only workaround for bug 182753

        Q_EMIT layoutChanged();
    }
}

/*
// NOTE: This code works, but is currently not used. Was intended as a workaround for 219876.

void SetupCollectionModel::emitDataChangedForChildren(const QModelIndex& parent)
{
    int rows    = rowCount(parent);
    int columns = columnCount(parent);

    Q_EMIT dataChanged(index(0, 0, parent), index(rows, columns, parent));

    for (int r = 0 ; r < rows ; ++r)
    {
        for (int c = 0 ; c < columns ; ++c)
        {
            QModelIndex i = index(r, c, parent);

            if (i.isValid())
            {
                emitDataChangedForChildren(i);
            }
        }
    }
}
*/

void SetupCollectionModel::updateCollection(int internalId)
{
    QModelIndex ind = indexForId(internalId, (int)ColumnStatus);

    if (!ind.isValid() || (internalId >= m_collections.count()))
    {
        return;
    }

    Item& item   = m_collections[ind.internalId()];
    int parentId = item.parentId;

    if (askForNewCollectionCategory(&parentId))
    {
        QString path = item.path;
        QString label;

        if (!item.location.isNull())
        {
            path = item.location.albumRootPath();
        }

        // Mark item as deleted so that
        // the path can be used again.

        item.deleted = true;

        if (askForNewCollectionPath(false, parentId, &path, &label))
        {
            item.parentId = parentId;
            item.label    = label;
            item.path     = path;
            item.updated  = true;

            // only workaround for bug 182753

            Q_EMIT layoutChanged();
        }

        item.deleted = false;
    }
}

void SetupCollectionModel::deleteCollection(int internalId)
{
    QModelIndex ind         = indexForId(internalId, (int)ColumnStatus);
    QModelIndex parentIndex = parent(ind);

    if (!ind.isValid() || (internalId >= m_collections.count()))
    {
        return;
    }

    int result    = QMessageBox::No;
    Item& item    = m_collections[ind.internalId()];
    QString label = data(indexForId(internalId, (int)ColumnName), Qt::DisplayRole).toString();

    Q_UNUSED(result);

    // Ask for confirmation

    if (item.appended)
    {
        result = QMessageBox::warning(m_dialogParentWidget,
                                      i18nc("@title:window", "Remove Path from the Collection?"),
                                      i18n("Do you want to remove the appended path \"%1\" from the collection \"%2\"?",
                                           item.path, label),
                                      QMessageBox::Yes | QMessageBox::No);
    }
    else
    {
        result = QMessageBox::warning(m_dialogParentWidget,
                                      i18nc("@title:window", "Remove Collection?"),
                                      i18n("Do you want to remove the collection \"%1\" from your list of collections?",
                                            label),
                                      QMessageBox::Yes | QMessageBox::No);
    }

    if (result == QMessageBox::Yes)
    {
        // Remove from model. Removing from CollectionManager is done in apply()!

        beginRemoveRows(parentIndex, ind.row(), ind.row());
        item.deleted = true;
        endRemoveRows();

        if      (item.appended)
        {
            Item& orgItem   = m_collections[item.orgIndex];

            orgItem.path    = orgItem.location.albumRootPath();
            orgItem.label   = orgItem.location.label();
            orgItem.childs.removeAll(item.path);
            orgItem.updated = true;
        }
        else if (!item.childs.isEmpty())
        {
            for (int i = 0 ; i < m_collections.count() ; ++i)
            {
                Item& remItem = m_collections[i];

                if (remItem.orgIndex == (int)ind.internalId())
                {
                    QModelIndex remIndex       = indexForId(i, (int)ColumnStatus);
                    QModelIndex remParentIndex = parent(remIndex);

                    beginRemoveRows(remParentIndex, remIndex.row(), remIndex.row());
                    remItem.deleted = true;
                    endRemoveRows();
                }
            }
        }

        // only workaround for bug 182753

        Q_EMIT layoutChanged();
    }
}

QVariant SetupCollectionModel::data(const QModelIndex& _index, int _role) const
{
    if (!_index.isValid())
    {
        return QVariant();
    }

    if (_index.internalId() == INTERNALID)
    {
        if (_index.column() == 0)
        {
            switch (_role)
            {
                case Qt::DisplayRole:
                {
                    switch (_index.row())
                    {
                        case CategoryLocal:
                        {
                            return i18n("Local Collections");
                        }

                        case CategoryRemovable:
                        {
                            return i18n("Collections on Removable Media");
                        }

                        case CategoryRemote:
                        {
                            return i18n("Collections on Network Shares");
                        }
                    }

                    break;
                }

                case Qt::DecorationRole:
                {
                    switch (_index.row())
                    {
                        case CategoryLocal:
                        {
                            return QIcon::fromTheme(QLatin1String("drive-harddisk"));
                        }

                        case CategoryRemovable:
                        {
                            return QIcon::fromTheme(QLatin1String("drive-removable-media"));
                        }

                        case CategoryRemote:
                        {
                            return QIcon::fromTheme(QLatin1String("network-wired-activated"));
                        }
                    }

                    break;
                }

                case IsCategoryRole:
                {
                    return true;
                }

                case CategoryButtonDisplayRole:
                {
                    return i18n("Add Collection");
                }

                case CategoryButtonMapId:
                {
                    return categoryButtonMapId(_index);
                }

                default:
                {
                    break;
                }
            }
        }
    }
    else
    {
        const Item& item = m_collections.at(_index.internalId());

        if ((_role == Qt::BackgroundRole) && item.appended)
        {
             return QPalette().alternateBase();
        }

        switch (_index.column())
        {
            case ColumnName:
            {
                if ((_role == Qt::DisplayRole) || (_role == Qt::EditRole))
                {
                    if (item.appended)
                    {
                        const Item& orgItem = m_collections.at(item.orgIndex);

                        if (!orgItem.label.isNull())
                        {
                            return orgItem.label;
                        }
                    }

                    if (!item.label.isNull())
                    {
                        return item.label;
                    }

                    if (!item.location.label().isNull())
                    {
                        return item.location.label();
                    }

                    return i18n("Col. %1", _index.row());
                }

                break;
            }

            case ColumnPath:
            {
                if ((_role == Qt::DisplayRole) || (_role == Qt::ToolTipRole))
                {
                    if (!item.path.isNull())
                    {
                        return QDir::toNativeSeparators(item.path);
                    }

                    // TODO: Path can be empty for items not available,
                    // query more info from CollectionManager

                    return QDir::toNativeSeparators(item.location.albumRootPath());
                }

                break;
            }

            case ColumnStatus:
            {
                if (_role == Qt::DecorationRole)
                {
                    if (item.updated)
                    {
                        return QIcon::fromTheme(QLatin1String("view-refresh"));
                    }

                    if (item.deleted)
                    {
                        return QIcon::fromTheme(QLatin1String("edit-delete"));
                    }

                    if (item.location.isNull())
                    {
                        return QIcon::fromTheme(QLatin1String("folder-new"));
                    }

                    if (item.appended)
                    {
                        return QIcon::fromTheme(QLatin1String("mail-attachment"));
                    }

                    switch (item.location.status())
                    {
                        case CollectionLocation::LocationAvailable:
                        {
                            return QIcon::fromTheme(QLatin1String("dialog-ok-apply"));
                        }

                        case CollectionLocation::LocationHidden:
                        {
                            return QIcon::fromTheme(QLatin1String("object-locked"));
                        }

                        case CollectionLocation::LocationUnavailable:
                        {
                            switch (item.parentId)
                            {
                                case CategoryLocal:
                                {
                                    return QIcon::fromTheme(QLatin1String("drive-harddisk")).pixmap(16, QIcon::Disabled);
                                }

                                case CategoryRemovable:
                                {
                                    return QIcon::fromTheme(QLatin1String("drive-removable-media-usb")).pixmap(16, QIcon::Disabled);
                                }

                                case CategoryRemote:
                                {
                                    return QIcon::fromTheme(QLatin1String("network-wired-activated")).pixmap(16, QIcon::Disabled);
                                }
                            }

                            break;
                        }

                        case CollectionLocation::LocationNull:
                        case CollectionLocation::LocationDeleted:
                        {
                            return QIcon::fromTheme(QLatin1String("edit-delete"));
                        }
                    }
                }
                else if (_role == Qt::ToolTipRole)
                {
                    switch (item.location.status())
                    {
                        case CollectionLocation::LocationUnavailable:
                        {
                            return i18n("This collection is currently not available.");
                        }

                        case CollectionLocation::LocationAvailable:
                        {
                            return i18n("No problems found, enjoy this collection.");
                        }

                        case CollectionLocation::LocationHidden:
                        {
                            return i18n("This collection is hidden.");
                        }

                        default:
                        {
                            break;
                        }
                    }
                }

                break;
            }

            case ColumnAppendButton:
            {
                switch (_role)
                {
                    case Qt::ToolTipRole:
                    {
                        return i18n("Append network path");
                    }

                    case IsAppendRole:
                    {
                        return ((item.location.type() == CollectionLocation::Network) && !item.appended);
                    }

                    case AppendDecorationRole:
                    {
                        return QIcon::fromTheme(QLatin1String("list-add"));
                    }

                    case AppendMapId:
                    {
                        return buttonMapId(_index);
                    }
                }

                break;
            }

            case ColumnUpdateButton:
            {
                switch (_role)
                {
                    case Qt::ToolTipRole:
                    {
                        return i18n("Update collection");
                    }

                    case IsUpdateRole:
                    {
                        return (!item.appended);
                    }

                    case UpdateDecorationRole:
                    {
                        return QIcon::fromTheme(QLatin1String("view-refresh"));
                    }

                    case UpdateMapId:
                    {
                        return buttonMapId(_index);
                    }
                }

                break;
            }

            case ColumnDeleteButton:
            {
                switch (_role)
                {
                    case Qt::ToolTipRole:
                    {
                        return i18n("Remove collection");
                    }

                    case IsDeleteRole:
                    {
                        return true;
                    }

                    case DeleteDecorationRole:
                    {
                        return QIcon::fromTheme(QLatin1String("edit-delete"));
                    }

                    case DeleteMapId:
                    {
                        return buttonMapId(_index);
                    }
                }

                break;
            }
        }
    }

    return QVariant();
}

QVariant SetupCollectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role == Qt::DisplayRole) && (orientation == Qt::Horizontal) && (section < NumberOfColumns))
    {
        switch (section)
        {
            case ColumnName:
            {
                return i18nc("#title: collection name",       "Name");
            }

            case ColumnPath:
            {
                return i18nc("#title: collection mount path", "Path");
            }

            case ColumnStatus:
            {
                return i18nc("#title: collection status",     "Status");
            }

            case ColumnAppendButton:
            {
                break;
            }

            case ColumnUpdateButton:
            {
                break;
            }

            case ColumnDeleteButton:
            {
                break;
            }
        }
    }

    return QVariant();
}

int SetupCollectionModel::rowCount(const QModelIndex& _parent) const
{
    if (!_parent.isValid())
    {
        return NumberOfCategories;    // Level 0: the three top level items
    }

    if (_parent.column() != 0)
    {
        return 0;
    }

    if (_parent.internalId() != INTERNALID)
    {
        return 0;                     // Level 2: no children
    }

    // Level 1: item children count

    int parentId = _parent.row();
    int rowCount = 0;

    for (const Item& item : std::as_const(m_collections))
    {
        if (!item.deleted && (item.parentId == parentId))
        {
            ++rowCount; // cppcheck-suppress useStlAlgorithm
        }
    }

    return rowCount;
}

int SetupCollectionModel::columnCount(const QModelIndex& /*_parent*/) const
{
    return NumberOfColumns;
}

Qt::ItemFlags SetupCollectionModel::flags(const QModelIndex& _index) const
{
    if (!_index.isValid())
    {
        return Qt::NoItemFlags;
    }

    if (_index.internalId() == INTERNALID)
    {
        return Qt::ItemIsEnabled;
    }
    else
    {
        Qt::ItemFlags flgs(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        switch (_index.column())
        {
            case ColumnName:
            {
                const Item& item = m_collections.at(_index.internalId());

                if (item.appended)
                {
                    return flgs;
                }

                return (flgs | Qt::ItemIsEditable);
            }

            default:
            {
                return flgs;
            }
        }
    }
}

bool SetupCollectionModel::setData(const QModelIndex& _index, const QVariant& _value, int _role)
{
    // only editable in one case

    if (
        _index.isValid()                    &&
        (_index.internalId() != INTERNALID) &&
        (_index.column() == ColumnName)     &&
        (_role == Qt::EditRole)
       )
    {
        Item& item = m_collections[_index.internalId()];
        item.label = _value.toString();

        Q_EMIT dataChanged(_index, _index);
    }

    return false;
}

QModelIndex SetupCollectionModel::index(int _row, int _column, const QModelIndex& _parent) const
{
    if      (!_parent.isValid())
    {
        if ((_row < NumberOfCategories) && (_row >= 0) && (_column == 0))
        {
            return createIndex(_row, 0, INTERNALID);
        }
    }
    else if ((_row >= 0) && (_column < NumberOfColumns))
    {
        // m_collections is a flat list with all entries, of all categories and also deleted entries.
        // The model indices contain as internal id the index to this list.

        int parentId = _parent.row();
        int rowCnt   = 0;

        for (int i = 0 ; i < m_collections.count() ; ++i)
        {
            const Item& item = m_collections.at(i);

            if (!item.deleted && (item.parentId == parentId))
            {
                if (rowCnt == _row)
                {
                    return createIndex(_row, _column, i);
                }

                ++rowCnt;
            }
        }
    }

    return QModelIndex();
}

QModelIndex SetupCollectionModel::parent(const QModelIndex& _index) const
{
    if (!_index.isValid())
    {
        return QModelIndex();
    }

    if (_index.internalId() == INTERNALID)
    {
        return QModelIndex();    // one of the three toplevel items
    }

    const Item& item = m_collections.at(_index.internalId());

    return createIndex(item.parentId, 0, INTERNALID);
}

QModelIndex SetupCollectionModel::indexForCategory(Category category) const
{
    return index(category, 0, QModelIndex());
}

QList<QModelIndex> SetupCollectionModel::categoryIndexes() const
{
    QList<QModelIndex> list;

    for (int cat = 0 ; cat < NumberOfCategories ; ++cat)
    {
        list << index(cat, 0, QModelIndex());
    }

    return list;
}

QModelIndex SetupCollectionModel::indexForId(int id, int column) const
{
    if (id >= m_collections.size())
    {
        return QModelIndex();
    }

    int   row             = 0;
    const Item& indexItem = m_collections.at(id);

    for (int i = 0 ; i < m_collections.count() ; ++i)
    {
        const Item& item = m_collections.at(i);

        if (!item.deleted && (item.parentId == indexItem.parentId))
        {
            if (i == id)
            {
                return createIndex(row, column, i);
            }

            ++row;
        }
    }

    return QModelIndex();
}

SetupCollectionModel::Category SetupCollectionModel::typeToCategory(CollectionLocation::Type type)
{
    switch (type)
    {
        default:
        case CollectionLocation::VolumeHardWired:
        {
            return CategoryLocal;
        }

        case CollectionLocation::VolumeRemovable:
        {
            return CategoryRemovable;
        }

        case CollectionLocation::Network:
        {
            return CategoryRemote;
        }
    }
}

bool SetupCollectionModel::askForNewCollectionPath(bool adding, int category, QString* const newPath, QString* const newLabel)
{
    QString picPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    if (newPath && !(*newPath).isEmpty() && QFileInfo::exists(*newPath))
    {
        picPath = *newPath;
    }

    QUrl curl       = DFileDialog::getExistingDirectoryUrl(m_dialogParentWidget,
                                                           i18nc("@title:window", "Choose the Folder Containing your Collection"),
                                                           QUrl::fromLocalFile(picPath));

    curl            = curl.adjusted(QUrl::StripTrailingSlash);

    if (curl.isEmpty())
    {
        return false;
    }

    lastAddedCollectionPath = curl.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toLocalFile();

    // Check path: First check with manager

    QString messageFromManager, deviceIcon;
    QList<CollectionLocation> assumeDeleted;

    for (const Item& item : std::as_const(m_collections))
    {
        if (item.deleted && !item.location.isNull())
        {
            assumeDeleted << item.location;
        }
    }

    CollectionManager::LocationCheckResult result;

    if (category == CategoryRemote)
    {
        result = CollectionManager::instance()->checkNetworkLocation(curl, assumeDeleted,
                                                                     &messageFromManager, &deviceIcon);
    }
    else
    {
        result = CollectionManager::instance()->checkLocation(curl, assumeDeleted,
                                                              &messageFromManager, &deviceIcon);
    }

    QString path = curl.toLocalFile();

    // If there are other added collections then CollectionManager does not know about them. Check here.

    for (const Item& item : std::as_const(m_collections))
    {
        if (!item.deleted && item.location.isNull())
        {
            if (!item.path.isEmpty() && path.startsWith(item.path))
            {
                if ((path == item.path) || path.startsWith(item.path + QLatin1Char('/')))
                {
                    messageFromManager = i18n("You have previously added a collection "
                                              "that contains the path \"%1\".", QDir::toNativeSeparators(path));
                    result             = CollectionManager::LocationNotAllowed;

                    break;
                }
            }
        }
    }

    // If check failed, display sorry message

    QString iconName;

    switch (result)
    {
        case CollectionManager::LocationAllRight:
        {
            iconName = QLatin1String("dialog-ok-apply");

            break;
        }

        case CollectionManager::LocationHasProblems:
        {
            iconName = QLatin1String("dialog-information");

            break;
        }

        case CollectionManager::LocationNotAllowed:
        case CollectionManager::LocationInvalidCheck:
        {
            QString warning;

            if (adding)
            {
                warning = i18nc("@title:window",
                                "Problem Adding Collection");
            }
            else
            {
                warning = i18nc("@title:window",
                                "Problem updating Collection");
            }

            QMessageBox::warning(m_dialogParentWidget, warning, messageFromManager);

            // fail

            return false;
        }
    }

    // Create a dialog that displays volume information and allows to change the name of the collection

    QDialog* const dialog     = new QDialog(m_dialogParentWidget);

    if (adding)
    {
        dialog->setWindowTitle(i18nc("@title:window", "Adding Collection"));
    }
    else
    {
        dialog->setWindowTitle(i18nc("@title:window", "Update Collection"));
    }

    QWidget* const mainWidget = new QWidget(dialog);
    QLabel* const nameLabel   = new QLabel;

    if (adding)
    {
        nameLabel->setText(i18n("Your new collection will be created with this name:"));
    }
    else
    {
        nameLabel->setText(i18n("Your collection will be updated to this name:"));
    }

    nameLabel->setWordWrap(true);

    // lineedit for collection name

    DTextEdit* const nameEdit = new DTextEdit;
    nameEdit->setLinesVisible(1);
    nameLabel->setBuddy(nameEdit);

    // label for the icon showing the type of storage (hard disk, CD, USB drive)

    QLabel* const deviceIconLabel = new QLabel;
    deviceIconLabel->setPixmap(QIcon::fromTheme(deviceIcon).pixmap(64));

    QGroupBox* const infoBox      = new QGroupBox;
/*
    infoBox->setTitle(i18n("More Information"));
*/
    // label either signalling everything is all right, or raising awareness to some problems
    // (like handling of CD identified by a label)

    QLabel* const iconLabel = new QLabel;
    iconLabel->setPixmap(QIcon::fromTheme(iconName).pixmap(48));
    QLabel* const infoLabel = new QLabel;
    infoLabel->setText(messageFromManager);
    infoLabel->setWordWrap(true);

    QHBoxLayout* const hbox1 = new QHBoxLayout;
    hbox1->addWidget(iconLabel);
    hbox1->addWidget(infoLabel);
    infoBox->setLayout(hbox1);

    QGridLayout* const grid1 = new QGridLayout;
    grid1->addWidget(deviceIconLabel, 0, 0, 3, 1);
    grid1->addWidget(nameLabel,       0, 1);
    grid1->addWidget(nameEdit,        1, 1);
    grid1->addWidget(infoBox,         2, 1);
    mainWidget->setLayout(grid1);

    QVBoxLayout* const vbx          = new QVBoxLayout(dialog);
    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Ok   |
                                                           QDialogButtonBox::Help |
                                                           QDialogButtonBox::Cancel,
                                                           dialog);
    vbx->addWidget(mainWidget);
    vbx->addWidget(buttons);
    dialog->setLayout(vbx);

    connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            dialog, SLOT(accept()));

    connect(buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            dialog, SLOT(reject()));

    connect(buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    // default to directory name as collection name

    QDir dir(path);
    nameEdit->setText(dir.dirName());

    if (dialog->exec() == QDialog::Accepted)
    {
        if (newPath && newLabel)
        {
            *newLabel = nameEdit->text();
            *newPath  = path;

            return true;
        }
    }

    return false;
}

bool SetupCollectionModel::askForNewCollectionCategory(int* const category)
{
    // Create a dialog that displays the category and allows to change the category of the collection

    QDialog* const dialog     = new QDialog(m_dialogParentWidget);
    dialog->setWindowTitle(i18nc("@title:window", "Select Category"));

    QWidget* const mainWidget = new QWidget(dialog);
    QLabel* const nameLabel   = new QLabel;
    nameLabel->setText(i18n("Your collection will use this category:"));
    nameLabel->setWordWrap(true);

    // combobox for collection category

    QComboBox* const categoryBox = new QComboBox;
    categoryBox->addItem(i18n("Local Collections"),              CategoryLocal);
    categoryBox->addItem(i18n("Collections on Removable Media"), CategoryRemovable);
    categoryBox->addItem(i18n("Collections on Network Shares"),  CategoryRemote);

    // label for the icon showing the refresh icon

    QLabel* const questionIconLabel = new QLabel;
    questionIconLabel->setPixmap(QIcon::fromTheme(QLatin1String("view-sort")).pixmap(64));

    QGridLayout* const grid1        = new QGridLayout;
    grid1->addWidget(questionIconLabel, 0, 0, 3, 1);
    grid1->addWidget(nameLabel,         0, 1);
    grid1->addWidget(categoryBox,       1, 1);
    mainWidget->setLayout(grid1);

    QVBoxLayout* const vbx          = new QVBoxLayout(dialog);
    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Ok   |
                                                           QDialogButtonBox::Help |
                                                           QDialogButtonBox::Cancel,
                                                           dialog);
    vbx->addWidget(mainWidget);
    vbx->addWidget(buttons);
    dialog->setLayout(vbx);

    connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            dialog, SLOT(accept()));

    connect(buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            dialog, SLOT(reject()));

    connect(buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    // default to current category

    if (category)
    {
        categoryBox->setCurrentIndex(categoryBox->findData(*category));
    }

    if (dialog->exec() == QDialog::Accepted)
    {
        if (category)
        {
            *category = categoryBox->currentData().toInt();
        }

        return true;
    }

    return false;
}

int SetupCollectionModel::categoryButtonMapId(const QModelIndex& _index) const
{
    if (!_index.isValid() || _index.parent().isValid())
    {
        return INTERNALID;
    }

    return _index.row();
}

int SetupCollectionModel::buttonMapId(const QModelIndex& _index) const
{
    if (!_index.isValid() || (_index.internalId() == INTERNALID))
    {
        return INTERNALID;
    }

    return _index.internalId();
}

void SetupCollectionModel::slotHelp()
{
    openOnlineDocumentation(QLatin1String("setup_application"),
                            QLatin1String("collections_settings"));
}

} // namespace Digikam

#include "moc_setupcollectionview.cpp"
