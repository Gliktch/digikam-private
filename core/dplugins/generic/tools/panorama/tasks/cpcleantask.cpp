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

#include "cpcleantask.h"

// Qt includes

#include <QFile>

namespace DigikamGenericPanoramaPlugin
{

CpCleanTask::CpCleanTask(const QString& workDirPath, const QUrl& input,
                         QUrl& cpCleanPtoUrl, const QString& cpCleanPath)
    : CommandTask  (PANO_CPCLEAN, workDirPath, cpCleanPath),
      cpCleanPtoUrl(cpCleanPtoUrl),
      cpFindPtoUrl (input)
{
}

void CpCleanTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    cpCleanPtoUrl = tmpDir;
    cpCleanPtoUrl.setPath(cpCleanPtoUrl.path()+ QLatin1String("cp_pano_clean.pto"));

    QStringList args;
    args << QLatin1String("-o");
    args << cpCleanPtoUrl.toLocalFile();
    args << cpFindPtoUrl.toLocalFile();

    runProcess(args);

    // CPClean does not return an error code when something went wrong...

    QFile ptoOutput(cpCleanPtoUrl.toLocalFile());

    if (!ptoOutput.exists())
    {
        successFlag = false;
        errString   = getProcessError();
    }

    printDebug(QLatin1String("cpclean"));
}

} // namespace DigikamGenericPanoramaPlugin
