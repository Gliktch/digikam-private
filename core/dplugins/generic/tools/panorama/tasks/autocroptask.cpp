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

#include "autocroptask.h"

// Qt includes

#include <QFile>

namespace DigikamGenericPanoramaPlugin
{

AutoCropTask::AutoCropTask(const QString& workDirPath,
                           const QUrl& autoOptimiserPtoUrl,
                           QUrl& viewCropPtoUrl,
                           bool /*buildGPano*/,
                           const QString& panoModifyPath)
    : CommandTask(PANO_AUTOCROP, workDirPath, panoModifyPath),
      autoOptimiserPtoUrl(autoOptimiserPtoUrl),
      viewCropPtoUrl(viewCropPtoUrl)
/*    , buildGPano(buildGPano)
*/
{
}

void AutoCropTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    viewCropPtoUrl = tmpDir;
    viewCropPtoUrl.setPath(viewCropPtoUrl.path() + QLatin1String("view_crop_pano.pto"));

    QStringList args;
    args << QLatin1String("-c");               // Center the panorama
    args << QLatin1String("-s");               // Straighten the panorama
    args << QLatin1String("--canvas=AUTO");    // Automatic size
    args << QLatin1String("--crop=AUTO");      // Automatic crop
    args << QLatin1String("-o");
    args << viewCropPtoUrl.toLocalFile();
    args << autoOptimiserPtoUrl.toLocalFile();

    runProcess(args);

    // PanoModify does not return an error code when something went wrong...

    QFile ptoOutput(viewCropPtoUrl.toLocalFile());

    if (!ptoOutput.exists())
    {
        successFlag = false;
        errString = getProcessError();
    }

    printDebug(QLatin1String("pano_modify"));
}

} // namespace DigikamGenericPanoramaPlugin
