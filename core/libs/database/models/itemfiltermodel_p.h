/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-11
 * Description : Qt item model for database entries - private containers
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <QThread>
#include <QTimer>
#include <QWaitCondition>

// Local includes

#include "iteminfo.h"
#include "itemfiltermodel.h"
#include "digikam_export.h"

namespace Digikam
{

const int PrepareChunkSize = 101;
const int FilterChunkSize  = 2001;

class Q_DECL_HIDDEN ItemFilterModelTodoPackage
{
public:

    ItemFilterModelTodoPackage() = default;

    ItemFilterModelTodoPackage(const QVector<ItemInfo>& infos,
                               const QVector<QVariant>& extraValues,
                               int version,
                               bool isForReAdd)
        : infos      (infos),
          extraValues(extraValues),
          version    (version),
          isForReAdd (isForReAdd)
    {
    }

public:

    QVector<ItemInfo>      infos;
    QVector<QVariant>      extraValues;
    unsigned int           version      = 0;
    bool                   isForReAdd   = false;
    QHash<qlonglong, bool> filterResults;
};

// ------------------------------------------------------------------------------------------------

class ItemFilterModelPreparer;
class ItemFilterModelFilterer;

/**
 * @class ItemFilterModelPrivate
 * @internal
 *
 * @note We need the EXPORT macro for the @class ItemFilterModelPrivate because
 * this class is shared across binary objects.
 * This does not make this classes here any more public.
 */
class DIGIKAM_DATABASE_EXPORT ItemFilterModel::ItemFilterModelPrivate : public QObject
{
    Q_OBJECT

public:

    ItemFilterModelPrivate();
    ~ItemFilterModelPrivate() override;

    void setupModel(ItemFilterModel* const qq);
    void setupWorkers();
    void infosToProcess(const QList<ItemInfo>& infos);
    void infosToProcess(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues, bool forReAdd = true);

public:

    ItemFilterModel*                   q                    = nullptr;

    ItemModel*                         imageModel           = nullptr;

    ItemFilterSettings                 filter;
    ItemSortSettings                   sorter;
    VersionItemFilterSettings          versionFilter;
    GroupItemFilterSettings            groupFilter;

    volatile unsigned int              version              = 0;
    unsigned int                       lastDiscardVersion   = 0;
    unsigned int                       lastFilteredVersion  = 0;
    int                                sentOut              = 0;
    int                                sentOutForReAdd      = 0;

    QTimer*                            updateFilterTimer    = nullptr;

    bool                               needPrepare          = false;
    bool                               needPrepareComments  = false;
    bool                               needPrepareTags      = false;
    bool                               needPrepareGroups    = false;

    QMutex                             mutex;
    ItemFilterSettings                 filterCopy;
    VersionItemFilterSettings          versionFilterCopy;
    GroupItemFilterSettings            groupFilterCopy;
    ItemFilterModelPreparer*           preparer             = nullptr;
    ItemFilterModelFilterer*           filterer             = nullptr;

    QHash<qlonglong, bool>             filterResults;
    bool                               hasOneMatch          = false;
    bool                               hasOneMatchForText   = false;

    QList<ItemFilterModelPrepareHook*> prepareHooks;

/*
    QHash<int, QSet<qlonglong> >       categoryCountHashInt;
    QHash<QString, QSet<qlonglong> >   categoryCountHashString;

public:

        void cacheCategoryCount(int id, qlonglong imageid) const
        {
            const_cast<ItemFilterModelPrivate*>(this)->categoryCountHashInt[id].insert(imageid);
        }

        void cacheCategoryCount(const QString& id, qlonglong imageid) const
        {
            const_cast<ItemFilterModelPrivate*>(this)->categoryCountHashString[id].insert(imageid);
        }
*/

public Q_SLOTS:

    void preprocessInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues);
    void processAddedInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues);
    void packageFinished(const ItemFilterModelTodoPackage& package);
    void packageDiscarded(const ItemFilterModelTodoPackage& package);

Q_SIGNALS:

    void packageToPrepare(const ItemFilterModelTodoPackage& package);
    void packageToFilter(const ItemFilterModelTodoPackage& package);
    void reAddItemInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues);
    void reAddingFinished();

private:

    /// @note disabled

    explicit ItemFilterModelPrivate(QObject*) = delete;
};

} // namespace Digikam
