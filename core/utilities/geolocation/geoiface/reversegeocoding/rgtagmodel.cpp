/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : A model to hold information about image tags.
 *
 * SPDX-FileCopyrightText: 2010 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rgtagmodel.h"

// Qt includes

#include <QColor>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class Q_DECL_HIDDEN RGTagModel::Private
{
public:

    Private() = default;

public:

    QAbstractItemModel*          tagModel           = nullptr;
    TreeBranch*                  rootTag            = nullptr;

    QModelIndex                  parent;
    int                          startInsert        = -1;
    int                          endInsert          = -1;

    QList<QList<TagData> >       newTags;

    QStringList                  auxTagList;
    QList<Type>                  auxTagTypeList;
    QList<QPersistentModelIndex> auxIndexList;

    QList<QList<TagData> >       savedSpacerList;

    QString                      textState;
    QString                      textLau1;
    QString                      textLau2;
};

RGTagModel::RGTagModel(QAbstractItemModel* const externalTagModel, QObject* const parent)
    : QAbstractItemModel(parent),
      d                 (new Private)
{
    d->tagModel      = externalTagModel;
    d->rootTag       = new TreeBranch();
    d->rootTag->type = TypeChild;

                   i18n("{Country}");
    d->textState = i18nc("Part of a country", "{State}");
                   i18n("{State district}");
                   i18n("{County}");
                   i18n("{City}");
                   i18n("{City district}");
                   i18n("{Suburb}");
                   i18n("{Town}");
                   i18n("{Village}");
                   i18n("{Hamlet}");
                   i18n("{Street}");
                   i18n("{House number}");
                   i18n("{Place}");
    d->textLau1  = i18nc("Local administrative area 1", "{LAU1}");
    d->textLau2  = i18nc("Local administrative area 2", "{LAU2}");

    connect(d->tagModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotSourceDataChanged(QModelIndex,QModelIndex)));

    connect(d->tagModel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
            this, SLOT(slotSourceHeaderDataChanged(Qt::Orientation,int,int)));

    connect(d->tagModel, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(slotColumnsAboutToBeInserted(QModelIndex,int,int)));

    connect(d->tagModel, SIGNAL(columnsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(slotColumnsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));

    connect(d->tagModel, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(slotColumnsAboutToBeRemoved(QModelIndex,int,int)));

    connect(d->tagModel, SIGNAL(columnsInserted(QModelIndex,int,int)),
            this, SLOT(slotColumnsInserted()));

    connect(d->tagModel, SIGNAL(columnsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(slotColumnsMoved()));

    connect(d->tagModel, SIGNAL(columnsRemoved(QModelIndex,int,int)),
            this, SLOT(slotColumnsRemoved()));

    connect(d->tagModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(slotLayoutAboutToBeChanged()));

    connect(d->tagModel, SIGNAL(layoutChanged()),
            this, SLOT(slotLayoutChanged()));

    connect(d->tagModel, SIGNAL(modelAboutToBeReset()),
            this, SLOT(slotModelAboutToBeReset()));

    connect(d->tagModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(slotRowsAboutToBeInserted(QModelIndex,int,int)));

    connect(d->tagModel, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(slotRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));

    connect(d->tagModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(slotRowsAboutToBeRemoved(QModelIndex,int,int)));

    connect(d->tagModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotRowsInserted()));

    connect(d->tagModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(slotRowsMoved()));

    connect(d->tagModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(slotRowsRemoved()));
}

RGTagModel::~RGTagModel()
{
    delete d->rootTag;
    delete d;
}

TreeBranch* RGTagModel::branchFromIndex(const QModelIndex& index) const
{
    return (index.isValid() ? static_cast<TreeBranch*>(index.internalPointer()) : d->rootTag);
}

void checkTree(TreeBranch* const checkBranch, int level)
{
    if (!checkBranch->sourceIndex.isValid())
    {
        return;
    }

    for (int j = 0 ; j < checkBranch->oldChildren.count() ; ++j)
    {
        checkTree(checkBranch->oldChildren[j], level+1);
    }

    for (int j = 0 ; j < checkBranch->spacerChildren.count() ; ++j)
    {
        checkTree(checkBranch->spacerChildren[j], level+1);
    }
}

