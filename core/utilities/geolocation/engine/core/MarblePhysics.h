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

// Local includes

#include "MarbleGlobal.h"

namespace Marble
{

class MarblePhysicsPrivate;
class MarbleAbstractPresenter;
class GeoDataLookAt;

class MarblePhysics : public QObject
{
    Q_OBJECT

public:

    /**
      * @brief Constructor
      * @param presenter the MarbleWidget that is being animated
      */
    explicit MarblePhysics(MarbleAbstractPresenter* presenter);

    /**
      * @brief Destructor
      */
    ~MarblePhysics() override;

    /**
      * @brief Initiate an animation to the target according to the given mode.
      * @param target camera position indicating the target of the animation
      * @param mode animation mode; @code Instant @endcode means no animation
      */
    void flyTo(const GeoDataLookAt& target, FlyToMode mode = Instant);

private Q_SLOTS:

    void updateProgress(qreal progress);

    /**
      * @brief Switch to still mode when an animation is finished
      */
    void startStillMode();

private:

    Q_DISABLE_COPY(MarblePhysics)

private:

    MarblePhysicsPrivate* d = nullptr;
};

} // Namespace Marble
