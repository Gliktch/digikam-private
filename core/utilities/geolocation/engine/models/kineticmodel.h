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

#include <QObject>
#include <QScopedPointer>
#include <QPointF>

namespace Marble
{

class KineticModelPrivate;

class KineticModel: public QObject
{
    Q_OBJECT
    Q_PROPERTY(int duration READ duration WRITE setDuration)
    Q_PROPERTY(QPointF position READ position NOTIFY positionChanged)
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval)

public:

    explicit KineticModel(QObject* parent = nullptr);
    ~KineticModel() override;

    int duration() const;
    QPointF position() const;
    int updateInterval() const;
    bool hasVelocity() const;

public Q_SLOTS:

    void setDuration(int ms);
    void setPosition(const QPointF& position);
    void setPosition(qreal posX, qreal posY);
    void setHeading(qreal heading);
    void jumpToPosition(const QPointF& position);
    void jumpToPosition(qreal posX, qreal posY);
    void setUpdateInterval(int ms);
    void stop();
    void start();

Q_SIGNALS:

    void positionChanged(qreal lon, qreal lat);
    void headingChanged(qreal heading);
    void finished();

private Q_SLOTS:

    void update();

private:

    QScopedPointer<KineticModelPrivate> d_ptr;

    Q_DECLARE_PRIVATE(KineticModel)
    Q_DISABLE_COPY(KineticModel);
};

} // namespace Marble
