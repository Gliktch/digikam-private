/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries - private containers
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText:      2011 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText:      2014 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemfiltermodel_p.h"

// Local includes

#include "digikam_debug.h"
#include "itemfiltermodelthreads.h"

namespace Digikam
{

ItemFilterModel::ItemFilterModelPrivate::ItemFilterModelPrivate()
{
    setupWorkers();
}

ItemFilterModel::ItemFilterModelPrivate::~ItemFilterModelPrivate()
{
    // facilitate thread stopping

    version = version + 1;
    preparer->deactivate();
    filterer->deactivate();

    delete preparer;
    delete filterer;
}

void ItemFilterModel::ItemFilterModelPrivate::setupModel(ItemFilterModel* const qq)
{
    q = qq;

    updateFilterTimer = new QTimer(this);
    updateFilterTimer->setSingleShot(true);
    updateFilterTimer->setInterval(250);

    connect(updateFilterTimer, SIGNAL(timeout()),
            q, SLOT(slotUpdateFilter()));

    // inter-thread redirection

    qRegisterMetaType<ItemFilterModelTodoPackage>("ItemFilterModelTodoPackage");
}

void ItemFilterModel::ItemFilterModelPrivate::preprocessInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    infosToProcess(infos, extraValues, true);
}

void ItemFilterModel::ItemFilterModelPrivate::processAddedInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    // These have already been added, we just process them afterwards

    infosToProcess(infos, extraValues, false);
}

void ItemFilterModel::ItemFilterModelPrivate::setupWorkers()
{
    preparer = new ItemFilterModelPreparer(this);
    filterer = new ItemFilterModelFilterer(this);

    // A package in constructed in infosToProcess.
    // Normal flow is infosToProcess -> preparer::process -> filterer::process -> packageFinished.
    // If no preparation is needed, the first step is skipped.
    // If filter version changes, both will discard old package and send them to packageDiscarded.

    connect(this, SIGNAL(packageToPrepare(ItemFilterModelTodoPackage)),
            preparer, SLOT(process(ItemFilterModelTodoPackage)));

    connect(this, SIGNAL(packageToFilter(ItemFilterModelTodoPackage)),
            filterer, SLOT(process(ItemFilterModelTodoPackage)));

    connect(preparer, SIGNAL(processed(ItemFilterModelTodoPackage)),
            filterer, SLOT(process(ItemFilterModelTodoPackage)));

    connect(filterer, SIGNAL(processed(ItemFilterModelTodoPackage)),
            this, SLOT(packageFinished(ItemFilterModelTodoPackage)));

    connect(preparer, SIGNAL(discarded(ItemFilterModelTodoPackage)),
            this, SLOT(packageDiscarded(ItemFilterModelTodoPackage)));

    connect(filterer, SIGNAL(discarded(ItemFilterModelTodoPackage)),
            this, SLOT(packageDiscarded(ItemFilterModelTodoPackage)));
}

void ItemFilterModel::ItemFilterModelPrivate::infosToProcess(const QList<ItemInfo>& infos)
{
    infosToProcess(infos, QList<QVariant>(), false);
}

void ItemFilterModel::ItemFilterModelPrivate::infosToProcess(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues, bool forReAdd)
{
    if (infos.isEmpty())
    {
        return;
    }

    filterer->schedule();

    if (needPrepare)
    {
        preparer->schedule();
    }

    Q_ASSERT(extraValues.isEmpty() || (infos.size() == extraValues.size()));

    // prepare and filter in chunks

    const int size                      = infos.size();
    const int maxChunkSize              = needPrepare ? PrepareChunkSize : FilterChunkSize;
    const bool hasExtraValues           = !extraValues.isEmpty();
    QList<ItemInfo>::const_iterator it  = infos.constBegin(), end;
    QList<QVariant>::const_iterator xit = extraValues.constBegin(), xend;
    int index                           = 0;
    QVector<ItemInfo> infoVector;
    QVector<QVariant> extraValueVector;

    while (it != infos.constEnd())
    {
        const int chunkSize = qMin(maxChunkSize, size - index);
        infoVector.resize(chunkSize);
        end                 = it + chunkSize;
        std::copy(it, end, infoVector.begin());

        if (hasExtraValues)
        {
            extraValueVector.resize(chunkSize);
            xend = xit + chunkSize;
            std::copy(xit, xend, extraValueVector.begin());
            xit  = xend;
        }

        it     = end;
        index += chunkSize;

        ++sentOut;

        if (forReAdd)
        {
            ++sentOutForReAdd;
        }

        if (needPrepare)
        {
            Q_EMIT packageToPrepare(ItemFilterModelTodoPackage(infoVector, extraValueVector, version, forReAdd));
        }
        else
        {
            Q_EMIT packageToFilter(ItemFilterModelTodoPackage(infoVector, extraValueVector, version, forReAdd));
        }
    }
}

void ItemFilterModel::ItemFilterModelPrivate::packageFinished(const ItemFilterModelTodoPackage& package)
{
    // check if it got discarded on the journey

    if (package.version != version)
    {
        packageDiscarded(package);
        return;
    }

    // incorporate result

    QHash<qlonglong, bool>::const_iterator it = package.filterResults.constBegin();

    for ( ; it != package.filterResults.constEnd() ; ++it)
    {
        filterResults.insert(it.key(), it.value());
    }

    // re-add if necessary

    if (package.isForReAdd)
    {
        Q_EMIT reAddItemInfos(package.infos.toList(), package.extraValues.toList());

        if (sentOutForReAdd == 1) // last package
        {
            Q_EMIT reAddingFinished();
        }
    }

    // decrement counters

    --sentOut;

    if (package.isForReAdd)
    {
        --sentOutForReAdd;
    }

    // If all packages have returned, filtered and readded, and no more are expected,
    // and there is need to tell the filter result to the view, do that

    if ((sentOut == 0) && (sentOutForReAdd == 0) && !imageModel->isRefreshing())
    {
        q->invalidate(); // use invalidate, not invalidateFilter only. Sorting may have changed as well.

        Q_EMIT q->filterMatches(hasOneMatch);
        Q_EMIT q->filterMatchesForText(hasOneMatchForText);

        filterer->deactivate();
        preparer->deactivate();
    }
}

void ItemFilterModel::ItemFilterModelPrivate::packageDiscarded(const ItemFilterModelTodoPackage& package)
{
    // Either, the model was reset, or the filter changed
    // In the former case throw all away, in the latter case, recycle

    if (package.version > lastDiscardVersion)
    {
        // Recycle packages: Send again with current version
        // Do not increment sentOut or sentOutForReAdd here: it was not decremented!

        if (needPrepare)
        {
            Q_EMIT packageToPrepare(ItemFilterModelTodoPackage(package.infos, package.extraValues, version, package.isForReAdd));
        }
        else
        {
            Q_EMIT packageToFilter(ItemFilterModelTodoPackage(package.infos, package.extraValues, version, package.isForReAdd));
        }
    }
}

} // namespace Digikam

#include "moc_itemfiltermodel_p.cpp"
