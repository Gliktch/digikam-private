/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-25
 * Description : a tool to convert RAW file to DNG
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Jens Mueller <tschenser at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "drawdecoder.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DNGWriter
{

public:

    enum JPEGPreview
    {
        NONE = 0,                           ///< No preview will be generated.
        MEDIUM,                             ///< A medium size preview will be generated.
        FULL_SIZE                           ///< A full size preview  will be generated.
    };

    enum ConvertError
    {
        PROCESS_CONTINUE       =  1,        ///< Current stages is done.
        PROCESS_COMPLETE       =  0,        ///< All stages done.
        PROCESS_FAILED         = -1,        ///< A failure happen while processing.
        PROCESS_CANCELED       = -2,        ///< User has canceled processing.
        FILE_NOT_SUPPORTED     = -3,        ///< Raw file format is not supported by converter.
        DNG_SDK_INTERNAL_ERROR = -4         ///< Adobe DNG SDK has generated an error while processing.
    };

public:

    explicit DNGWriter();
    ~DNGWriter();

    void setInputFile(const QString& filePath);
    void setOutputFile(const QString& filePath);

    QString inputFile()             const;
    QString outputFile()            const;

    void setCompressLossLess(bool b);
    bool compressLossLess()         const;

    void setUpdateFileDate(bool b);
    bool updateFileDate()           const;

    void setBackupOriginalRawFile(bool b);
    bool backupOriginalRawFile()    const;

    void setPreviewMode(int mode);
    int  previewMode()              const;

    int  convert();
    void cancel();
    void reset();

public:

    static QString xmpSdkVersion();
    static QString dngSdkVersion();

public:

    // Declared public for DNGWriterHost class.
    class Private;

private:

    /// @note disabled
    DNGWriter(const DNGWriter&)            = delete;
    DNGWriter& operator=(const DNGWriter&) = delete;

private:

    Private* const d = nullptr;
};

} // namespace Digikam
