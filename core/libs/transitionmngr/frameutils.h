/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-24
 * Description : utils to generate video frames.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QSize>
#include <QString>
#include <QImage>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT FrameUtils
{
public:

    static QImage makeFramedImage(const QString& file, const QSize& outSize);
    static QImage makeScaledImage(QImage& timg, const QSize& outSize);
};

} // namespace Digikam
