/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-24
 * Description : DNG Converter threads manager
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dngconverterthread.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "drawinfo.h"
#include "drawdecoder.h"
#include "dngconvertertask.h"
#include "dmetadata.h"
#include "digikam_debug.h"

namespace DigikamGenericDNGConverterPlugin
{

class Q_DECL_HIDDEN DNGConverterActionThread::Private
{
public:

    Private() = default;

public:

    bool backupOriginalRawFile  = false;
    bool compressLossLess       = true;
    bool updateFileDate         = false;
    int  previewMode            = DNGWriter::FULL_SIZE;
};

DNGConverterActionThread::DNGConverterActionThread(QObject* const parent)
    : ActionThreadBase(parent),
      d               (new Private)
{
    qRegisterMetaType<DNGConverterActionData>();
}

DNGConverterActionThread::~DNGConverterActionThread()
{
    // cancel the thread

    this->cancel();

    // wait for the thread to finish

    wait();

    delete d;
}

void DNGConverterActionThread::setBackupOriginalRawFile(bool b)
{
    d->backupOriginalRawFile = b;
}

void DNGConverterActionThread::setCompressLossLess(bool b)
{
    d->compressLossLess = b;
}

void DNGConverterActionThread::setUpdateFileDate(bool b)
{
    d->updateFileDate = b;
}

void DNGConverterActionThread::setPreviewMode(int mode)
{
    d->previewMode = mode;
}

void DNGConverterActionThread::processRawFile(const QUrl& url)
{
    QList<QUrl> oneFile;
    oneFile.append(url);
    processRawFiles(oneFile);
}

void DNGConverterActionThread::identifyRawFile(const QUrl& url)
{
    QList<QUrl> oneFile;
    oneFile.append(url);
    identifyRawFiles(oneFile);
}

void DNGConverterActionThread::identifyRawFiles(const QList<QUrl>& urlList)
{
    ActionJobCollection collection;

    for (QList<QUrl>::const_iterator it = urlList.constBegin() ; it != urlList.constEnd() ; ++it)
    {
        DNGConverterTask* const t = new DNGConverterTask(this, *it, IDENTIFY);
        t->setBackupOriginalRawFile(d->backupOriginalRawFile);
        t->setCompressLossLess(d->compressLossLess);
        t->setUpdateFileDate(d->updateFileDate);
        t->setPreviewMode(d->previewMode);

        connect(t, SIGNAL(signalStarting(DigikamGenericDNGConverterPlugin::DNGConverterActionData)),
                this, SIGNAL(signalStarting(DigikamGenericDNGConverterPlugin::DNGConverterActionData)));

        connect(t, SIGNAL(signalFinished(DigikamGenericDNGConverterPlugin::DNGConverterActionData)),
                this, SIGNAL(signalFinished(DigikamGenericDNGConverterPlugin::DNGConverterActionData)));

        connect(this, SIGNAL(signalCancelDNGConverterTask()),
                t, SLOT(slotCancel()), Qt::QueuedConnection);

        collection.insert(t, 0);
    }

    appendJobs(collection);
}

void DNGConverterActionThread::processRawFiles(const QList<QUrl>& urlList)
{
    ActionJobCollection collection;

    for (QList<QUrl>::const_iterator it = urlList.constBegin() ; it != urlList.constEnd() ; ++it)
    {
        DNGConverterTask* const t = new DNGConverterTask(this, *it, PROCESS);
        t->setBackupOriginalRawFile(d->backupOriginalRawFile);
        t->setCompressLossLess(d->compressLossLess);
        t->setUpdateFileDate(d->updateFileDate);
        t->setPreviewMode(d->previewMode);

        connect(t, SIGNAL(signalStarting(DigikamGenericDNGConverterPlugin::DNGConverterActionData)),
                this, SIGNAL(signalStarting(DigikamGenericDNGConverterPlugin::DNGConverterActionData)));

        connect(t, SIGNAL(signalFinished(DigikamGenericDNGConverterPlugin::DNGConverterActionData)),
                this, SIGNAL(signalFinished(DigikamGenericDNGConverterPlugin::DNGConverterActionData)));

        connect(this, SIGNAL(signalCancelDNGConverterTask()),
                t, SLOT(slotCancel()), Qt::QueuedConnection);

        collection.insert(t, 0);
    }

    appendJobs(collection);
}

void DNGConverterActionThread::cancel()
{
    if (isRunning())
    {
        Q_EMIT signalCancelDNGConverterTask();
    }

    ActionThreadBase::cancel();
}

}  // namespace DigikamGenericDNGConverterPlugin

#include "moc_dngconverterthread.cpp"
