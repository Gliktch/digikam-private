/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-20
 * Description : Database Engine container to wrap data types
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Local includes

#include "dbengineactiontype.h"

namespace Digikam
{

class Q_DECL_HIDDEN DbEngineActionType::Private
{

public:

    Private() = default;

    bool     isValue    = false;
    QVariant actionValue;
};

DbEngineActionType::DbEngineActionType()
    : d(new Private)
{
}

DbEngineActionType::DbEngineActionType(const DbEngineActionType& actionType)
    : d(new Private)
{
    d->isValue     = actionType.d->isValue;
    d->actionValue = actionType.d->actionValue;
}

DbEngineActionType::~DbEngineActionType()
{
    delete d;
}

DbEngineActionType DbEngineActionType::value(const QVariant& value)
{
    DbEngineActionType actionType;
    actionType.setValue(true);
    actionType.setActionValue(value);

    return actionType;
}

DbEngineActionType DbEngineActionType::fieldEntry(const QVariant& actionValue)
{
    DbEngineActionType actionType;
    actionType.setValue(false);
    actionType.setActionValue(actionValue);

    return actionType;
}

QVariant DbEngineActionType::getActionValue()
{
    return d->actionValue;
}

void DbEngineActionType::setActionValue(const QVariant& actionValue)
{
    d->actionValue = actionValue;
}

bool DbEngineActionType::isValue() const
{
    return d->isValue;
}

void DbEngineActionType::setValue(bool isValue)
{
    d->isValue = isValue;
}

} // namespace Digikam
