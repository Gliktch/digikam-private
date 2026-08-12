/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow frames from images.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "vidslidetask.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QDir>
#include <QString>
#include <QStringList>
#include <QImage>
#include <QSize>
#include <QPainter>
#include <QFileInfo>
#include <QFile>
#include <QIODevice>
#include <QDateTime>
#include <QTextStream>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "frameutils.h"
#include "frameosd.h"
#include "dfileoperations.h"
#include "transitionmngr.h"
#include "effectmngr.h"

namespace Digikam
{

VidSlideTask::VidSlideTask(VidSlideSettings* const settings)
    : ActionJob()
{
    m_settings = settings;
}

VidSlideTask::~VidSlideTask()
{
    cancel();
}

void VidSlideTask::run()
{
    ActionJob::run();       // To customize thread name

    FrameOsd osd;

    int frameId           = 1;
    m_settings->filesList = m_settings->tempDir + QLatin1String("fileslist.txt");
    QFile fList(m_settings->filesList);

    if (!fList.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot create files list:" << m_settings->filesList;
    }

    QTextStream out(&fList);

    // ---------------------------------------------
    // Setup output video file

    QUrl dest       = QUrl::fromLocalFile(m_settings->outputDir);
    dest            = dest.adjusted(QUrl::StripTrailingSlash);
    dest.setPath(dest.path() + QLatin1Char('/') + QLatin1String("videoslideshow.") + m_settings->videoFormat());
    QString outFile = dest.toLocalFile();
    QFileInfo fi(outFile);

    if (fi.exists() && (m_settings->conflictRule != FileSaveConflictBox::OVERWRITE))
    {
        outFile = DFileOperations::getUniqueFileUrl(dest).toLocalFile();
    }

    QImage qiimg;
    QSize osize = m_settings->videoSize();

    // --------------------------------------------------------------
    // Loop to encode frames with images list as temporary JPEG files

    TransitionMngr transmngr;
    transmngr.setOutputSize(osize);

    EffectMngr effmngr;
    effmngr.setOutputSize(osize);
    effmngr.setFrames(m_settings->imgFrames);

    for (int i = 0 ; ((i < m_settings->inputImages.count() + 1) && !m_cancel) ; ++i)
    {
        if (i == 0)
        {
            qiimg = FrameUtils::makeFramedImage(QString(), osize);
        }

        QString ofile;

        if (i < m_settings->inputImages.count())
        {
            ofile = m_settings->inputImages[i].toLocalFile();
        }

        QImage qoimg = FrameUtils::makeFramedImage(ofile, osize);

        // -- Transition encoding ----------

        transmngr.setInImage(qiimg);
        transmngr.setOutImage(qoimg);
        transmngr.setTransition(m_settings->transition);

        int ttmout = 0;
        QImage frame;
        QString framePath;

        do
        {
            frame     = QImage(transmngr.currentFrame(ttmout));
            framePath = m_settings->tempDir + QString::fromLatin1("frame_%1").arg(frameId, 9, 10, QLatin1Char('0')) + QLatin1String(".jpg");

            if (!frame.save(framePath, "JPEG"))
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot generate frame:" << framePath;
            }
            else
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Frame generated:" << framePath;
            }

            out << QString::fromUtf8("file '%1'").arg(QFileInfo(framePath).fileName()) << Qt::endl;
            ++frameId;
        }
        while ((ttmout != -1) && !m_cancel);

        // -- Images encoding ----------

        if (i < m_settings->inputImages.count())
        {
            int count  = 0;
            int itmout = 0;
            effmngr.setImage(qoimg);
            effmngr.setEffect(m_settings->vEffect);

            do
            {
                qiimg     = effmngr.currentFrame(itmout);

                if ((osize.width() >= 1024) && (osize.height() >= 576))
                {
                    osd.insertOsdToFrame(qiimg,
                                         QUrl::fromLocalFile(ofile),
                                         m_settings->osdSettings,
                                         m_settings->iface);
                }

                frame     = qiimg;
                framePath = m_settings->tempDir + QString::fromLatin1("frame_%1").arg(frameId, 9, 10, QLatin1Char('0')) + QLatin1String(".jpg");

                if (!frame.save(framePath, "JPEG"))
                {
                    qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot generate frame:" << framePath;
                }
                else
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << "Frame generated:" << ":" << framePath;
                }

                out << QString::fromUtf8("file '%1'").arg(QFileInfo(framePath).fileName()) << Qt::endl;
                ++frameId;
                ++count;
            }
            while ((count < m_settings->imgFrames) && !m_cancel);
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "Generating frames from image" << i << "done";

        if (!ofile.isEmpty())
        {
            Q_EMIT signalMessage(i18n("Generating frames from %1 Done", ofile), false);
        }

        Q_EMIT signalProgress(i);
    }

    fList.close();

    m_settings->outputFile = outFile;

    Q_EMIT signalComplete(!m_cancel);
}

} // namespace Digikam
