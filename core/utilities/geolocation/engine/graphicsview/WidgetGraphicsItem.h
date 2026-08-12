/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "ScreenGraphicsItem.h"
#include "digikam_export.h"

class QWidget;

namespace Marble
{

class WidgetGraphicsItemPrivate;

class DIGIKAM_EXPORT WidgetGraphicsItem : public ScreenGraphicsItem
{
public:

    explicit WidgetGraphicsItem(MarbleGraphicsItem* parent = nullptr);

    ~WidgetGraphicsItem()                             override;

    void setWidget(QWidget* widget);
    QWidget* widget()                           const;

protected:

    /**
     * Paints the item in item coordinates.
     */
    void paint(QPainter* painter)                     override;

    bool eventFilter(QObject*, QEvent*)               override;

private:

    Q_DISABLE_COPY(WidgetGraphicsItem)
    Q_DECLARE_PRIVATE(WidgetGraphicsItem)
};

} // Namespace Marble
