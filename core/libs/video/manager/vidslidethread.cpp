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

#include "vidslidethread.h"

// Qt include

#include <QDir>
#include <QDateTime>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"

namespace Digikam
{

VidSlideThread::VidSlideThread(QObject* const parent)
    : ActionThreadBase(parent)
{
    setObjectName(QLatin1String("VidSlideThread"));
}

VidSlideThread::~VidSlideThread()
{
    cancel();
    wait();

    if (m_settings && QDir().exists(m_settings->tempDir))
    {
        QDir(m_settings->tempDir).removeRecursively();
    }
}

void VidSlideThread::prepareFrames(VidSlideSettings* const settings)
{
    m_settings          = settings;

    m_settings->tempDir = m_settings->outputDir + QLatin1String("/.")         +
                          QString::number(QDateTime::currentSecsSinceEpoch()) +
                          QLatin1Char('/');

    if (!QDir().mkpath(m_settings->tempDir))
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot create temporary directory:" << m_settings->tempDir;

        Q_EMIT signalMessage(i18n("Error while creating temporary directory %1!", m_settings->tempDir), true);

        return;
    }

    ActionJobCollection collection;

    VidSlideTask* const t = new VidSlideTask(settings);

    connect(t, SIGNAL(signalProgress(int)),
            this, SIGNAL(signalProgress(int)));

    connect(t, SIGNAL(signalMessage(QString,bool)),
            this, SIGNAL(signalMessage(QString,bool)));

    connect(t, SIGNAL(signalComplete(bool)),
            this, SLOT(slotEncodeFrames(bool)));

    collection.insert(t, 0);

    appendJobs(collection);
}

void VidSlideThread::slotEncodeFrames(bool prepareDone)
{
    if (!prepareDone)
    {
        Q_EMIT signalComplete(false);

        return;
    }

    Q_EMIT signalMessage(i18n("Encoding frames..."), false);

    if (m_encoder)
    {
        delete m_encoder;
    }

    m_encoder = new FFmpegLauncher(this);
    m_encoder->setSettings(m_settings);

    connect(m_encoder, &ProcessLauncher::signalComplete,
            this, &VidSlideThread::slotEncodeDone,
            Qt::DirectConnection);

    m_encoder->encodeFrames();
}

void VidSlideThread::slotEncodeDone(bool success, int exitCode)
{
    bool b = false;

    if (!success || (exitCode != 0))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot generate output video" << m_settings->outputFile;
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Output video is" << m_settings->outputFile;
        b = true;
    }

    if (b)
    {
        if (QDir().exists(m_settings->tempDir))
        {
            QDir(m_settings->tempDir).removeRecursively();
        }

        Q_EMIT signalMessage(i18n("Encoding done."), false);
    }
    else
    {
        Q_EMIT signalMessage(i18n("Error while encoding frames!"), true);
    }

    Q_EMIT signalComplete(b);
}

QString VidSlideThread::encodingTraces() const
{
    if (m_encoder)
    {
        return m_encoder->output();
    }

    return QString();
}

} // namespace Digikam

#include "moc_vidslidethread.cpp"
