/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-03
 * Description : IO file Settings Container.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "drawdecoding.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT IOFileSettings
{

public:

    IOFileSettings()  = default;
    ~IOFileSettings() = default;

public:

    /// JPEG quality value.
    int          JPEGCompression        = 75;

    /// JPEG chroma sub-sampling value.
    int          JPEGSubSampling        = 1;     ///< Medium sub-sampling

    /// PNG compression value.
    int          PNGCompression         = 9;

    /// TIFF deflate compression.
    bool         TIFFCompression        = false;

    /// JPEG2000 quality value.
    int          JPEG2000Compression    = 75;

    /// JPEG2000 lossless compression.
    bool         JPEG2000LossLess       = true;

    /// PGF quality value.
    int          PGFCompression         = 3;

    /// PGF lossless compression.
    bool         PGFLossLess            = true;

    /// HEIF quality value.
    int          HEIFCompression        = 75;

    /// HEIF lossless compression.
    bool         HEIFLossLess           = true;

    /// JXL quality value.
    int          JXLCompression         = 75;

    /// JXL lossless compression.
    bool         JXLLossLess            = true;

    /// WEBP quality value.
    int          WEBPCompression        = 75;

    /// WEBP lossless compression.
    bool         WEBPLossLess           = true;

    /// AVIF quality value.
    int          AVIFCompression        = 75;

    /// AVIF lossless compression.
    bool         AVIFLossLess           = true;

    /// Use Raw Import tool to load a RAW picture.
    bool         useRAWImport           = true;

    QString rawImportToolIid            = QLatin1String("org.kde.digikam.plugin.rawimport.Native");

    /// RAW File decoding options.
    DRawDecoding rawDecodingSettings;
};

} // namespace Digikam
