/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2000-06-08
 * Description : Core database <-> thumbnail database interface
 *
 * SPDX-FileCopyrightText: 2009      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "thumbnailinfo.h"
#include "loadsavethread.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ThumbsDbInfoProvider : public ThumbnailInfoProvider
{
public:

    ThumbnailInfo thumbnailInfo(const ThumbnailIdentifier& identifier) override;
};

// ----------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT DatabaseLoadSaveFileInfoProvider : public LoadSaveFileInfoProvider
{
public:

    int   orientationHint(const QString& path) override;
    QSize dimensionsHint(const QString& path)  override;
};

} // namespace Digikam