QModelIndex RGTagModel::fromSourceIndex(const QModelIndex& externalTagModelIndex) const
{
    if (!externalTagModelIndex.isValid())
    {
        return QModelIndex();
    }

    Q_ASSERT(externalTagModelIndex.model() == d->tagModel);

    QList<QModelIndex> parents;
    QModelIndex myIndex = externalTagModelIndex;
    parents << myIndex;

    while (myIndex.parent().isValid())
    {
        myIndex = myIndex.parent();
        parents.prepend(myIndex);
    }

    TreeBranch* subModelBranch = d->rootTag;
    int level                  = 0;

    while (level <= parents.count())
    {
        if (subModelBranch->sourceIndex == externalTagModelIndex)
        {
            return createIndex(subModelBranch->sourceIndex.row()              +
                               subModelBranch->parent->spacerChildren.count() +
                               subModelBranch->parent->newChildren.count(),
                               subModelBranch->sourceIndex.column(), subModelBranch);
        }

        int where = -1;

        for (int i = 0 ; i < subModelBranch->oldChildren.count() ; ++i)
        {
            if (
                (level < parents.size()) &&
                (subModelBranch->oldChildren[i]->sourceIndex == parents[level])
               )
            {
                where = i;
                break;
            }
        }

        if (where >= 0)
        {
            subModelBranch = subModelBranch->oldChildren[where];
        }
        else
        {
            if (level>=parents.count())
            {
                return QModelIndex();
            }

            // TODO: check when rows are different

            TreeBranch* const newTreeBranch = new TreeBranch();
            newTreeBranch->sourceIndex      = parents[level];
            newTreeBranch->data             = d->tagModel->data(externalTagModelIndex, Qt::DisplayRole).toString();
            newTreeBranch->parent           = subModelBranch;
            newTreeBranch->type             = TypeChild;

            subModelBranch->oldChildren.append(newTreeBranch);
            subModelBranch                  = newTreeBranch;
        }

        level++;
    }

    // no index is found

    return QModelIndex();
}

QModelIndex RGTagModel::toSourceIndex(const QModelIndex& tagModelIndex) const
{
    if (!tagModelIndex.isValid())
    {
        return QModelIndex();
    }

    Q_ASSERT(tagModelIndex.model() == this);
/*
    static_cast<TreeBranch*>(tagModelIndex.internalPointer());
*/
    const TreeBranch* const treeBranch = branchFromIndex(tagModelIndex);

    if (!treeBranch)
    {
        return QModelIndex();
    }

    return treeBranch->sourceIndex;
}

void RGTagModel::addSpacerTag(const QModelIndex& parent, const QString& spacerName)
{
/*
    TreeBranch* const parentBranch = parent.isValid() ? static_cast<TreeBranch*>(parent.internalPointer()) : d->rootTag;
*/
    TreeBranch* const parentBranch = branchFromIndex(parent);
    bool found                     = false;

    if (!parentBranch->spacerChildren.isEmpty())
    {
        for (int i = 0 ; i < parentBranch->spacerChildren.count() ; ++i)
        {
            if (parentBranch->spacerChildren[i]->data == spacerName)
            {
                found = true;
                break;
            }
        }
    }

    if (!found)
    {
        TreeBranch* const newSpacer = new TreeBranch();
        newSpacer->parent           = parentBranch;
        newSpacer->data             = spacerName;
        newSpacer->type             = TypeSpacer;

        beginInsertRows(parent, parentBranch->spacerChildren.count(), parentBranch->spacerChildren.count());
        parentBranch->spacerChildren.append(newSpacer);
        endInsertRows();
    }
}

QPersistentModelIndex RGTagModel::addNewTag(const QModelIndex& parent, const QString& newTagName, const QString& newElement)
{
    TreeBranch* const parentBranch = branchFromIndex(parent);
/*
    parent.isValid() ? static_cast<TreeBranch*>(parent.internalPointer()) : d->rootTag;
*/
    bool found                     = false;
    QPersistentModelIndex retIndex;

    if (!parentBranch->newChildren.isEmpty())
    {
        for ( int i = 0 ; i < parentBranch->newChildren.count() ; ++i)
        {
            if (parentBranch->newChildren[i]->data == newTagName)
            {
                found    = true;
                retIndex = createIndex(parentBranch->spacerChildren.count() + i, 0, parentBranch->newChildren[i]);
                break;
            }
        }
    }

    if (!found)
    {
        TreeBranch* const newTagChild = new TreeBranch();
        newTagChild->parent           = parentBranch;
        newTagChild->data             = newTagName;
        newTagChild->help             = newElement;
        newTagChild->type             = TypeNewChild;

        beginInsertRows(parent, parentBranch->spacerChildren.count() + parentBranch->newChildren.count(),
                        parentBranch->spacerChildren.count() + parentBranch->newChildren.count());
        parentBranch->newChildren.append(newTagChild);
        endInsertRows();

        retIndex                      = createIndex(parentBranch->spacerChildren.count() + parentBranch->newChildren.count() - 1, 0,
                                                    parentBranch->newChildren.last());
    }

    return retIndex;
}

