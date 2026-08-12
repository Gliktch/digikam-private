/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-23
 * Description : face marquer widget for FacesEngine Demo
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Adrien Bustany <madcat at mymadcat dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QRectF>
#include <QGraphicsItemGroup>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>

namespace FaceEngineDemo
{

class FancyRect;

class Marquee : public QObject,
    public QGraphicsItemGroup
{
    Q_OBJECT

public:

    explicit Marquee(FancyRect* const rect, QGraphicsItem* const parent = nullptr);
    ~Marquee() override;

    QRectF boundingRect() const override;
    QRectF toRectF()      const;

Q_SIGNALS:

    void selected(Marquee* m);
    void changed();
    void entered();
    void left();

protected:

    void mousePressEvent(QGraphicsSceneMouseEvent* e)   override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* e)    override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;

private:

    void createHandles();
    void placeHandles();
    void placeLabel();

private:

    /// @note disabled
    explicit Marquee(QObject*);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace FaceEngineDemo
