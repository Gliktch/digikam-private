/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-02-25
 * Description : Table view column helpers: Geographic column
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

class ColumnGeoProperties : public TableViewColumn
{
    Q_OBJECT

public:

    enum SubColumn
    {
        SubColumnHasCoordinates = 0,
        SubColumnCoordinates    = 1,
        SubColumnAltitude       = 2
    } subColumn;

public:

    explicit ColumnGeoProperties(TableViewShared* const tableViewShared,
                                 const TableViewColumnConfiguration& pConfiguration,
                                 const SubColumn pSubColumn,
                                 QObject* const parent = nullptr);
    ~ColumnGeoProperties()                                                                                  override = default;

    QString getTitle()                                                                                const override;
    ColumnFlags getColumnFlags()                                                                      const override;
    QVariant data(TableViewModel::Item* const item, const int role)                                   const override;
    ColumnCompareResult compare(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const override;
    TableViewColumnConfigurationWidget* getConfigurationWidget(QWidget* const parentWidget)           const override;
    void setConfiguration(const TableViewColumnConfiguration& newConfiguration) override;

    static QStringList getSubColumns();
    static TableViewColumnDescription getDescription();
};

// -----------------------------------------------------------------------------------------------

class ColumnGeoConfigurationWidget : public TableViewColumnConfigurationWidget
{
    Q_OBJECT

public:
    explicit ColumnGeoConfigurationWidget(TableViewShared* const sharedObject,
                                          const TableViewColumnConfiguration& columnConfiguration,
                                          QWidget* const parentWidget);
    ~ColumnGeoConfigurationWidget()                     override = default;

    TableViewColumnConfiguration getNewConfiguration()  override;

private:

    ColumnGeoProperties::SubColumn subColumn            = ColumnGeoProperties::SubColumnHasCoordinates;
    QComboBox*                     selectorAltitudeUnit = nullptr;
};

} // namespace TableViewColumns

} // namespace Digikam
