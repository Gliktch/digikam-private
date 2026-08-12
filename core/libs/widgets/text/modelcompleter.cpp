/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-13
 * Description : A QCompleter for AbstractAlbumModels
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "modelcompleter.h"

// Qt includes

#include <QTimer>
#include <QPointer>
#include <QStringListModel>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN ModelCompleter::Private
{
public:

    Private() = default;

public:

    int                          displayRole        = Qt::DisplayRole;
    int                          uniqueIdRole       = Qt::DisplayRole;

    QTimer*                      delayedModelTimer  = nullptr;
    QStringListModel*            stringModel        = nullptr;
    QPointer<QAbstractItemModel> model;

    /**
     * This map maps model indexes to their current text representation in the
     * completion object. This is needed because if data changes in one index,
     * the old text value is not known anymore, so that it cannot be removed
     * from the completion object.
     * TODO: if we want to use models that return unique strings but not integer, add support.
     */
    QHash<int, QString>          idToTextHash;
};

ModelCompleter::ModelCompleter(QObject* const parent)
    : QCompleter(parent),
      d         (new Private)
{
    d->stringModel = new QStringListModel(this);
    setModel(d->stringModel);

    setModelSorting(CaseSensitivelySortedModel);
    setCaseSensitivity(Qt::CaseInsensitive);
    setCompletionMode(PopupCompletion);
    setCompletionRole(Qt::DisplayRole);
    setFilterMode(Qt::MatchContains);
    setMaxVisibleItems(10);
    setCompletionColumn(0);

    d->delayedModelTimer = new QTimer(this);
    d->delayedModelTimer->setInterval(1000);
    d->delayedModelTimer->setSingleShot(true);

    connect(d->delayedModelTimer, SIGNAL(timeout()),
            this, SLOT(slotDelayedModelTimer()));

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SIGNAL(signalActivated()));

    connect(this, SIGNAL(highlighted(QModelIndex)),
            this, SLOT(slotHighlighted(QModelIndex)));
}

ModelCompleter::~ModelCompleter()
{
    delete d;
}

void ModelCompleter::setItemModel(QAbstractItemModel* const model, int uniqueIdRole, int displayRole)
{
    // First release old model.

    if (d->model)
    {
        disconnect(d->model);
        d->idToTextHash.clear();
        d->stringModel->setStringList(QStringList());
    }

    d->model        = model;
    d->displayRole  = displayRole;
    d->uniqueIdRole = uniqueIdRole;

    // Connect to the new model.

    if (d->model)
    {
        connect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(slotRowsInserted(QModelIndex,int,int)));

        connect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(slotRowsAboutToBeRemoved(QModelIndex,int,int)));

        connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(slotDataChanged(QModelIndex,QModelIndex)));

        connect(d->model, SIGNAL(modelReset()),
                this, SLOT(slotModelReset()));

        // Do an initial sync with the new model.

        sync(d->model);
    }
}

QAbstractItemModel* ModelCompleter::itemModel() const
{
    return d->model;
}

void ModelCompleter::addItem(const QString& item)
{
    QStringList list = d->stringModel->stringList();
    setList(list << item);
}

void ModelCompleter::setList(const QStringList& list)
{
    d->stringModel->setStringList(list);
    d->stringModel->sort(0);
}

QStringList ModelCompleter::items() const
{
    return d->stringModel->stringList();
}

void ModelCompleter::slotDelayedModelTimer()
{
    QStringList list = d->idToTextHash.values();
    list.removeDuplicates();
    setList(list);
}

void ModelCompleter::slotRowsInserted(const QModelIndex& parent, int start, int end)
{
    for (int i = start ; i <= end ; ++i)
    {
        // This cannot work if this is called from rowsAboutToBeInserted
        // because then the model doesn't know the index yet. So never do this
        // ;)

        const QModelIndex child = d->model->index(i, 0, parent);

        if (child.isValid())
        {
            sync(d->model, child);
        }
        else
        {
            qCDebug(DIGIKAM_WIDGETS_LOG) << "inserted rows are not valid for parent" << parent
                                         << parent.data(d->displayRole).toString()
                                         << "and child" << child;
        }
    }

    d->delayedModelTimer->start();
}

void ModelCompleter::slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    for (int i = start ; i <= end ; ++i)
    {
        QModelIndex index = d->model->index(i, 0, parent);

        if (!index.isValid())
        {
            qCDebug(DIGIKAM_WIDGETS_LOG) << "Received an invalid index to be removed";
            continue;
        }

        int id = index.data(d->uniqueIdRole).toInt();

        if (d->idToTextHash.contains(id))
        {
            QString itemName = d->idToTextHash.value(id);
            d->idToTextHash.remove(id);

            // Only delete an item in the completion object if there is no other
            // item with the same display name.

            const auto keys = d->idToTextHash.keys(itemName);

            if (keys.isEmpty())
            {
                d->delayedModelTimer->start();
            }
        }
        else
        {
/*
            qCDebug(DIGIKAM_WIDGETS_LOG) << "idToTextHash seems to be out of sync with the model."
                                         << "There is no entry for model index" << index;
*/
        }
    }
}

void ModelCompleter::slotModelReset()
{
    sync(d->model);
}

void ModelCompleter::slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    for (int row = topLeft.row() ; row <= bottomRight.row() ; ++row)
    {
        if (!d->model->hasIndex(row, topLeft.column(), topLeft.parent()))
        {
            qCDebug(DIGIKAM_WIDGETS_LOG) << "Got wrong change event for index with row" << row
                                         << ", column"   << topLeft.column()
                                         << "and parent" << topLeft.parent()
                                         << "in model"   << d->model
                                         << ". Ignoring it.";
            continue;
        }

        QModelIndex index  = d->model->index(row, topLeft.column(), topLeft.parent());

        if (!index.isValid())
        {
            qCDebug(DIGIKAM_WIDGETS_LOG) << "illegal index in changed data";
            continue;
        }

        int id              = index.data(d->uniqueIdRole).toInt();
        QString itemName    = index.data(d->displayRole).toString();
        d->idToTextHash[id] = itemName;

        d->delayedModelTimer->start();
    }
}

void ModelCompleter::sync(QAbstractItemModel* const model)
{
    d->idToTextHash.clear();

    for (int i = 0 ; i < model->rowCount() ; ++i)
    {
        const QModelIndex index = model->index(i, 0);
        sync(model, index);
    }

    d->delayedModelTimer->start();
}

void ModelCompleter::sync(QAbstractItemModel* const model, const QModelIndex& index)
{
    QString itemName = index.data(d->displayRole).toString();
    d->idToTextHash.insert(index.data(d->uniqueIdRole).toInt(), itemName);

    for (int i = 0 ; i < model->rowCount(index) ; ++i)
    {
        const QModelIndex child = model->index(i, 0, index);
        sync(model, child);
    }
}

void ModelCompleter::slotHighlighted(const QModelIndex& index)
{
    if (index.isValid())
    {
        QString itemName = index.data().toString();
        const auto vals  = d->idToTextHash.values();

        if (vals.count(itemName) == 1)
        {
            Q_EMIT signalHighlighted(d->idToTextHash.key(itemName));
        }
    }
}

} // namespace Digikam

#include "moc_modelcompleter.cpp"
