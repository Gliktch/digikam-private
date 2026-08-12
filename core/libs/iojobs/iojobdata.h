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

#pragma once

// Qt includes

#include <QUrl>
#include <QList>
#include <QDateTime>

// Local includes

#include "digikam_export.h"
#include "dtrashiteminfo.h"

namespace Digikam
{

class PAlbum;
class ItemInfo;

class DIGIKAM_GUI_EXPORT IOJobData
{

public:

    enum Operation
    {
        Unknown = 0,
        CopyAlbum,
        CopyImage,
        CopyFiles,
        CopyToExt,
        MoveAlbum,
        MoveImage,
        MoveFiles,
        Restore,
        Rename,
        Delete,
        Trash,
        Empty
    };

    enum FileConflict
    {
        Continue = 0,
        AutoRename,
        Overwrite
    };

public:

    explicit IOJobData(int operation,
                       const QList<ItemInfo>& infos,
                       PAlbum* const dest = nullptr);

    explicit IOJobData(int operation,
                       const QList<ItemInfo>& infos,
                       const QUrl& dest);

    explicit IOJobData(int operation,
                       const QList<QUrl>& urls,
                       PAlbum* const dest = nullptr);

    explicit IOJobData(int operation,
                       PAlbum* const src,
                       PAlbum* const dest = nullptr);

    explicit IOJobData(int operation,
                       const QList<QUrl>& urls,
                       const QUrl& dest);

    explicit IOJobData(int operation,
                       const ItemInfo& info,
                       const QString& newName,
                       bool overwrite = false);

    explicit IOJobData(int operation,
                       const DTrashItemInfoList& infos);

    ~IOJobData();

    void               setItemInfos(const QList<ItemInfo>& infos);
    void               setSourceUrls(const QList<QUrl>& urls);

    void               setDestUrl(const QUrl& srcUrl,
                                  const QUrl& destUrl);

    void               setProgressId(const QString& id);

    void               setFileConflict(int fc);
    void               setErrorOrCancel(bool err);

    int                operation()                          const;

    int                fileConflict()                       const;

    bool               errorOrCancel()                      const;

    PAlbum*            srcAlbum()                           const;
    PAlbum*            destAlbum()                          const;

    QUrl               destUrl(const QUrl& srcUrl = QUrl()) const;
    QUrl               getNextUrl()                         const;

    QString            destName(const QUrl& srcUrl)         const;
    QString            getProgressId()                      const;
    QDateTime          jobTime()                            const;

    ItemInfo           findItemInfo(const QUrl& url)        const;

    QList<int>         srcAlbumIds()                        const;
    QList<QUrl>        sourceUrls()                         const;
    QList<ItemInfo>    itemInfos()                          const;

    DTrashItemInfoList trashItems()                         const;

private:

    /// @note disabled
    IOJobData(const IOJobData&)            = delete;
    IOJobData& operator=(const IOJobData&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
