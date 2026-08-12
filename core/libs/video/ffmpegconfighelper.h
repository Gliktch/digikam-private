/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-03-17
 * Description : Methods to list FFMPEG features.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QStringList>
#include <QMap>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

typedef QMap<QString, QStringList> FFMpegProperties;

class DIGIKAM_EXPORT FFMpegConfigHelper
{
public:

    /**
     * Return a map of Video Codec Name with a list of properties:
     *    - Codecs description.
     *    - Read support.
     *    - Write support.
     */
    static FFMpegProperties getVideoCodecsProperties();

    /**
     * Return a map of Audio Codec Name with a list of properties:
     *    - Codecs description.
     *    - Read support.
     *    - Write support.
     */
    static FFMpegProperties getAudioCodecsProperties();

    /**
     * Return a map,of File extensions supported with a list of properties:
     *    - Format description.
     */
    static FFMpegProperties getExtensionsProperties();

private:

    FFMpegConfigHelper()  = delete;
    ~FFMpegConfigHelper() = delete;

    Q_DISABLE_COPY(FFMpegConfigHelper)
};

} // namespace Digikam
