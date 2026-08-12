/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-11-04
 * Description : interface to hugin_executor
 *
 * SPDX-FileCopyrightText: 2015-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "huginexecutortask.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace DigikamGenericPanoramaPlugin
{

HuginExecutorTask::HuginExecutorTask(const QString& workDirPath,
                                     const QUrl& input,
                                     QUrl& panoUrl,
                                     PanoramaFileType fileType,
                                     const QString& huginExecutorPath,
                                     bool preview)
    : CommandTask(preview ? PANO_HUGINEXECUTORPREVIEW : PANO_HUGINEXECUTOR, workDirPath, huginExecutorPath),
      ptoUrl     (input),
      panoUrl    (panoUrl),
      fileType   (fileType)
{
}

void HuginExecutorTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    panoUrl = tmpDir;
    QFileInfo fi(QDir::toNativeSeparators(ptoUrl.toLocalFile()));

    switch (fileType)
    {
        case JPEG:
        {
            panoUrl.setPath(panoUrl.path() + fi.completeBaseName() + QLatin1String(".jpg"));
            break;
        }

        case TIFF:
        {
            panoUrl.setPath(panoUrl.path() + fi.completeBaseName() + QLatin1String(".tif"));
            break;
        }

        case HDR:
        {
            panoUrl.setPath(panoUrl.path() + fi.completeBaseName() + QLatin1String(".hdr"));
            break;
        }
    }

    QStringList args;
    args << QLatin1String("-s");
    args << QLatin1String("-p");
    args << fi.completeBaseName();
    args << ptoUrl.toLocalFile();

    runProcess(args);

    // HuginExecutor does not return an error code when something went wrong...

    QFile panoOutput(panoUrl.toLocalFile());

    if (!panoOutput.exists())
    {
        successFlag = false;
        errString   = getProcessError();
    }

    printDebug(QLatin1String("hugin_executor"));
}

} // namespace DigikamGenericPanoramaPlugin
