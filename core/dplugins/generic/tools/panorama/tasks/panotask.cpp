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

#include "panotask.h"

// Qt includes

#include <QFileInfo>

namespace DigikamGenericPanoramaPlugin
{

PanoTask::PanoTask(PanoAction action, const QString& workDirPath)
    : action        (action),
      isAbortedFlag (false),
      successFlag   (false),
      tmpDir        (QUrl::fromLocalFile(workDirPath + QLatin1Char('/')))
{
}

bool PanoTask::success() const
{
    return successFlag;
}

void PanoTask::requestAbort()
{
    isAbortedFlag = true;
}

} // namespace DigikamGenericPanoramaPlugin
