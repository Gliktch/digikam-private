/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-21
 * Description : Queue common settings container.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>

// Local includes

#include "iofilesettings.h"
#include "drawdecodersettings.h"
#include "filesaveconflictbox.h"

namespace Digikam
{

/**
 * This container host all common settings used by a queue, not including assigned batch tools
 */
class QueueSettings
{

public:

    enum RenamingRule
    {
        USEORIGINAL = 0,
        CUSTOMIZE
    };

    enum RawLoadingRule
    {
        USEEMBEDEDJPEG = 0,
        DEMOSAICING
    };

public:

    QueueSettings();
    ~QueueSettings() = default;

public:

    bool                              useMultiCoreCPU       = false;

    bool                              saveAsNewVersion      = true;

    /// Setting managed through Metadata control panel.
    bool                              exifSetOrientation    = true;

    /// If true, original file dir will be used to process queue items.
    bool                              useOrgAlbum           = true;

    QString                           renamingParser;

    QUrl                              workingUrl;

    FileSaveConflictBox::ConflictRule conflictRule          = FileSaveConflictBox::DIFFNAME;
    RenamingRule                      renamingRule          = USEORIGINAL;
    RawLoadingRule                    rawLoadingRule        = DEMOSAICING;

    DRawDecoderSettings               rawDecodingSettings;

    IOFileSettings                    ioFileSettings;
};

} // namespace Digikam
