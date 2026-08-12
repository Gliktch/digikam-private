/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : contextmenu helper class
 *
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "contextmenuhelper_p.h"

namespace Digikam
{

ContextMenuHelper::ContextMenuHelper(QMenu* const parent)
    : QObject(parent),
      d      (new Private(this))
{
    d->parent              = parent;
    d->stdActionCollection = DigikamApp::instance()->actionCollection();
}

ContextMenuHelper::~ContextMenuHelper()
{
    delete d;
}

void ContextMenuHelper::setItemFilterModel(ItemFilterModel* const model)
{
    d->imageFilterModel = model;
}

void ContextMenuHelper::setSelectedIds(const imageIds& ids)
{
    if (d->selectedIds.isEmpty())
    {
        d->selectedIds = ids;
    }
}

void ContextMenuHelper::setSelectedItems(const QList<QUrl>& urls)
{
    if (d->selectedItems.isEmpty())
    {
        d->selectedItems = urls;
    }
}

bool ContextMenuHelper::imageIdsHaveSameCategory(const imageIds& ids, DatabaseItem::Category category)
{
    bool sameCategory = true;
    QVariantList varList;

    for (const qlonglong& id : std::as_const(ids))
    {
        varList = CoreDbAccess().db()->getImagesFields(id, DatabaseFields::Category);

        if (
            varList.isEmpty() ||
            ((DatabaseItem::Category)varList.first().toInt() != category)
           )
        {
            sameCategory = false;
            break;
        }
    }

    return sameCategory;
}

} // namespace Digikam

#include "moc_contextmenuhelper.cpp"