QList<TagData> RGTagModel::getTagAddress()
{
    QList<TagData> tagAddress;

    for (int i = 0 ; i < d->auxTagList.count() ; ++i)
    {
        TagData tagData;
        tagData.tagName = d->auxTagList[i];
        tagData.tagType = d->auxTagTypeList[i];
        tagAddress.append(tagData);
    }

    return tagAddress;
}

void RGTagModel::addDataInTree(TreeBranch* currentBranch,
                               int currentRow,
                               const QStringList& addressElements,
                               const QStringList& elementsData)
{
    for (int i = 0 ; i < currentBranch->spacerChildren.count() ; ++i)
    {
        bool newDataAdded = false;

        // this spacer is not an address element

        if (currentBranch->spacerChildren[i]->data.indexOf(QLatin1String("{")) != 0)
        {
            d->auxTagList.append(currentBranch->spacerChildren[i]->data);
            d->auxTagTypeList.append(TypeSpacer);
            addDataInTree(currentBranch->spacerChildren[i], i, addressElements, elementsData);
            d->auxTagList.removeLast();
            d->auxTagTypeList.removeLast();
        }
        else
        {
            for (int j = 0 ; j < addressElements.count() ; ++j)
            {
                if (currentBranch->spacerChildren[i]->data == addressElements[j])
                {
                    newDataAdded = true;
                    QModelIndex currentIndex;

                    if (currentBranch == d->rootTag)
                    {
                        currentIndex = QModelIndex();
                    }
                    else
                    {
                        currentIndex = createIndex(currentRow, 0, currentBranch);
                    }

                    // checks if adds the new tag as a sibling to a spacer, or as a child of a new tag

                    QPersistentModelIndex auxIndex;

                    if (
                        (
                         (currentBranch->type != TypeSpacer) ||
                         (currentBranch->data.indexOf(QLatin1String("{")) != 0)
                        ) ||
                        (d->auxIndexList.isEmpty()))
                    {
                        auxIndex = addNewTag(currentIndex, elementsData[j], addressElements[j]);
                    }
                    else
                    {
                        auxIndex = addNewTag(d->auxIndexList.last(), elementsData[j], addressElements[j]);
                    }

                    d->auxTagList.append(elementsData[j]);
                    d->auxTagTypeList.append(TypeNewChild);
                    d->auxIndexList.append(auxIndex);

                    QList<TagData> newTag = getTagAddress();
                    newTag.last().tipName = addressElements[j];
                    d->newTags.append(newTag);
                }
            }

            if (currentBranch->spacerChildren.at(i))
            {
                addDataInTree(currentBranch->spacerChildren[i], i, addressElements, elementsData);
            }

            if (newDataAdded)
            {
                d->auxTagList.removeLast();
                d->auxTagTypeList.removeLast();
                d->auxIndexList.removeLast();
            }
        }
    }

    for (int i = 0 ; i < currentBranch->newChildren.count() ; ++i)
    {
        d->auxTagList.append(currentBranch->newChildren[i]->data);
        d->auxTagTypeList.append(TypeNewChild);
        addDataInTree(currentBranch->newChildren.at(i), i + currentBranch->spacerChildren.count(), addressElements, elementsData);
        d->auxTagList.removeLast();
        d->auxTagTypeList.removeLast();
    }

    for (int i = 0 ; i < currentBranch->oldChildren.count() ; ++i)
    {
        d->auxTagList.append(currentBranch->oldChildren.at(i)->data);
        d->auxTagTypeList.append(TypeChild);
        addDataInTree(currentBranch->oldChildren.at(i),
                      i + currentBranch->spacerChildren.count() + currentBranch->newChildren.count(),
                      addressElements,
                      elementsData);
        d->auxTagList.removeLast();
        d->auxTagTypeList.removeLast();
    }
}

QList<QList<TagData> > RGTagModel::addNewData(const QStringList& elements, const QStringList& resultedData)
{
    d->newTags.clear();

    // elements contains address elements {Country}, {City}, ...
    // resultedData contains RG data (example Spain,Barcelona)

    addDataInTree(d->rootTag, 0, elements, resultedData);

    return d->newTags;
}

int RGTagModel::columnCount(const QModelIndex& parent) const
{
    const TreeBranch* const parentBranch = branchFromIndex(parent); //static_cast<TreeBranch*>(parent.internalPointer());

    if (!parentBranch)
    {
        return 1;
    }

    if      (parentBranch->type == TypeSpacer)
    {
        return 1;
    }
    else if (parentBranch->type == TypeNewChild)
    {
        return 1;
    }

    return d->tagModel->columnCount(toSourceIndex(parent));
}

