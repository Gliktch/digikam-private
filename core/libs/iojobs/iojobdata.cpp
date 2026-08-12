/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-02-24
 * Description : Container for IOJob data.
 *
 * SPDX-FileCopyrightText: 2018 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iojobdata.h"

// Qt includes

#include <QRecursiveMutex>
#include <QMutexLocker>

// Local includes

#include "album.h"
#include "iteminfo.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN IOJobData::Private
{
public:

    Private() = default;

    int                operation        = Unknown;
    int                fileConflict     = Continue;

    bool               errorOrCancel    = false;

    PAlbum*            srcAlbum         = nullptr;
    PAlbum*            destAlbum        = nullptr;

    DTrashItemInfoList trashItemList;
    QMap<QUrl, QUrl>   changeDestMap;
    QList<ItemInfo>    itemInfosList;
    QList<QUrl>        sourceUrlList;
    QList<int>         sourceAlbumIds;

    QUrl               destUrl;

    QString            progressId;
    QDateTime          jobTime          = QDateTime::currentDateTime();

    QRecursiveMutex    mutex;
};

IOJobData::IOJobData(int operation,
                     const QList<ItemInfo>& infos,
                     PAlbum* const dest)
    : d(new Private)
{
    d->operation = operation;
    d->destAlbum = dest;

    setItemInfos(infos);

    if (d->destAlbum)
    {
        d->destUrl = d->destAlbum->fileUrl();
    }
}

IOJobData::IOJobData(int operation,
                     const QList<ItemInfo>& infos,
                     const QUrl& dest)
    : d(new Private)
{
    d->operation = operation;
    d->destUrl   = dest;

    setItemInfos(infos);
}

IOJobData::IOJobData(int operation,
                     const QList<QUrl>& urls,
                     PAlbum* const dest)
    : d(new Private)
{
    d->operation     = operation;
    d->sourceUrlList = urls;
    d->destAlbum     = dest;

    if (d->destAlbum)
    {
        d->destUrl = d->destAlbum->fileUrl();
    }
}

IOJobData::IOJobData(int operation,
                     PAlbum* const src,
                     PAlbum* const dest)
    : d(new Private)
{
    d->operation = operation;
    d->srcAlbum  = src;
    d->destAlbum = dest;

    if (d->srcAlbum)
    {
        d->sourceUrlList << d->srcAlbum->fileUrl();
    }

    if (d->destAlbum)
    {
        d->destUrl = d->destAlbum->fileUrl();
    }
}

IOJobData::IOJobData(int operation,
                     const QList<QUrl>& urls,
                     const QUrl& dest)
    : d(new Private)
{
    d->operation     = operation;
    d->sourceUrlList = urls;
    d->destUrl       = dest;
}

IOJobData::IOJobData(int operation,
                     const ItemInfo& info,
                     const QString& newName,
                     bool overwrite)
    : d(new Private)
{
    d->operation = operation;

    if (overwrite)
    {
        d->fileConflict = Overwrite;
    }

    setItemInfos(QList<ItemInfo>() << info);

    d->destUrl = info.fileUrl().adjusted(QUrl::RemoveFilename);
    d->destUrl.setPath(d->destUrl.path() + newName);
}

IOJobData::IOJobData(int operation,
                     const DTrashItemInfoList& infos)
    : d(new Private)
{
    d->operation     = operation;
    d->trashItemList = infos;

    // We need source URLs as dummy.

    for (const DTrashItemInfo& item : std::as_const(d->trashItemList))
    {
        d->sourceUrlList << QUrl::fromLocalFile(item.trashPath);
    }
}

IOJobData::~IOJobData()
{
    delete d;
}

void IOJobData::setItemInfos(const QList<ItemInfo>& infos)
{
    d->itemInfosList = infos;

    d->sourceUrlList.clear();
    d->sourceAlbumIds.clear();

    for (const ItemInfo& info : std::as_const(d->itemInfosList))
    {
        d->sourceUrlList << info.fileUrl();

        if (!d->sourceAlbumIds.contains(info.albumId()))
        {
            d->sourceAlbumIds << info.albumId();
        }
    }
}

void IOJobData::setSourceUrls(const QList<QUrl>& urls)
{
    d->sourceUrlList = urls;
}

void IOJobData::setDestUrl(const QUrl& srcUrl,
                           const QUrl& destUrl)
{
    QMutexLocker locker(&d->mutex);

    d->changeDestMap.insert(srcUrl, destUrl);
}

void IOJobData::setProgressId(const QString& id)
{
    d->progressId = id;
}

void IOJobData::setFileConflict(int fc)
{
    d->fileConflict = fc;
}

void IOJobData::setErrorOrCancel(bool err)
{
    d->errorOrCancel |= err;
}

int IOJobData::operation() const
{
    return d->operation;
}

int IOJobData::fileConflict() const
{
    return d->fileConflict;
}

bool IOJobData::errorOrCancel() const
{
    return d->errorOrCancel;
}

PAlbum* IOJobData::srcAlbum() const
{
    return d->srcAlbum;
}

PAlbum* IOJobData::destAlbum() const
{
    return d->destAlbum;
}

QUrl IOJobData::destUrl(const QUrl& srcUrl) const
{
    QMutexLocker locker(&d->mutex);

    if (srcUrl.isEmpty())
    {
        return d->destUrl;
    }

    return d->changeDestMap.value(srcUrl, d->destUrl);
}

QUrl IOJobData::getNextUrl() const
{
    QMutexLocker locker(&d->mutex);

    QUrl url;

    if (!d->sourceUrlList.isEmpty())
    {
        url = d->sourceUrlList.takeFirst();
    }

    return url;
}

QString IOJobData::destName(const QUrl& srcUrl) const
{
    QMutexLocker locker(&d->mutex);

    QUrl url = srcUrl.adjusted(QUrl::StripTrailingSlash);
    url      = d->changeDestMap.value(srcUrl, url);

    return url.fileName();
}

QString IOJobData::getProgressId() const
{
    return d->progressId;
}

QDateTime IOJobData::jobTime() const
{
    return d->jobTime;
}

ItemInfo IOJobData::findItemInfo(const QUrl& url) const
{
    for (const ItemInfo& info : std::as_const(d->itemInfosList))
    {
        if (info.fileUrl() == url)
        {    // cppcheck-suppress useStlAlgorithm
            return info;
        }
    }

    return ItemInfo();
}

QList<int> IOJobData::srcAlbumIds() const
{
    return d->sourceAlbumIds;
}

QList<QUrl> IOJobData::sourceUrls() const
{
    return d->sourceUrlList;
}

QList<ItemInfo> IOJobData::itemInfos() const
{
    return d->itemInfosList;
}

DTrashItemInfoList IOJobData::trashItems() const
{
    return d->trashItemList;
}

} // namespace Digikam
