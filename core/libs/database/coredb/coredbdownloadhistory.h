/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-01
 * Description : Core database interface to manage camera item download history.
 *
 * SPDX-FileCopyrightText: 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QDateTime>

// Local includes

#include "digikam_export.h"
#include "camiteminfo.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT CoreDbDownloadHistory
{
public:

    /**
     * Queries the status of a download item that is uniquely described by the four parameters.
     * The identifier is recommended to be an MD5 hash of properties describing the camera,
     * if available, and the directory path (though you are free to use all four parameters as you want)
     */
    static CamItemInfo::DownloadStatus status(const QString& identifier, const QString& name,
                                              qlonglong fileSize, const QDateTime& date);

    /**
     * Sets the status of the item to Downloaded
     */
    static void setDownloaded(const QString& identifier, const QString& name,
                              qlonglong fileSize, const QDateTime& date);
};

} // namespace Digikam
