/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-02-25
 * Description : Table view column helpers: Thumbnail column
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "tableview_columnfactory.h"

namespace Digikam
{

class LoadingDescription;

namespace TableViewColumns
{

class ColumnThumbnail : public TableViewColumn
{
    Q_OBJECT

private:

    explicit ColumnThumbnail(TableViewShared* const tableViewShared,
                             const TableViewColumnConfiguration& pConfiguration,
                             QWidget* const parent);
    ~ColumnThumbnail()                                                                                              override = default;

public:

    ColumnFlags getColumnFlags()                                                                              const override;
    QString getTitle()                                                                                        const override;
    QVariant data(TableViewModel::Item* const item, const int role)                                           const override;
    bool paint(QPainter* const painter, const QStyleOptionViewItem& option, TableViewModel::Item* const item) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, TableViewModel::Item* const item)                      const override;
    void updateThumbnailSize()                                                                                      override;

    static bool CreateFromConfiguration(TableViewShared* const tableViewShared,
                                        const TableViewColumnConfiguration& pConfiguration,
                                        TableViewColumn** const pNewColumn,
                                        QWidget* const parent);

    static TableViewColumnDescription getDescription();

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb);

private:

    int      m_thumbnailSize = 64;
    QWidget* m_displayWidget = nullptr;
};

} // namespace TableViewColumns

} // namespace Digikam
