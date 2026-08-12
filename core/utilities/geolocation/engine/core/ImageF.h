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

#include <QImage>

namespace Marble
{

class ImageF
{

private:

    ImageF();

public:

    /**
    * @brief Returns the color value of the result tile at a given floating point position.
    * @return The uint that describes the color value of the given pixel
    */
    static uint pixelF(const QImage& image, qreal x, qreal y);
};

} // Namespace Marble
