/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-09
 * Description : resize image threads manager.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Andi Clemens <andi dot clemens at googlemail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imageresizethread.h"

// Qt includes

#include <QFileInfo>
#include <QTemporaryDir>

// Local includes

#include "digikam_debug.h"
#include "imageresizejob.h"

namespace DigikamGenericSendByMailPlugin
{

ImageResizeThread::ImageResizeThread(QObject* const parent)
    : ActionThreadBase(parent)
{
    setObjectName(QLatin1String("ImageResizeThread"));
    m_count  = new int;
    *m_count = 0;
}

ImageResizeThread::~ImageResizeThread()
{
    delete m_count;
}

void ImageResizeThread::resize(MailSettings* const settings)
{
    ActionJobCollection collection;
    *m_count = 0;

    for (QMap<QUrl, QUrl>::const_iterator it = settings->itemsList.constBegin() ;
         it != settings->itemsList.constEnd() ; ++it)
    {
        ImageResizeJob* const t = new ImageResizeJob(m_count);
        t->m_orgUrl             = it.key();
        t->m_settings           = settings;

        QTemporaryDir tmpDir(t->m_settings->tempPath);
        tmpDir.setAutoRemove(false);

        QFileInfo fi(t->m_orgUrl.fileName());
        t->m_destName = tmpDir.path() + QLatin1Char('/') +
                        QString::fromUtf8("%1.%2").arg(fi.baseName()).arg(t->m_settings->format().toLower());

        connect(t, SIGNAL(startingResize(QUrl)),
                this, SIGNAL(startingResize(QUrl)));

        connect(t, SIGNAL(finishedResize(QUrl,QUrl,int)),
                this, SIGNAL(finishedResize(QUrl,QUrl,int)));

        connect(t, SIGNAL(failedResize(QUrl,QString,int)),
                this, SIGNAL(failedResize(QUrl,QString,int)));

        collection.insert(t, 0);
    }

    appendJobs(collection);
}

void ImageResizeThread::cancel()
{
    *m_count = 0;
    ActionThreadBase::cancel();
}

} // namespace DigikamGenericSendByMailPlugin

#include "moc_imageresizethread.cpp"
