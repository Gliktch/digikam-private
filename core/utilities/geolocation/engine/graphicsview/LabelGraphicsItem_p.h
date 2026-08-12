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

// Qt includes

#include <QString>
#include <QSizeF>
#include <QImage>
#include <QIcon>

// Local includes

#include "FrameGraphicsItem_p.h"

class QFont;

namespace Marble
{

class LabelGraphicsItem;

class Q_DECL_HIDDEN LabelGraphicsItemPrivate : public FrameGraphicsItemPrivate
{
public:

    LabelGraphicsItemPrivate(LabelGraphicsItem* labelGraphicsItem,
                             MarbleGraphicsItem* parent);

    // TODO: This has to go up to MarbleGraphicsItem
    static QFont font();

public:

    QString     m_text;
    QImage      m_image;
    QIcon       m_icon;

    QSizeF      m_minimumSize;
};

} // namespace Marble
