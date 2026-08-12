/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-01-16
 * Description : categorize item view based on DCategorizedView
 *
 * SPDX-FileCopyrightText: 2007      by Rafael Fernández López <ereslibre at kde dot org>
 * SPDX-FileCopyrightText: 2007      by John Tapsell <tapsell at kde dot org>
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dcategorizedsortfilterproxymodel_p.h"

// Qt includes

#include <QItemSelection>
#include <QStringList>
#include <QCollator>
#include <QAction>
#include <QSize>

// Local includes

#include "categorizeditemmodel.h"

namespace Digikam
{

DCategorizedSortFilterProxyModel::DCategorizedSortFilterProxyModel(QObject* const parent)
    : QSortFilterProxyModel(parent),
      d                    (new Private)
{
    setDynamicSortFilter(false);
}

DCategorizedSortFilterProxyModel::~DCategorizedSortFilterProxyModel()
{
    delete d;
}

void DCategorizedSortFilterProxyModel::sort(int column, Qt::SortOrder order)
{
    d->sortColumn = column;
    d->sortOrder  = order;

    QSortFilterProxyModel::sort(column, order);
}

bool DCategorizedSortFilterProxyModel::isCategorizedModel() const
{
    return d->categorizedModel;
}

void DCategorizedSortFilterProxyModel::setCategorizedModel(bool categorizedModel)
{
    if (categorizedModel == d->categorizedModel)
    {
        return;
    }

    d->categorizedModel = categorizedModel;

    invalidate();
}

int DCategorizedSortFilterProxyModel::sortColumn() const
{
    return d->sortColumn;
}

Qt::SortOrder DCategorizedSortFilterProxyModel::sortOrder() const
{
    return d->sortOrder;
}

void DCategorizedSortFilterProxyModel::setSortCategoriesByNaturalComparison(bool sortCategoriesByNaturalComparison)
{
    if (sortCategoriesByNaturalComparison == d->sortCategoriesByNaturalComparison)
    {
        return;
    }

    d->sortCategoriesByNaturalComparison = sortCategoriesByNaturalComparison;

    invalidate();
}

bool DCategorizedSortFilterProxyModel::sortCategoriesByNaturalComparison() const
{
    return d->sortCategoriesByNaturalComparison;
}

bool DCategorizedSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    if (d->categorizedModel)
    {
        int compare = compareCategories(left, right);

        if      (compare > 0) // left is greater than right
        {
            return false;
        }
        else if (compare < 0) // left is less than right
        {
            return true;
        }
    }

    return subSortLessThan(left, right);
}

bool DCategorizedSortFilterProxyModel::subSortLessThan(const QModelIndex& left, const QModelIndex& right) const
{
    return QSortFilterProxyModel::lessThan(left, right);
}

int DCategorizedSortFilterProxyModel::compareCategories(const QModelIndex& left, const QModelIndex& right) const
{
    QVariant l = (left.model()  ? left.model()->data(left,   CategorySortRole) : QVariant());
    QVariant r = (right.model() ? right.model()->data(right, CategorySortRole) : QVariant());

    Q_ASSERT(l.isValid());
    Q_ASSERT(r.isValid());

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    Q_ASSERT(l.typeId() == r.typeId());

    if (l.typeId() == QMetaType::QString)

#else

    Q_ASSERT(l.type() == r.type());

    if (l.type() == QVariant::String)

#endif

    {
        QString lstr = l.toString();
        QString rstr = r.toString();

        if (d->sortCategoriesByNaturalComparison)
        {
            return (d->collator.compare(lstr, rstr));
        }
        else
        {
            if (lstr < rstr)
            {
                return (-1);
            }

            if (lstr > rstr)
            {
                return 1;
            }

            return 0;
        }
    }

    qlonglong lint = l.toLongLong();
    qlonglong rint = r.toLongLong();

    if (lint < rint)
    {
        return -1;
    }

    if (lint > rint)
    {
        return 1;
    }

    return 0;
}

// -----------------------------------------------------------------------------------------------------------------------

ActionSortFilterProxyModel::ActionSortFilterProxyModel(QObject* const parent)
    : DCategorizedSortFilterProxyModel(parent)
{
}

bool ActionSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index     = sourceModel()->index(source_row, 0, source_parent);
    QAction* const action = ActionItemModel::actionForIndex(index);

    return (action && action->isVisible());
}

} // namespace Digikam

#include "moc_dcategorizedsortfilterproxymodel.cpp"
