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

#include "optimisationtask.h"

// Qt includes

#include <QFile>

namespace DigikamGenericPanoramaPlugin
{

OptimisationTask::OptimisationTask(const QString& workDirPath, const QUrl& input,
                                   QUrl& autoOptimiserPtoUrl, bool levelHorizon, bool gPano,
                                   const QString& autooptimiserPath)
    : CommandTask        (PANO_OPTIMIZE, workDirPath, autooptimiserPath),
      autoOptimiserPtoUrl(autoOptimiserPtoUrl),
      ptoUrl             (input),
      levelHorizon       (levelHorizon),
      buildGPano         (gPano)
{
}

void OptimisationTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    autoOptimiserPtoUrl = tmpDir;
    autoOptimiserPtoUrl.setPath(autoOptimiserPtoUrl.path() + QLatin1String("auto_op_pano.pto"));

    QStringList args;
    args << QLatin1String("-am");

    if (levelHorizon)
    {
       args << QLatin1String("-l");
    }

    // NOTE: This parameter changes the projection type
    // See also Bug 346053 and Bug 416492

    Q_UNUSED(buildGPano);
/*
    if (buildGPano)
    {
        args << QLatin1String("-s");
    }
*/
    args << QLatin1String("-o");
    args << autoOptimiserPtoUrl.toLocalFile();
    args << ptoUrl.toLocalFile();

    runProcess(args);

    // AutoOptimiser does not return an error code when something went wrong...

    QFile ptoOutput(autoOptimiserPtoUrl.toLocalFile());

    if (!ptoOutput.exists())
    {
        successFlag = false;
        errString = getProcessError();
    }

    printDebug(QLatin1String("autooptimiser"));
}

} // namespace DigikamGenericPanoramaPlugin
