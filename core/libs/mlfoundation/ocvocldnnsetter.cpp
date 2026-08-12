/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-25-04
 * Description : Enable/Disable OpenCL in OpenCV when not compatible with DNN
 *
 * SPDX-FileCopyrightText: 2025 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ocvocldnnsetter.h"

// Qt includes

#include <QApplication>
#include <QMutex>
#include <QMutexLocker>

// Local includes

#include "digikam_debug.h"
#include "systemsettings.h"
#include "digikam_opencv.h"

namespace Digikam
{

class Q_DECL_HIDDEN OpenCVOpenCLDNNSetter::Private
{
public:

    QMutex   openCLMutex;
    int      openCLRefCount         = 0;
    bool     dnnOpenCL              = false;
    bool     globalOpenCL           = false;
    int      ref                    = 0;

public:

    Private()
    {
        SystemSettings system = SystemSettings(qApp->applicationName());
        globalOpenCL          = system.enableOpenCL;

        dnnOpenCL             = (system.enableOpenCL &&
                                 system.enableDnnOpenCL);
    }
};

OpenCVOpenCLDNNSetter::Private* OpenCVOpenCLDNNSetter::d = nullptr;

// ------------------------------------------------------------------------

OpenCVOpenCLDNNSetter::OpenCVOpenCLDNNSetter()
{
    if (!d)
    {
        d = new Private;
    }

    ++(d->ref);

    relockOpenCL();
}

OpenCVOpenCLDNNSetter::~OpenCVOpenCLDNNSetter()
{
    unlockOpenCL();

    if (d)
    {
        --(d->ref);
/*
        // don't delete d. We need to keep it around for the next instance

        if (0 == d->ref)
        {
            delete d;
            d = nullptr;
        }
*/
    }
}

void OpenCVOpenCLDNNSetter::unlockOpenCL()
{
    // call setUseOpenCL only if the global and DNN settings are different

    if (locked && (d->dnnOpenCL != d->globalOpenCL))
    {
        QMutexLocker lock(&d->openCLMutex);

        --d->openCLRefCount;

        if (0 == d->openCLRefCount)
        {
            cv::ocl::setUseOpenCL(d->globalOpenCL);
        }

        locked = false;
    }
}

void OpenCVOpenCLDNNSetter::relockOpenCL()
{
    // call setUseOpenCL only if the global and DNN settings are different

    if (!locked && (d->dnnOpenCL != d->globalOpenCL))
    {
        QMutexLocker lock(&d->openCLMutex);

        if (0 == d->openCLRefCount)
        {
            cv::ocl::setUseOpenCL(d->dnnOpenCL);
        }

        ++d->openCLRefCount;

        locked = true;
    }
}

} // namespace Digikam