bool RGTagModel::setData(const QModelIndex& /*index*/, const QVariant& /*value*/, int /*role*/)
{
    return false;
}

QVariant RGTagModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

/*
    static_cast<TreeBranch*>(index.internalPointer());
*/
    const TreeBranch* const treeBranch = branchFromIndex(index);

    if      ((!treeBranch) || (treeBranch->type == TypeChild))
    {
        return d->tagModel->data(toSourceIndex(index), role);
    }
    else if ((treeBranch->type == TypeSpacer) && (role == Qt::DisplayRole))
    {
        return translateSpacer(treeBranch->data);
    }
    else if ((treeBranch->type == TypeSpacer) && (role == Qt::ForegroundRole))
    {
        return QColor(Qt::red);
    }
    else if ((treeBranch->type == TypeNewChild) && (role == Qt::DisplayRole))
    {
        return treeBranch->data;
    }
    else if ((treeBranch->type == TypeNewChild) && (role == Qt::ForegroundRole))
    {
        return QColor(Qt::green);
    }
    else if ((treeBranch->type == TypeNewChild) && (role == Qt::ToolTipRole))
    {
        return translateSpacer(treeBranch->help);
    }

    return QVariant();
}

QModelIndex RGTagModel::index(int row, int column, const QModelIndex& parent) const
{
    if ((column != 0) || (row < 0))
    {
        return QModelIndex();
    }
/*
    if (parent.isValid())
    {
        parentBranch = static_cast<TreeBranch*>(parent.internalPointer());
    }
*/
    TreeBranch* const parentBranch = branchFromIndex(parent);//d->rootTag;

    // this should not happen!

    if (!parentBranch)
    {
        return QModelIndex();
    }

    if      (row < parentBranch->spacerChildren.count())
    {
        return createIndex(row, column, parentBranch->spacerChildren[row]);
    }
    else if (
             // cppcheck-suppress knownConditionTrueFalse
             (row >= parentBranch->spacerChildren.count()) &&
             (row < (parentBranch->newChildren.count() + parentBranch->spacerChildren.count()))
            )
    {
        return createIndex(row, column, parentBranch->newChildren[row-parentBranch->spacerChildren.count()]);
    }

    return fromSourceIndex(d->tagModel->index(row - parentBranch->spacerChildren.count() - parentBranch->newChildren.count(),
                                              column, toSourceIndex(parent)));
}

QModelIndex RGTagModel::parent(const QModelIndex& index) const
{
    TreeBranch* const currentBranch = branchFromIndex(index);  // static_cast<TreeBranch*>(index.internalPointer());

    if (!currentBranch)
    {
        return QModelIndex();
    }

    if ((currentBranch->type == TypeSpacer) || (currentBranch->type == TypeNewChild))
    {
        TreeBranch* const parentBranch = currentBranch->parent;

        if (!parentBranch)
        {
            return QModelIndex();
        }

        TreeBranch* const gParentBranch = parentBranch->parent;

        if (!gParentBranch)
        {
            return QModelIndex();
        }

        if      (parentBranch->type == TypeSpacer)
        {
            for (int parentRow = 0 ; parentRow < gParentBranch->spacerChildren.count() ; ++parentRow)
            {
                if (gParentBranch->spacerChildren[parentRow] == parentBranch)
                {
                    return createIndex(parentRow, 0, parentBranch);
                }
            }

            return QModelIndex();
        }
        else if (parentBranch->type == TypeNewChild)
        {
            for (int parentRow = 0 ; parentRow < gParentBranch->newChildren.count() ; ++parentRow)
            {
                if (gParentBranch->newChildren[parentRow] == parentBranch)
                {
                    return createIndex(parentRow+gParentBranch->spacerChildren.count(), 0, parentBranch);
                }
            }
        }
        else if (parentBranch->type == TypeChild)
        {
            // TODO: don't we have a function for this?

            for (int parentRow = 0 ; parentRow < gParentBranch->oldChildren.count() ; ++parentRow)
            {
                if (gParentBranch->oldChildren[parentRow] == parentBranch)
                {
                    return createIndex(parentRow + gParentBranch->spacerChildren.count() + gParentBranch->newChildren.count(),
                                       0, parentBranch);
                }
            }

            return QModelIndex();
        }
    }

    return fromSourceIndex(d->tagModel->parent(toSourceIndex(index)));
}

