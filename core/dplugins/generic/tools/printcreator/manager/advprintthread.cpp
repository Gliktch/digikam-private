/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-07
 * Description : a tool to print images
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "advprintthread.h"

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"

namespace DigikamGenericPrintCreatorPlugin
{

AdvPrintThread::AdvPrintThread(QObject* const parent)
    : ActionThreadBase(parent)
{
    setObjectName(QLatin1String("AdvPrintThread"));
}

AdvPrintThread::~AdvPrintThread()
{
    cancel();
    wait();
}

void AdvPrintThread::preparePrint(AdvPrintSettings* const settings, int sizeIndex)
{
    ActionJobCollection collection;

    AdvPrintTask* const t = new AdvPrintTask(settings,
                                             AdvPrintTask::PREPAREPRINT,
                                             QSize(),
                                             sizeIndex);

    connect(t, SIGNAL(signalProgress(int)),
            this, SIGNAL(signalProgress(int)));

    connect(t, SIGNAL(signalComplete(bool)),
            this, SIGNAL(signalComplete(bool)));

    connect(t, SIGNAL(signalMessage(QString,bool)),
            this, SIGNAL(signalMessage(QString,bool)));

    collection.insert(t, 0);

    appendJobs(collection);
}

void AdvPrintThread::print(AdvPrintSettings* const settings)
{
    ActionJobCollection collection;

    AdvPrintTask* const t = new AdvPrintTask(settings,
                                             AdvPrintTask::PRINT);

    connect(t, SIGNAL(signalProgress(int)),
            this, SIGNAL(signalProgress(int)));

    connect(t, SIGNAL(signalComplete(bool)),
            this, SIGNAL(signalComplete(bool)));

    connect(t, SIGNAL(signalMessage(QString,bool)),
            this, SIGNAL(signalMessage(QString,bool)));

    collection.insert(t, 0);

    appendJobs(collection);
}

void AdvPrintThread::preview(AdvPrintSettings* const settings, const QSize& size)
{
    ActionJobCollection collection;

    AdvPrintTask* const t = new AdvPrintTask(settings,
                                             AdvPrintTask::PREVIEW,
                                             size);

    connect(t, SIGNAL(signalProgress(int)),
            this, SIGNAL(signalProgress(int)));

    connect(t, SIGNAL(signalComplete(bool)),
            this, SIGNAL(signalComplete(bool)));

    connect(t, SIGNAL(signalMessage(QString,bool)),
            this, SIGNAL(signalMessage(QString,bool)));

    connect(t, SIGNAL(signalPreview(QImage)),
            this, SIGNAL(signalPreview(QImage)));

    collection.insert(t, 0);

    appendJobs(collection);
}

} // namespace DigikamGenericPrintCreatorPlugin

#include "moc_advprintthread.cpp"
