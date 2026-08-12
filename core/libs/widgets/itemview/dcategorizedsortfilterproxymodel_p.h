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

#pragma once

#include "dcategorizedsortfilterproxymodel.h"

// Qt includes

#include <QCollator>

namespace Digikam
{

class DCategorizedSortFilterProxyModel;

class Q_DECL_HIDDEN DCategorizedSortFilterProxyModel::Private
{
public:

    Private()  = default;
    ~Private() = default;

public:

    int           sortColumn                            = 0;
    Qt::SortOrder sortOrder                             = Qt::AscendingOrder;
    bool          categorizedModel                      = false;
    bool          sortCategoriesByNaturalComparison     = true;
    QCollator     collator;
};

} // namespace Digikam
