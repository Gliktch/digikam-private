/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-03-14
 * Description : Table view column helpers: Photo properties
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

class ColumnPhotoProperties : public TableViewColumn
{
    Q_OBJECT

public:

    enum SubColumn
    {
        SubColumnCameraMaker  = 0,
        SubColumnCameraModel  = 1,
        SubColumnLens         = 2,
        SubColumnAperture     = 3,
        SubColumnFocal        = 4,
        SubColumnExposure     = 5,
        SubColumnSensitivity  = 6,
        SubColumnModeProgram  = 7,
        SubColumnFlash        = 8,
        SubColumnWhiteBalance = 9
    };

public:

    explicit ColumnPhotoProperties(TableViewShared* const tableViewShared,
                                   const TableViewColumnConfiguration& pConfiguration,
                                   const SubColumn pSubColumn,
                                   QObject* const parent = nullptr);
    ~ColumnPhotoProperties()                                                                                override = default;

    QString getTitle()                                                                                const override;
    ColumnFlags getColumnFlags()                                                                      const override;
    QVariant data(TableViewModel::Item* const item, const int role)                                   const override;
    ColumnCompareResult compare(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const override;
    TableViewColumnConfigurationWidget* getConfigurationWidget(QWidget* const parentWidget)           const override;
    void setConfiguration(const TableViewColumnConfiguration& newConfiguration) override;

    static TableViewColumnDescription getDescription();
    static QStringList getSubColumns();

private:

    SubColumn subColumn = SubColumnExposure;
};

// ----------------------------------------------------------------------------------------------------------------------

class ColumnPhotoConfigurationWidget : public TableViewColumnConfigurationWidget
{
    Q_OBJECT

public:

    explicit ColumnPhotoConfigurationWidget(TableViewShared* const sharedObject,
                                            const TableViewColumnConfiguration& columnConfiguration,
                                            QWidget* const parentWidget);
    ~ColumnPhotoConfigurationWidget()                   override = default;

    TableViewColumnConfiguration getNewConfiguration()  override;

private Q_SLOTS:

    void slotUpdateUI();

private:

    ColumnPhotoProperties::SubColumn subColumn                  = ColumnPhotoProperties::SubColumnExposure;
    QComboBox*                       selectorExposureTimeFormat = nullptr;
    QComboBox*                       selectorExposureTimeUnit   = nullptr;
};

} // namespace TableViewColumns

} // namespace Digikam
