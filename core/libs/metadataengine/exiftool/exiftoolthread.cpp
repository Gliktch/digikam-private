/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-05-13
 * Description : ExifTool thread for the ExifTool process
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2022      by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "exiftoolthread.h"

// Local includes

#include "digikam_debug.h"
#include "exiftoolprocess.h"
#include "actionthreadbase.h"

namespace Digikam
{

ExifToolThread::ExifToolThread(QObject* const parent)
    : QThread(nullptr)
{
    connect(parent, &QObject::destroyed,
            this, [this](QObject*)
        {
            quit();
            wait();

            delete ExifToolProcess::internalPtr;
            delete this;
        }
    );
}

void ExifToolThread::run()
{
    ActionThreadBase::setCurrentThreadName(QLatin1String("ExifToolThread"));       // To customize thread name

    ExifToolProcess* const proc = ExifToolProcess::instance();
    proc->moveToThread(this);
    proc->initExifTool();

    Q_EMIT exifToolProcessStarted();

    exec();

    if (ExifToolProcess::isCreated())
    {
        ExifToolProcess::instance()->shutDownExifTool();
    }
}

} // namespace Digikam

#include "moc_exiftoolthread.cpp"
