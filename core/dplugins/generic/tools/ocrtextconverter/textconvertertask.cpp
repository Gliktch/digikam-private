/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-26
 * Description : Text Converter thread
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2022      by Quoc Hung Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "textconvertertask.h"

// Qt includes

#include <QPointer>

// Local includes

#include "digikam_debug.h"
#include "ocrtesseractengine.h"

using namespace Digikam;

namespace DigikamGenericTextConverterPlugin
{

class Q_DECL_HIDDEN TextConverterTask::Private
{
public:

    Private() = default;

public:

    OcrOptions                   opt;

    QUrl                         url;
    TextConverterAction          action = TextConverterAction();

    QPointer<OcrTesseractEngine> ocrEngine;
};

TextConverterTask::TextConverterTask(QObject* const parent,
                                     const QUrl& fileUrl,
                                     const TextConverterAction& action)
    : ActionJob(parent),
      d        (new Private)
{
    d->url    = fileUrl;
    d->action = action;
}

TextConverterTask::~TextConverterTask()
{
    slotCancel();

    delete d->ocrEngine;
    delete d;
}

void TextConverterTask::setOcrOptions(const OcrOptions& opt)
{
    d->opt = opt;
}

OcrOptions TextConverterTask::ocrOptions() const
{
    return d->opt;
}

void TextConverterTask::run()
{
    ActionJob::run();       // To customize thread name

    if (m_cancel)
    {
        return;
    }

    switch (d->action)
    {
        case PROCESS:
        {
            TextConverterActionData ad1;
            ad1.action     = PROCESS;
            ad1.fileUrl    = d->url;
            ad1.starting   = true;

            Q_EMIT signalStarting(ad1);

            d->ocrEngine   = new OcrTesseractEngine;
            d->ocrEngine->setInputFile(d->url.toLocalFile());
            d->ocrEngine->setOcrOptions(d->opt);
            int ret        = d->ocrEngine->runOcrProcess();

            TextConverterActionData ad2;
            ad2.action     = PROCESS;
            ad2.fileUrl    = d->url;
            ad2.destPath   = d->ocrEngine->outputFile();
            ad2.result     = ret;
            ad2.outputText = d->ocrEngine->outputText();

            Q_EMIT signalFinished(ad2);

            break;
        }

        default:
        {
            qCritical(DIGIKAM_GENERAL_LOG) << "Unknown action specified";
            break;
        }
    }

    Q_EMIT signalDone();
}

void TextConverterTask::slotCancel()
{
    if (d->ocrEngine)
    {
        d->ocrEngine->cancelOcrProcess();
    }
}

} // namespace DigikamGenericTextConverterPlugin

#include "moc_textconvertertask.cpp"
