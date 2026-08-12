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

#include "mjpegframethread.h"

// Local includes

#include "mjpegframetask.h"

namespace DigikamGenericMjpegStreamPlugin
{

MjpegFrameThread::MjpegFrameThread(QObject* const parent)
    : ActionThreadBase(parent)
{
}

MjpegFrameThread::~MjpegFrameThread()
{
    // cancel the thread

    cancel();

    // wait for the thread to finish

    wait();
}

void MjpegFrameThread::createFrameJob(const MjpegStreamSettings& set)
{
    ActionJobCollection collection;

    MjpegFrameTask* const t = new MjpegFrameTask(set);

    connect(t, SIGNAL(signalFrameChanged(QByteArray)),
            this, SIGNAL(signalFrameChanged(QByteArray)));

    collection.insert(t, 0);

    appendJobs(collection);
}

} // namespace DigikamGenericMjpegStreamPlugin

#include "moc_mjpegframethread.cpp"
