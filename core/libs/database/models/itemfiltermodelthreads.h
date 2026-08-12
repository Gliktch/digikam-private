/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-11
 * Description : Qt item model for database entries - threads management
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QThread>

// Local includes

#include "digikam_export.h"
#include "workerobject.h"
#include "itemfiltermodel.h"

namespace Digikam
{

class ItemFilterModelTodoPackage;

class DIGIKAM_DATABASE_EXPORT ItemFilterModelWorker : public WorkerObject
{
    Q_OBJECT

public:

    explicit ItemFilterModelWorker(ItemFilterModel::ItemFilterModelPrivate* const dd);

    bool checkVersion(const ItemFilterModelTodoPackage& package);

public Q_SLOTS:

    virtual void process(ItemFilterModelTodoPackage package) = 0;

Q_SIGNALS:

    void processed(const ItemFilterModelTodoPackage& package);
    void discarded(const ItemFilterModelTodoPackage& package);

protected:

    ItemFilterModel::ItemFilterModelPrivate* d = nullptr;
};

// -----------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemFilterModelPreparer : public ItemFilterModelWorker
{
    Q_OBJECT

public:

    explicit ItemFilterModelPreparer(ItemFilterModel::ItemFilterModelPrivate* const d)
        : ItemFilterModelWorker(d)
    {
    }

    void process(ItemFilterModelTodoPackage package) override;
};

// ----------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemFilterModelFilterer : public ItemFilterModelWorker
{
    Q_OBJECT

public:

    explicit ItemFilterModelFilterer(ItemFilterModel::ItemFilterModelPrivate* const d)
        : ItemFilterModelWorker(d)
    {
    }

    void process(ItemFilterModelTodoPackage package) override;
};

} // namespace Digikam
