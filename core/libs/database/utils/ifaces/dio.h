/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-17
 * Description : low level files management interface.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2018      by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMultiHash>
#include <QObject>
#include <QMutex>
#include <QMap>

// Local includes

#include "digikam_export.h"
#include "dtrashiteminfo.h"

class QUrl;

namespace Digikam
{

class Album;
class PAlbum;
class ItemInfo;
class IOJobData;
class ProgressItem;

class DIGIKAM_GUI_EXPORT DIO : public QObject
{
    Q_OBJECT

public:

    DIO();
    ~DIO() override = default;

    static DIO* instance();

    static void cleanUp();

    static bool itemsUnderProcessing();

    /**
     * All DIO methods will take care for sidecar files, if they exist
     */

    /// Copy an album to another album
    static void copy(PAlbum* const src, PAlbum* const dest);

    /// Copy items to another album
    static void copy(const QList<ItemInfo>& infos, PAlbum* const dest);

    /// Copy an external file to another album
    static void copy(const QUrl& src, PAlbum* const dest);

    /// Copy external files to another album
    static void copy(const QList<QUrl>& srcList, PAlbum* const dest);

    /// Copy items to external folder
    static void copy(const QList<ItemInfo>& infos, const QUrl& dest);

    /// Move an album into another album
    static void move(PAlbum* const src, PAlbum* const dest);

    /// Move items to another album
    static void move(const QList<ItemInfo>& infos, PAlbum* const dest);

    /// Move external files another album
    static void move(const QUrl& src, PAlbum* const dest);

    /// Move external files into another album
    static void move(const QList<QUrl>& srcList, PAlbum* const dest);

    static void del(const QList<ItemInfo>& infos, bool useTrash);
    static void del(const ItemInfo& info, bool useTrash);
    static void del(PAlbum* const album, bool useTrash);

    /// Rename item to new name
    static void rename(const QUrl& src, const QString& newName, bool overwrite = false);

    /// Trash operations
    static int  getTrashCounter(const QString& albumRootPath);
    static void restoreTrash(const DTrashItemInfoList& infos);
    static void emptyTrash(const DTrashItemInfoList& infos);
    static void buildCollectionTrashCounters();

Q_SIGNALS:

    void signalTrashCounters();
    void signalTrashFinished();
    void signalRenameFinished();
    void signalRenameFailed(const QUrl& url);

private:

    void processJob(IOJobData* const data);
    void createJob(IOJobData* const data);

    void updateAlbumDate(int albumID);
    QString getItemString(IOJobData* const data)         const;
    ProgressItem* getProgressItem(IOJobData* const data) const;
    void addAlbumChildrenToList(QList<int>& list, Album* const album);

private Q_SLOTS:

    void slotResult();
    void slotCancel(ProgressItem* item);
    void slotOneProccessed(const QUrl& url);
    void slotTrashCounterMap(const QMap<QString, int>& counterMap);

private:

    /// @note disabled
    explicit DIO(QObject*) = delete;

private:

    int                             m_processingCount    = 0;
    QMap<QString, int>              m_trashCounterMap;
    QMultiHash<IOJobData*, QString> m_currentPathHash;
    QMutex                          m_dioMutex;

    friend class DIOCreator;
};

} // namespace Digikam