int RGTagModel::rowCount(const QModelIndex& parent) const
{
/*
    parent.isValid() ? static_cast<TreeBranch*>(parent.internalPointer()) : d->rootTag;
*/
    TreeBranch* const parentBranch = branchFromIndex(parent);
    int myRowCount                 = parentBranch->spacerChildren.count() + parentBranch->newChildren.count();

    // TODO: we don't know whether the oldChildren have been set up, therefore query the source model

    if (parentBranch->type == TypeChild)
    {
        const QModelIndex sourceIndex =  toSourceIndex(parent);
        myRowCount                   += d->tagModel->rowCount(sourceIndex);
    }

    return myRowCount;
}

bool RGTagModel::setHeaderData(int /*section*/,
                               Qt::Orientation /*orientation*/,
                               const QVariant& /*value*/,
                               int /*role*/)
{
    return false;
}

QVariant RGTagModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return d->tagModel->headerData(section, orientation, role);
}

Qt::ItemFlags RGTagModel::flags(const QModelIndex& index) const
{
    const TreeBranch* const currentBranch = branchFromIndex(index);

    if (currentBranch && ((currentBranch->type == TypeSpacer) || (currentBranch->type == TypeNewChild)))
    {
        return QAbstractItemModel::flags(index);
    }

    return d->tagModel->flags(toSourceIndex(index));
}

void RGTagModel::slotSourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    Q_EMIT dataChanged(fromSourceIndex(topLeft),fromSourceIndex(bottomRight));
}

void RGTagModel::slotSourceHeaderDataChanged(const Qt::Orientation orientation, int first, int last)
{
    Q_EMIT headerDataChanged(orientation, first, last);
}

void RGTagModel::slotColumnsAboutToBeInserted(const QModelIndex& parent, int start, int end)
{
    // TODO: Should we do something here?

    beginInsertColumns(fromSourceIndex(parent), start, end);
}

void RGTagModel::slotColumnsAboutToBeMoved(const QModelIndex& sourceParent,
                                           int sourceStart,
                                           int sourceEnd,
                                           const QModelIndex& destinationParent,
                                           int destinationColumn)
{
    beginMoveColumns(fromSourceIndex(sourceParent), sourceStart, sourceEnd, fromSourceIndex(destinationParent), destinationColumn);
}

void RGTagModel::slotColumnsAboutToBeRemoved(const QModelIndex& parent, int start, int end )
{
    beginRemoveColumns(fromSourceIndex(parent), start, end);
}

void RGTagModel::slotColumnsInserted()
{
    endInsertColumns();
}

void RGTagModel::slotColumnsMoved()
{
    endMoveColumns();
}

void RGTagModel::slotColumnsRemoved()
{
    endRemoveColumns();
}

void RGTagModel::slotLayoutAboutToBeChanged()
{
    Q_EMIT layoutAboutToBeChanged();
}

void RGTagModel::slotLayoutChanged()
{
    Q_EMIT layoutChanged();
}

void RGTagModel::slotModelAboutToBeReset()
{
    beginResetModel();
}

void RGTagModel::slotModelReset()
{
    beginResetModel();
    endResetModel();
}

void RGTagModel::slotRowsAboutToBeInserted(const QModelIndex& parent, int start, int end)
{
    TreeBranch* const parentBranch = parent.isValid() ? static_cast<TreeBranch*>(fromSourceIndex(parent).internalPointer()) : d->rootTag;
    d->parent                      = fromSourceIndex(parent);
    d->startInsert                 = start;
    d->endInsert                   = end;

    beginInsertRows(d->parent,
                    start + parentBranch->newChildren.count() + parentBranch->spacerChildren.count(),
                    end   + parentBranch->newChildren.count() + parentBranch->spacerChildren.count());
}

void RGTagModel::slotRowsAboutToBeMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd,
                                        const QModelIndex& destinationParent, int destinationRow)
{
    beginMoveRows(fromSourceIndex(sourceParent), sourceStart, sourceEnd, fromSourceIndex(destinationParent), destinationRow );
}

void RGTagModel::slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
}

void RGTagModel::slotRowsInserted()
{
   TreeBranch* const parentBranch = d->parent.isValid() ? static_cast<TreeBranch*>(d->parent.internalPointer()) : d->rootTag;

    for (int i = d->startInsert ; i < d->endInsert ; ++i)
    {
        TreeBranch* const newBranch = new TreeBranch();
        newBranch->parent           = parentBranch;
        newBranch->sourceIndex      = d->tagModel->index(i, 0, d->parent);
        newBranch->type             = TypeChild;

        parentBranch->oldChildren.insert(i, newBranch);
    }

    endInsertRows();

    d->parent      = QModelIndex();
    d->startInsert = -1;
    d->endInsert   = -1;
}

