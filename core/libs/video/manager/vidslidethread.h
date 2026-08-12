/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "vidslidesettings.h"
#include "vidslidetask.h"
#include "actionthreadbase.h"
#include "digikam_export.h"
#include "ffmpeglauncher.h"

namespace Digikam
{

class DIGIKAM_EXPORT VidSlideThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit VidSlideThread(QObject* const parent);
    ~VidSlideThread() override;

    /**
     * Stage 1: prepare frames in temporary directory.
     */
    void prepareFrames(VidSlideSettings* const settings);

    QString encodingTraces() const;

Q_SIGNALS:

    void signalProgress(int);
    void signalComplete(bool);
    void signalMessage(const QString&, bool);

private Q_SLOTS:

    /**
     * Stage 2: encode frames as video stream.
     */
    void slotEncodeFrames(bool prepareDone);

    /**
     * Called when stage 2 is done.
     */
    void slotEncodeDone(bool timedOut, int exitCode);

private:

    VidSlideSettings* m_settings = nullptr;
    FFmpegLauncher*   m_encoder  = nullptr;
};

} // namespace Digikam
