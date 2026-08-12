/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-11-14
 * Description : Files downloader
 *
 * SPDX-FileCopyrightText: 2020-2024 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DownloadInfo
{
public:

    DownloadInfo()  = default;
    DownloadInfo(const QString& _path,
                 const QString& _name,
                 const QString& _hash,
                 const qint64&  _size);
    DownloadInfo(const DownloadInfo& other);
    ~DownloadInfo() = default;

    DownloadInfo& operator=(const DownloadInfo& other);

    /**
     * The file path on the server.
     */
    QString path;

    /**
     * The file name on the server.
     */
    QString name;

    /**
     * The file hash as SHA256.
     */
    QString hash;

    /**
     * The file size.
     */
    qint64  size = 0;
};

} // namespace Digikam