void RGTagModel::slotRowsMoved()
{
    endMoveRows();
}

void RGTagModel::slotRowsRemoved()
{
}

void RGTagModel::deleteTag(const QModelIndex& currentIndex)
{
    if (!currentIndex.isValid())
    {
        return;
    }

    QModelIndex parentIndex              = currentIndex.parent();
    int currentRow                       = currentIndex.row();
    TreeBranch* const parentBranch       =  branchFromIndex(parentIndex);
    TreeBranch* const currentChildBranch = branchFromIndex(currentIndex);

    if (currentChildBranch->type == TypeChild)
    {
        return;
    }

    if (
        (currentChildBranch->spacerChildren.count() > 0) ||
        (currentChildBranch->newChildren.count()    > 0)
       )
    {
        beginMoveRows(currentIndex, 0, currentChildBranch->spacerChildren.count() - 1,
                      parentIndex, parentBranch->spacerChildren.count());

        for (int j = 0 ; j < currentChildBranch->spacerChildren.count() ; ++j)
        {
            parentBranch->spacerChildren.append(currentChildBranch->spacerChildren[j]);
            parentBranch->spacerChildren.last()->parent = parentBranch;
/*
            QModelIndex testIndex =
*/
            createIndex(parentBranch->spacerChildren.count() - 1, 0, parentBranch->spacerChildren.last());
        }

        currentChildBranch->spacerChildren.clear();
        endMoveRows();

        beginMoveRows(currentIndex, currentChildBranch->spacerChildren.count(),
                      currentChildBranch->spacerChildren.count() + currentChildBranch->newChildren.count() - 1,
                      parentIndex, parentBranch->spacerChildren.count() + parentBranch->newChildren.count());

        for (int j = currentChildBranch->spacerChildren.count() ;
             (j < (currentChildBranch->spacerChildren.count() + currentChildBranch->newChildren.count())) ; ++j)
        {
            parentBranch->newChildren.append(currentChildBranch->newChildren[j - currentChildBranch->spacerChildren.count()]);
            parentBranch->newChildren.last()->parent = parentBranch;
        }

        currentChildBranch->newChildren.clear();
        endMoveRows();
    }

    beginRemoveRows(parentIndex, currentRow, currentRow);

    // TODO: is it good here?

    if (currentRow < parentBranch->spacerChildren.count())
    {
        parentBranch->spacerChildren.removeAt(currentRow);
    }
    else
    {
        parentBranch->newChildren.removeAt(currentRow - parentBranch->spacerChildren.count());
    }

    endRemoveRows();
}

void RGTagModel::findAndDeleteSpacersOrNewTags(TreeBranch* currentBranch, int currentRow, Type whatShouldRemove)
{
/*
    QModelIndex currentIndex =
*/
    createIndex(currentRow, 0, currentBranch);

    for (int i = 0 ; i < currentBranch->spacerChildren.count() ; ++i)
    {
        findAndDeleteSpacersOrNewTags(currentBranch->spacerChildren[i], i, whatShouldRemove);

        if (whatShouldRemove == TypeSpacer)
        {
            QModelIndex spacerIndex = createIndex(i, 0, currentBranch->spacerChildren[i]);
            deleteTag(spacerIndex);
            i--;
        }
    }

    for (int i = 0 ; i < currentBranch->newChildren.count() ; ++i)
    {
        findAndDeleteSpacersOrNewTags(currentBranch->newChildren[i], i + currentBranch->spacerChildren.count(), whatShouldRemove);

        if (whatShouldRemove == TypeNewChild)
        {
            QModelIndex newTagIndex = createIndex(i+currentBranch->spacerChildren.count(), 0, currentBranch->newChildren[i]);
            deleteTag(newTagIndex);
            i--;
        }
    }

    for (int i = 0 ; i < currentBranch->oldChildren.count() ; ++i)
    {
        findAndDeleteSpacersOrNewTags(currentBranch->oldChildren[i],
                                      i + currentBranch->spacerChildren.count() + currentBranch->newChildren.count(),
                                      whatShouldRemove);
    }
}

void RGTagModel::deleteAllSpacersOrNewTags(const QModelIndex& currentIndex, Type whatShouldRemove)
{
    if      (whatShouldRemove == TypeSpacer)
    {
/*
        currentIndex.isValid() ? static_cast<TreeBranch*>(currentIndex.internalPointer()) : d->rootTag;
*/
        TreeBranch* const currentBranch = branchFromIndex(currentIndex);
        findAndDeleteSpacersOrNewTags(currentBranch, 0, whatShouldRemove);
    }
    else if (whatShouldRemove == TypeNewChild)
    {
        findAndDeleteSpacersOrNewTags(d->rootTag, 0, whatShouldRemove);
    }
}

