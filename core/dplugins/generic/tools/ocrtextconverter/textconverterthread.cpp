/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-26
 * Description : Text Converter threads manager
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2022      by Quoc Hung Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "textconverterthread.h"

// Local includes

#include "digikam_debug.h"
#include "textconvertertask.h"

using namespace Digikam;

namespace DigikamGenericTextConverterPlugin
{

class Q_DECL_HIDDEN TextConverterActionThread::Private
{
public:

    Private() = default;

public:

    OcrOptions opt;
};

TextConverterActionThread::TextConverterActionThread(QObject* const parent)
    : ActionThreadBase(parent),
      d               (new Private)
{
    qRegisterMetaType<TextConverterActionData>();
}

TextConverterActionThread::~TextConverterActionThread()
{
    // cancel the thread

    this->cancel();

    // wait for the thread to finish

    wait();

    delete d;
}

void TextConverterActionThread::setOcrOptions(const OcrOptions& opt)
{
    d->opt = opt;

    if (!opt.multicores)
    {
        setMaximumNumberOfThreads(1);
    }
}

OcrOptions TextConverterActionThread::ocrOptions() const
{
    return d->opt;
}

void TextConverterActionThread::ocrProcessFile(const QUrl& url)
{
    QList<QUrl> oneFile;
    oneFile.append(url);
    ocrProcessFiles(oneFile);
}

void TextConverterActionThread::ocrProcessFiles(const QList<QUrl>& urlList)
{
    ActionJobCollection collection;

    for (QList<QUrl>::const_iterator it = urlList.constBegin() ; it != urlList.constEnd() ; ++it)
    {
        TextConverterTask* const t = new TextConverterTask(this, *it, PROCESS);
        t->setOcrOptions(d->opt);

        connect(t, SIGNAL(signalStarting(DigikamGenericTextConverterPlugin::TextConverterActionData)),
                this, SIGNAL(signalStarting(DigikamGenericTextConverterPlugin::TextConverterActionData)));

        connect(t, SIGNAL(signalFinished(DigikamGenericTextConverterPlugin::TextConverterActionData)),
                this, SIGNAL(signalFinished(DigikamGenericTextConverterPlugin::TextConverterActionData)));

        connect(this, SIGNAL(signalCancelTextConverterTask()),
                t, SLOT(slotCancel()),
                Qt::QueuedConnection);

        collection.insert(t, 0);
    }

    appendJobs(collection);
}

void TextConverterActionThread::cancel()
{
    if (isRunning())
    {
        Q_EMIT signalCancelTextConverterTask();
    }

    ActionThreadBase::cancel();
}

}  // namespace DigikamGenericTextConverterPlugin

#include "moc_textconverterthread.cpp"
