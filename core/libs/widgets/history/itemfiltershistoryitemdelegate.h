/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-02
 * Description : delegate for custom painting of used filters view
 *
 * SPDX-FileCopyrightText: 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QStyledItemDelegate>

namespace Digikam
{

class ItemFiltersHistoryItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    explicit ItemFiltersHistoryItemDelegate(QObject* const parent = nullptr);
    ~ItemFiltersHistoryItemDelegate()                                                                 override = default;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index)                const override;
};

} //namespace Digikam