void RGTagModel::readdTag(TreeBranch*& currentBranch,
                          int currentRow,
                          const QList<TagData>& tagAddressElements,
                          int currentAddressElementIndex)
{
    bool found1 = false;
    int  foundIndex;

    if (currentAddressElementIndex >= tagAddressElements.count())
    {
        return;
    }

    if (tagAddressElements[currentAddressElementIndex].tipName == QLatin1String("{Country code}"))
    {
        return;
    }

    if (tagAddressElements[currentAddressElementIndex].tagType == TypeSpacer)
    {
        for (int i = 0 ; i < currentBranch->spacerChildren.count() ; ++i)
        {
            if (currentBranch->spacerChildren[i]->data == tagAddressElements[currentAddressElementIndex].tagName)
            {
                found1     = true;
                foundIndex = i;
                break;
            }
        }

        if (found1)
        {
            readdTag(currentBranch->spacerChildren[foundIndex], foundIndex, tagAddressElements, currentAddressElementIndex + 1);
            return;
        }
        else
        {
            // recreates the spacer

            QModelIndex currentIndex;

            if (currentBranch == d->rootTag)
            {
                currentIndex = QModelIndex();
            }
            else
            {
                currentIndex = createIndex(currentRow, 0, currentBranch);
            }

            addSpacerTag(currentIndex, tagAddressElements[currentAddressElementIndex].tagName);

            if ((tagAddressElements.count() - 1) > currentAddressElementIndex)
            {
                readdTag(currentBranch->spacerChildren[currentBranch->spacerChildren.count() - 1],
                         currentBranch->spacerChildren.count() - 1,
                         tagAddressElements,
                         currentAddressElementIndex + 1);
            }
        }
    }
    else if (tagAddressElements[currentAddressElementIndex].tagType == TypeNewChild)
    {
        for (int i = 0 ; i < currentBranch->newChildren.count() ; ++i)
        {
            if (currentBranch->newChildren[i]->data == tagAddressElements[currentAddressElementIndex].tagName)
            {
                found1     = true;
                foundIndex = i;
                break;
            }
        }

        if (found1)
        {
            readdTag(currentBranch->newChildren[foundIndex],
                     foundIndex + currentBranch->spacerChildren.count(),
                     tagAddressElements,
                     currentAddressElementIndex + 1);
            return;
        }
        else
        {
            QModelIndex currentIndex;

            if (currentBranch == d->rootTag)
            {
                currentIndex = QModelIndex();
            }
            else
            {
                currentIndex = createIndex(currentRow, 0, currentBranch);
            }

            addNewTag(currentIndex, tagAddressElements[currentAddressElementIndex].tagName,
                                    tagAddressElements[currentAddressElementIndex].tipName);

            if ((tagAddressElements.count() - 1) > currentAddressElementIndex)
            {
                readdTag(currentBranch->newChildren[currentBranch->newChildren.count() - 1],
                         currentBranch->spacerChildren.count() + currentBranch->newChildren.count() - 1,
                         tagAddressElements,
                         currentAddressElementIndex + 1);
            }
        }
    }
    else if (tagAddressElements[currentAddressElementIndex].tagType == TypeChild)
    {
        bool found2 = false;

        for (int i = 0 ; i < currentBranch->oldChildren.count() ; ++i)
        {
            if (currentBranch->oldChildren[i]->data == tagAddressElements[currentAddressElementIndex].tagName)
            {
                found2     = true;
                foundIndex = i;
                break;
            }
        }

        if (found2)
        {
            readdTag(currentBranch->oldChildren[foundIndex],
                     foundIndex + currentBranch->spacerChildren.count() + currentBranch->newChildren.count(),
                     tagAddressElements, currentAddressElementIndex + 1);
            return;
        }
        else
        {
            QModelIndex currentIndex;

            if (currentBranch == d->rootTag)
            {
                currentIndex = QModelIndex();
            }
            else
            {
                currentIndex = createIndex(currentRow, 0, currentBranch);
            }

            addSpacerTag(currentIndex, tagAddressElements[currentAddressElementIndex].tagName);

            if ((tagAddressElements.count() - 1) > currentAddressElementIndex)
            {
                readdTag(currentBranch->spacerChildren[currentBranch->spacerChildren.count() - 1],
                         currentBranch->spacerChildren.count() - 1,
                         tagAddressElements,
                         currentAddressElementIndex + 1);
            }
        }
    }
}

