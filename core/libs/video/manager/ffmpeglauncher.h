/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : FFmpeg launcher to encode frames as video
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMap>
#include <QString>
#include <QTime>

// Local includes

#include "processlauncher.h"
#include "vidslidesettings.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT FFmpegLauncher : public ProcessLauncher
{
    Q_OBJECT

public:

    explicit FFmpegLauncher(QObject* const parent = nullptr);
    ~FFmpegLauncher() override = default;

    /**
     * Set encoding frames settings.
     */
    void setSettings(VidSlideSettings* const settings);

    /**
     * Encode frames in a separated thread.
     */
    void encodeFrames();

    /**
     * Get the map of supported codecs with features.
     */
    QMap<QString, QString> supportedCodecs();

    /**
     * Get the map of supported formats with features.
     */
    QMap<QString, QString> supportedFormats();

    /**
     * Return the length of an audio file.
     * If duration cannot be decoded, it returns a null QTime.
     */
    QTime soundTrackLength(const QString& audioPath);

private:

    VidSlideSettings* m_settings = nullptr;
};

} // namespace Digikam
