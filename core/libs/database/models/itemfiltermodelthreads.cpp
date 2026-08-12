/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries - threads management
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

#include "itemfiltermodelthreads.h"
#include "itemfiltermodel_p.h"

namespace Digikam
{

ItemFilterModelWorker::ItemFilterModelWorker(ItemFilterModel::ItemFilterModelPrivate* const dd)
    : d(dd)
{
}

bool ItemFilterModelWorker::checkVersion(const ItemFilterModelTodoPackage& package)
{
    return (d->version == package.version);
}

} // namespace Digikam

#include "moc_itemfiltermodelthreads.cpp"
