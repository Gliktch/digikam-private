/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-21-07
 * Description : Camera item download settings container.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QDateTime>

// Local includes

#include "digikam_config.h"
#include "digikam_globals.h"

#ifdef HAVE_JXL
#   include "dngwriter.h"
#endif

namespace Digikam
{

class DownloadSettings
{

public:

    DownloadSettings()  = default;
    ~DownloadSettings() = default;

public:

    /// Settings from AdvancedSettings widget
    bool       autoRotate       = true;
    bool       fixDateTime      = false;
    bool       convertJpeg      = false;
    bool       documentName     = false;

    QDateTime  newDateTime;

    /// New format to convert Jpeg files.
    QString    losslessFormat;

    /// Metadata template title.
    QString    templateTitle;

    /// File path to download
    QString    folder;
    QString    file;
    QString    dest;

    /// Mime type from file to download
    QString    mime;

#ifdef HAVE_JXL

    /// Settings from DNG convert widget
    bool       convertDng       = false;
    bool       backupRaw        = false;
    bool       compressDng      = true;
    int        previewMode      = DNGWriter::FULL_SIZE;

#endif

    /// Settings from ScriptingSettings widget
    QString    script;

    /// Pre-rating of each camera file
    int        rating           = NoRating;

    /// Pre-pickLabel of each camera file
    int        pickLabel        = NoPickLabel;

    /// Pre-colorLabel of each camera file
    int        colorLabel       = NoColorLabel;

    /// Pre-tags of each camera file
    QList<int> tagIds;
};

typedef QList<DownloadSettings> DownloadSettingsList;

} // namespace Digikam