void RGTagModel::readdNewTags(const QList<QList<TagData> >& tagAddressList)
{
    for (int i = 0 ; i < tagAddressList.count() ; ++i)
    {
        QList<TagData> currentAddressTag = tagAddressList[i];
        readdTag(d->rootTag, 0, currentAddressTag, 0);
    }
}

QList<TagData> RGTagModel::getSpacerAddress(TreeBranch* currentBranch)
{
    QList<TagData> spacerAddress;

    while (currentBranch->parent != nullptr)
    {
        TagData currentTag;
        currentTag.tagName = currentBranch->data;
        currentTag.tagType = currentBranch->type;

        spacerAddress.prepend(currentTag);
        currentBranch      = currentBranch->parent;
    }

    return spacerAddress;
}

void RGTagModel::climbTreeAndGetSpacers(const TreeBranch* currentBranch)
{
    for (int i = 0 ; i < currentBranch->spacerChildren.count() ; ++i)
    {
        QList<TagData> currentSpacerAddress;
        currentSpacerAddress = getSpacerAddress(currentBranch->spacerChildren[i]);
        d->savedSpacerList.append(currentSpacerAddress);
        climbTreeAndGetSpacers(currentBranch->spacerChildren[i]);
    }

    for (int i = 0 ; i < currentBranch->newChildren.count() ; ++i)
    {
        climbTreeAndGetSpacers(currentBranch->newChildren[i]);
    }

    for (int i = 0 ; i < currentBranch->oldChildren.count() ; ++i)
    {
        climbTreeAndGetSpacers(currentBranch->oldChildren[i]);
    }
}

QList<QList<TagData> > RGTagModel::getSpacers()
{
    d->savedSpacerList.clear();
    climbTreeAndGetSpacers(d->rootTag);

    return d->savedSpacerList;
}

void RGTagModel::addExternalTags(TreeBranch* parentBranch, int currentRow)
{
    QModelIndex parentIndex = createIndex(currentRow, 0, parentBranch);
    const int howManyRows   = rowCount(parentIndex);

    for (int i = 0 ; i < howManyRows ; ++i)
    {
        QModelIndex currentIndex        = index(i,0,parentIndex);
        TreeBranch* const currentBranch = branchFromIndex(currentIndex);  //currentIndex.isValid() ? static_cast<TreeBranch*>(currentIndex.internalPointer()) : d->rootTag;

        if (currentBranch == d->rootTag)
        {
            return;
        }

        addExternalTags(currentBranch,i);
    }
}

void RGTagModel::addAllExternalTagsToTreeView()
{
    addExternalTags(d->rootTag, 0);
}

void RGTagModel::addAllSpacersToTag(const QModelIndex& currentIndex, const QStringList& spacerList, int spacerListIndex)
{
    if (spacerListIndex >= spacerList.count())
    {
        return;
    }
/*
    currentIndex.isValid() ? static_cast<TreeBranch*>(currentIndex.internalPointer()) : d->rootTag;
*/
    TreeBranch* const currentBranch = branchFromIndex(currentIndex);

    for (int i = 0 ; i < currentBranch->spacerChildren.count() ; ++i)
    {
        if (currentBranch->data == spacerList[spacerListIndex])
        {
            QModelIndex foundIndex = createIndex(i, 0, currentBranch->spacerChildren[i]);
            addAllSpacersToTag(foundIndex, spacerList, spacerListIndex + 1);

            return;
        }
    }

    addSpacerTag(currentIndex, spacerList[spacerListIndex]);
    QModelIndex newIndex = createIndex(currentBranch->spacerChildren.count() - 1, 0,
                                       currentBranch->spacerChildren[currentBranch->spacerChildren.count() - 1]);
    addAllSpacersToTag(newIndex, spacerList, spacerListIndex + 1);
}

Type RGTagModel::getTagType(const QModelIndex& index) const
{
/*
    index.isValid() ? static_cast<TreeBranch*>(index.internalPointer()) : d->rootTag;
*/
    const TreeBranch* const treeBranch = branchFromIndex(index);

    return treeBranch->type;
}

QString RGTagModel::translateSpacer(const QString& text) const
{
    if      (text == QLatin1String("{State}"))
    {
        return d->textState;
    }
    else if (text == QLatin1String("{LAU1}"))
    {
        return d->textLau1;
    }
    else if (text == QLatin1String("{LAU2}"))
    {
        return d->textLau2;
    }

    return i18n(text.toUtf8().constData());
}

} // namespace Digikam

#include "moc_rgtagmodel.cpp"
