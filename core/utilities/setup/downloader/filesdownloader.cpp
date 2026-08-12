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

#include "filesdownloader_p.h"

namespace Digikam
{

FilesDownloader::FilesDownloader(QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    createDownloadInfo();
}

FilesDownloader::~FilesDownloader()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    delete d;
}

} // namespace Digikam

#include "moc_filesdownloader.cpp"
