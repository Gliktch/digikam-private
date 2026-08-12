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

#include "FpsLayer.h"

// Qt includes

#include <QPoint>
#include <QElapsedTimer>
#include <QFont>
#include <QPainter>

namespace Marble
{

FpsLayer::FpsLayer(QElapsedTimer* time)
    : m_time(time)
{
}

void FpsLayer::paint(QPainter* painter) const
{
    const qreal fps = 1000.0 / (qreal)(m_time->elapsed());
    const QString fpsString = QString::fromUtf8("Speed: %1 fps")
                              .arg(fps, 5, 'f', 1, QLatin1Char(' '));

    const QPoint fpsLabelPos(10, 20);

    painter->setFont(QFont(QStringLiteral("Sans Serif"), 10));

    painter->setPen(Qt::black);
    painter->setBrush(Qt::black);
    painter->drawText(fpsLabelPos, fpsString);

    painter->setPen(Qt::white);
    painter->setBrush(Qt::white);
    painter->drawText(fpsLabelPos.x() - 1, fpsLabelPos.y() - 1, fpsString);
}

} // namespace Marble
