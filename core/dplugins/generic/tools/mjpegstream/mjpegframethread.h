/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-24
 * Description : a MJPEG frame generator.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QByteArray>

// Local includes

#include "actionthread.h"
#include "mjpegstreamsettings.h"

using namespace Digikam;

namespace DigikamGenericMjpegStreamPlugin
{

class MjpegFrameThread : public ActionThreadBase
{
    Q_OBJECT

public:

    /**
     * Thread manager to fork MJPEg frame task to a separated core.
     */
    explicit MjpegFrameThread(QObject* const parent);
    ~MjpegFrameThread() override;

    /**
     * Instantiate MJPEG frame task thread with right settings.
     */
    void createFrameJob(const MjpegStreamSettings&);

Q_SIGNALS:

    /**
     * Re-route signal from task to MJPEG server.
     */
    void signalFrameChanged(const QByteArray&);
};

} // namespace DigikamGenericMjpegStreamPlugin
