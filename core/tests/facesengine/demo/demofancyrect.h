/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-23
 * Description : QGraphicsRectItem wrapper for FacesEngine Demo
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008      by Adrien Bustany <madcat at mymadcat dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QRectF>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

namespace FaceEngineDemo
{

class FancyRect : public QGraphicsRectItem
{

public:

    explicit FancyRect(QGraphicsItem* const parent = nullptr);
    explicit FancyRect(const QRectF& rect, QGraphicsItem* const parent = nullptr);

    FancyRect(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* const parent = nullptr);
    FancyRect(QGraphicsRectItem* const other, QGraphicsItem* const parent);

public:

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
};

} // namespace FaceEngineDemo
