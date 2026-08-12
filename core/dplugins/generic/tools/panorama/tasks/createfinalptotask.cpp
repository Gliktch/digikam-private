/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-12-05
 * Description : a tool to create panorama by fusion of several images.
 *
 * SPDX-FileCopyrightText: 2012-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "createfinalptotask.h"

// Qt includes

#include <QFile>

// KDE includes

#include <klocalizedstring.h>

namespace DigikamGenericPanoramaPlugin
{

CreateFinalPtoTask::CreateFinalPtoTask(const QString& workDirPath,
                                       QSharedPointer<const PTOType> ptoData,
                                       QUrl& finalPtoUrl,
                                       const QRect& crop)
    : PanoTask   (PANO_CREATEFINALPTO, workDirPath),
      ptoData    (*ptoData),
      finalPtoUrl(finalPtoUrl),
      crop       (crop)
{
}

void CreateFinalPtoTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    finalPtoUrl = tmpDir;
    finalPtoUrl.setPath(finalPtoUrl.path() + QLatin1String("final.pto"));

    QFile pto(finalPtoUrl.toLocalFile());

    if (pto.exists())
    {
        errString   = i18n("PTO file already created in the temporary directory.");
        successFlag = false;
        return;
    }

    if (!pto.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        errString   = i18n("PTO file cannot be created in the temporary directory.");
        successFlag = false;
        return;
    }

    pto.close();

    ptoData.project.crop = crop;
    ptoData.createFile(finalPtoUrl.toLocalFile());
    successFlag          = true;

    return;
}

} // namespace DigikamGenericPanoramaPlugin
