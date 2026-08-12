/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-03-27
 * Description : file copy thread.
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fcthread.h"

// Local includes

#include "fctask.h"

namespace DigikamGenericFileCopyPlugin
{

FCThread::FCThread(QObject* const parent)
    : ActionThreadBase(parent)
{
}

FCThread::~FCThread()
{
    // cancel the thread

    this->cancel();

    // wait for the thread to finish

    wait();
}

void FCThread::createCopyJobs(const QList<QUrl>& itemsList,
                              const FCContainer& settings)
{
    ActionJobCollection collection;

    for (const QUrl& srcUrl : std::as_const(itemsList))
    {
        FCTask* const t = new FCTask(srcUrl, settings);

        connect(t, SIGNAL(signalUrlProcessed(QUrl,QUrl)),
                this, SIGNAL(signalUrlProcessed(QUrl,QUrl)));

        connect(this, SIGNAL(signalCancelTask()),
                t, SLOT(cancel()), Qt::QueuedConnection);

        collection.insert(t, 0);
     }

    appendJobs(collection);
}

void FCThread::cancel()
{
    if (isRunning())
    {
        Q_EMIT signalCancelTask();
    }

    ActionThreadBase::cancel();
}

} // namespace DigikamGenericFileCopyPlugin

#include "moc_fcthread.cpp"
