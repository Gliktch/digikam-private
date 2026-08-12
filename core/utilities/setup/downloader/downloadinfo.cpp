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

#include "downloadinfo.h"

namespace Digikam
{

DownloadInfo::DownloadInfo(const QString& _path,
                           const QString& _name,
                           const QString& _hash,
                           const qint64&  _size)
    : path(_path),
      name(_name),
      hash(_hash),
      size(_size)
{
}

DownloadInfo::DownloadInfo(const DownloadInfo& other)
    : path(other.path),
      name(other.name),
      hash(other.hash),
      size(other.size)
{
}

DownloadInfo& DownloadInfo::operator=(const DownloadInfo& other)
{
    path = other.path;
    name = other.name;
    hash = other.hash;
    size = other.size;

    return *this;
}

} // namespace Digikam
