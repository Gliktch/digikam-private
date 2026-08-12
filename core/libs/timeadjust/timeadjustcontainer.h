/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-04-19
 * Description : time adjust settings container.
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QMap>
#include <QDateTime>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/**
 * Container that store all timestamp adjustments.
 */
class DIGIKAM_EXPORT TimeAdjustContainer
{

public:

    enum UseDateSource
    {
        APPDATE = 0,
        FILENAME,
        FILEDATE,
        METADATADATE,
        CUSTOMDATE
    };

    enum UseMetaDateType
    {
        EXIFIPTCXMP = 0,
        EXIFCREATED,
        EXIFORIGINAL,
        EXIFDIGITIZED,
        IPTCCREATED,
        XMPCREATED,
        FUZZYCREATED,
        FUZZYORIGINAL,
        FUZZYDIGITIZED
    };

    enum UseFileDateType
    {
        FILELASTMOD = 0,
        FILECREATED
    };

    enum AdjType
    {
        COPYVALUE = 0,
        ADDVALUE,
        SUBVALUE,
        INTERVAL
    };

public:

    TimeAdjustContainer()  = default;
    ~TimeAdjustContainer() = default;

    /**
     * Check if at least one option is selected
     */
    bool atLeastOneUpdateToProcess()                                const;

    QDateTime calculateAdjustedDate(const QDateTime& originalTime, int index = 0);
    QDateTime getDateTimeFromString(const QString& dateStr)         const;
    QMap<QString, bool> getDateTimeTagsMap()                        const;

public:

    QDateTime customDate        = QDateTime::currentDateTime();
    QDateTime customTime        = QDateTime::currentDateTime();
    QDateTime adjustmentTime    = QDateTime();

    bool      updUseExifTool    = false;
    bool      updIfAvailable    = true;
    bool      updEXIFModDate    = false;
    bool      updEXIFOriDate    = false;
    bool      updEXIFDigDate    = false;
    bool      updEXIFThmDate    = false;
    bool      updIPTCDate       = false;
    bool      updXMPVideo       = false;
    bool      updXMPDate        = false;
    bool      updFileModDate    = false;

    /**
     * Only a temporary variable, will not be saved
     */
    bool      enableExifTool    = false;

    int       dateSource        = APPDATE;
    int       metadataSource    = EXIFIPTCXMP;
    int       fileDateSource    = FILELASTMOD;
    int       adjustmentType    = COPYVALUE;
    int       adjustmentDays    = 0;
};

// -------------------------------------------------------------------

/**
 * Container that hold the time difference for clock photo dialog.
 */
class DeltaTime
{

public:

    DeltaTime()  = default;
    ~DeltaTime() = default;

    /**
     * Check if at least one option is selected.
     */
    bool isNull() const
    {
        return (
                (deltaDays    == 0) &&
                (deltaHours   == 0) &&
                (deltaMinutes == 0) &&
                (deltaSeconds == 0)
               );
    }

public:

    bool deltaNegative  = false;

    int  deltaDays      = 0;
    int  deltaHours     = 0;
    int  deltaMinutes   = 0;
    int  deltaSeconds   = 0;
};

} // namespace Digikam
