/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-05-13
 * Description : Table view column helpers: Audio/video properties
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

class ColumnAudioVideoProperties : public TableViewColumn
{
    Q_OBJECT

public:

    enum SubColumn
    {
        SubColumnAudioBitRate     = 0,
        SubColumnAudioChannelType = 1,
        SubColumnAudioCodec       = 2,
        SubColumnDuration         = 3,
        SubColumnFrameRate        = 4,
        SubColumnVideoCodec       = 5
    };

public:

    explicit ColumnAudioVideoProperties(TableViewShared* const tableViewShared,
                                        const TableViewColumnConfiguration& pConfiguration,
                                        const SubColumn pSubColumn,
                                        QObject* const parent = nullptr);
    ~ColumnAudioVideoProperties()                                                                           override = default;

    QString getTitle()                                                                                const override;
    ColumnFlags getColumnFlags()                                                                      const override;
    QVariant data(TableViewModel::Item* const item, const int role)                                   const override;
    ColumnCompareResult compare(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const override;
    void setConfiguration(const TableViewColumnConfiguration& newConfiguration) override;

    static TableViewColumnDescription getDescription();
    static QStringList getSubColumns();

private:

    SubColumn subColumn = SubColumnDuration;
};

} // namespace TableViewColumns

} // namespace Digikam
