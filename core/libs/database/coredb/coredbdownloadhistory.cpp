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

#include "coredbdownloadhistory.h"

// Local includes

#include "coredb.h"
#include "coredbaccess.h"

namespace Digikam
{

CamItemInfo::DownloadStatus CoreDbDownloadHistory::status(const QString& identifier, const QString& name,
                                                          qlonglong fileSize, const QDateTime& date)
{
    QList<qint64> seconds;
    seconds << 0 << 3600 << -3600;

    for (const qint64 second : std::as_const(seconds))
    {
        QDateTime dt = date.addSecs(second);

        if (CoreDbAccess().db()->findInDownloadHistory(identifier, name, fileSize, dt) != -1)
        {
            return CamItemInfo::DownloadedYes;
        }
    }

    return CamItemInfo::DownloadedNo;
}

void CoreDbDownloadHistory::setDownloaded(const QString& identifier, const QString& name,
                                          qlonglong fileSize, const QDateTime& date)
{
    CoreDbAccess().db()->addToDownloadHistory(identifier, name, fileSize, date);
}

} // namespace Digikam
