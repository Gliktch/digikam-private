/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-02-28
 * Description : Table view column helpers: Digikam properties
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QStringList>

// Local includes

#include "tableview_columnfactory.h"

namespace Digikam
{

namespace TableViewColumns
{

class ColumnDigikamProperties : public TableViewColumn
{
    Q_OBJECT

public:

    enum SubColumn
    {
        SubColumnRating     = 0,
        SubColumnPickLabel  = 1,
        SubColumnColorLabel = 2,
        SubColumnTitle      = 3,
        SubColumnCaption    = 4,
        SubColumnTags       = 5,
        SubColumnGrouped    = 6,
        SubColumnVersioned  = 7
    };

public:

    explicit ColumnDigikamProperties(TableViewShared* const tableViewShared,
                                     const TableViewColumnConfiguration& pConfiguration,
                                     const SubColumn pSubColumn,
                                     QObject* const parent = nullptr);
    ~ColumnDigikamProperties()                                                                              override = default;

    QString getTitle()                                                                                const override;
    ColumnFlags getColumnFlags()                                                                      const override;
    QVariant data(TableViewModel::Item* const item, const int role)                                   const override;
    ColumnCompareResult compare(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const override;
    bool columnAffectedByChangeset(const ImageChangeset& imageChangeset)                              const override;

public:

    static TableViewColumnDescription getDescription();
    static QStringList getSubColumns();

private:

    SubColumn subColumn = SubColumnRating;
};

} // namespace TableViewColumns

} // namespace Digikam
