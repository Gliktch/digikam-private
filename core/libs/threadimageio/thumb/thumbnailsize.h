/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-09-07
 * Description : thumbnails size interface
 *
 * SPDX-FileCopyrightText: 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT ThumbnailSize
{

public:

    enum Size
    {
        Step        = 8,
        Tiny        = 32,
        VerySmall   = 64,
        MediumSmall = 80,
        /**
         * Most usable small size of thumbnails to prevent overloaded
         * overlays show under thumbs (as Pick label and Group indicator)
         * See bugs #321337 and #275381 for details.
         */
        Small       = 100,
        Medium      = 142,
        Large       = 160,
        Huge        = 256,
        HD          = 512,
        MAX         = 1024
    };

public:

    ThumbnailSize()  = default;
    explicit ThumbnailSize(int size);
    ThumbnailSize(const ThumbnailSize& thumbsize);

    ~ThumbnailSize() = default;

    ThumbnailSize& operator=(const ThumbnailSize& thumbsize);
    bool operator==(const ThumbnailSize& thumbsize) const;
    bool operator!=(const ThumbnailSize& thumbsize) const;

    int size()                                      const;

public:

    static void setUseLargeThumbs(bool val);
    static bool getUseLargeThumbs();
    static void readSettings(const KConfigGroup& group);
    static void saveSettings(KConfigGroup& group, bool val);
    static int  maxThumbsSize();

private:

    int  m_Size = Medium;
};

} // namespace Digikam
