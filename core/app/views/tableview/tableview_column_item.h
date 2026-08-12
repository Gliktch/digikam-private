/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-02-25
 * Description : Table view column helpers: Item properties
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

class ColumnItemProperties : public TableViewColumn
{
    Q_OBJECT

public:

    enum SubColumn
    {
        SubColumnWidth                = 0,
        SubColumnHeight               = 1,
        SubColumnDimensions           = 2,
        SubColumnPixelCount           = 3,
        SubColumnBitDepth             = 4,
        SubColumnColorMode            = 5,
        SubColumnType                 = 6,
        SubColumnCreationDateTime     = 7,
        SubColumnDigitizationDateTime = 8,
        SubColumnAspectRatio          = 9,
        SubColumnSimilarity           = 10
    };

public:

    explicit ColumnItemProperties(TableViewShared* const tableViewShared,
                                  const TableViewColumnConfiguration& pConfiguration,
                                  const SubColumn pSubColumn,
                                  QObject* const parent = nullptr);
    ~ColumnItemProperties()                                                                                 override = default;

    QString getTitle()                                                                                const override;
    ColumnFlags getColumnFlags()                                                                      const override;
    QVariant data(TableViewModel::Item* const item, const int role)                                   const override;
    ColumnCompareResult compare(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const override;

    static TableViewColumnDescription getDescription();
    static QStringList getSubColumns();

private:

    SubColumn subColumn = SubColumnCreationDateTime;
};

} // namespace TableViewColumns

} // namespace Digikam
