/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-22
 * Description : A model for the view to display a list of items.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QItemDelegate>

// Local includes

#include "gpsitemlist.h"

namespace Digikam
{

class GPSItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:

    explicit GPSItemDelegate(GPSItemList* const imageList, QObject* const parent = nullptr);
    ~GPSItemDelegate()                                                                                          override;

    void setThumbnailSize(const int size);
    int  getThumbnailSize()                                                                               const;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& sortMappedindex) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& sortMappedindex)                const override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
