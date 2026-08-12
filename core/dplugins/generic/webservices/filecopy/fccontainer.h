/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-07-05
 * Description : File copy settings container.
 *
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>

// Local includes

#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericFileCopyPlugin
{

class FCContainer
{

public:

    enum FileCopyType
    {
        CopyFile = 0,
        FullSymLink,
        RelativeSymLink
    };

    enum ImageFormat
    {
        JPEG = 0,
        PNG
    };

public:

    FCContainer()  = default;
    ~FCContainer() = default;

public:

    DInfoInterface* iface                   = nullptr;

    QUrl            destUrl;

    int             behavior                = CopyFile;
    int             imageFormat             = JPEG;
    int             imageResize             = 1024;
    int             imageCompression        = 75;

    bool            sidecars                = false;
    bool            writeMetadataToFile     = false;
    bool            overwrite               = false;
    bool            albumPath               = false;
    bool            removeMetadata          = false;
    bool            changeImageProperties   = false;
};

} // namespace DigikamGenericFileCopyPlugin
