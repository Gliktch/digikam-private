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

#include "ScreenGraphicsItem_p.h"
#include "WidgetGraphicsItem.h"

class QWidget;

namespace Marble
{

class Q_DECL_HIDDEN WidgetGraphicsItemPrivate : public ScreenGraphicsItemPrivate
{
public:

    WidgetGraphicsItemPrivate(WidgetGraphicsItem* widgetGraphicsItem,
                              MarbleGraphicsItem* parent);

    ~WidgetGraphicsItemPrivate() override;

public:

    QWidget* m_widget       = nullptr;
    QWidget* m_marbleWidget = nullptr;
    QWidget* m_activeWidget = nullptr;
};

} // Namespace Marble
