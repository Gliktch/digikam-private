/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-02-25
 * Description : Table view column helpers: File properties
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

class QComboBox;

namespace Digikam
{

namespace TableViewColumns
{

class ColumnFileProperties : public TableViewColumn
{
    Q_OBJECT

public:

    enum SubColumn
    {
        SubColumnName         = 0,
        SubColumnFilePath     = 1,
        SubColumnSize         = 2,
        SubColumnLastModified = 3
    };

public:

    explicit ColumnFileProperties(TableViewShared* const tableViewShared,
                                  const TableViewColumnConfiguration& pConfiguration,
                                  const SubColumn pSubColumn,
                                  QObject* const parent = nullptr);
    ~ColumnFileProperties()                                                                                 override = default;

    TableViewColumnConfigurationWidget* getConfigurationWidget(QWidget* const parentWidget)           const override;
    void setConfiguration(const TableViewColumnConfiguration& newConfiguration) override;
    QString getTitle()                                                                                const override;
    ColumnFlags getColumnFlags()                                                                      const override;
    QVariant data(TableViewModel::Item* const item, const int role)                                   const override;
    ColumnCompareResult compare(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const override;

public:

    static TableViewColumnDescription getDescription();
    static QStringList                getSubColumns();

private:

    SubColumn subColumn = SubColumnName;
};

// ---------------------------------------------------------------------------------------

class ColumnFileConfigurationWidget : public TableViewColumnConfigurationWidget
{
    Q_OBJECT

public:

    explicit ColumnFileConfigurationWidget(TableViewShared* const sharedObject,
                                           const TableViewColumnConfiguration& columnConfiguration,
                                           QWidget* const parentWidget);
    ~ColumnFileConfigurationWidget()                    override = default;

    TableViewColumnConfiguration getNewConfiguration()  override;

private:

    ColumnFileProperties::SubColumn subColumn        = ColumnFileProperties::SubColumnName;
    QComboBox*                      selectorSizeType = nullptr;
};

} // namespace TableViewColumns

} // namespace Digikam
