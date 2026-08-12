/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-03-15
 * Description : a tool to create panorama by fusion of several images.
 *
 * SPDX-FileCopyrightText: 2012-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "cpfindtask.h"

// Qt includes

#include <QFile>

namespace DigikamGenericPanoramaPlugin
{

CpFindTask::CpFindTask(const QString& workDirPath,
                       const QUrl& input,
                       QUrl& cpFindUrl,
                       bool celeste,
                       const QString& cpFindPath)
    : CommandTask   (PANO_CPFIND, workDirPath, cpFindPath),
      cpFindPtoUrl  (cpFindUrl),
      celeste       (celeste),
      ptoUrl        (input)
{
}

void CpFindTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    // Run CPFind to get control points and order the images

    cpFindPtoUrl = tmpDir;
    cpFindPtoUrl.setPath(cpFindPtoUrl.path() + QLatin1String("cp_pano.pto"));

    QStringList args;

    if (celeste)
    {
        args << QLatin1String("--celeste");
    }

    args << QLatin1String("-o");
    args << cpFindPtoUrl.toLocalFile();
    args << ptoUrl.toLocalFile();

    runProcess(args);

    // CPFind does not return an error code when something went wrong...

    QFile ptoOutput(cpFindPtoUrl.toLocalFile());

    if (!ptoOutput.exists())
    {
        successFlag = false;
        errString   = getProcessError();
    }

    printDebug(QLatin1String("cpfind"));
}

} // namespace DigikamGenericPanoramaPlugin
