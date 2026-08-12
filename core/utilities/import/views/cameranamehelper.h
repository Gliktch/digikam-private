/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-01
 * Description : camera name helper class
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT CameraNameHelper
{

public:

    static QString cameraName(const QString& name);

    static QString cameraNameAutoDetected(const QString& name);

    static QString createCameraName(const QString& vendor,
                                    const QString& product      = QString(),
                                    const QString& mode         = QString(),
                                    bool           autoDetected = false);

    static bool sameDevices(const QString& deviceA, const QString& deviceB);

private:

    enum Token
    {
        VendorAndProduct = 1,
        Mode
    };

private:

    static QString extractCameraNameToken(const QString& cameraName, Token tokenID);
    static QString parseAndFormatCameraName(const QString& cameraName, bool parseMode, bool autoDetected);
};

} // namespace Digikam
