/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-22-01
 * Description : batch sync pictures metadata with database
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "album.h"
#include "iteminfo.h"
#include "maintenancetool.h"

namespace Digikam
{

class MetadataSynchronizer : public MaintenanceTool
{
    Q_OBJECT

public:

    enum SyncDirection
    {
        WriteFromDatabaseToFile = 0,
        ReadFromFileToDatabase
    };

public:

    /**
     * @brief Constructor which sync all images metadata from an Albums list.
     * If list is empty, whole Albums collection is processed.
     */
    explicit MetadataSynchronizer(const AlbumList& list = AlbumList(),
                                  SyncDirection direction = WriteFromDatabaseToFile,
                                  ProgressItem* const parent = nullptr);

    /**
     * @brief Constructor which sync all images metadata from an Images list
     */
    explicit MetadataSynchronizer(const ItemInfoList& list,
                                  SyncDirection = WriteFromDatabaseToFile,
                                  ProgressItem* const parent = nullptr);

    void setTagsOnly(bool value);

    ~MetadataSynchronizer()         override;

    void setUseMultiCoreCPU(bool b) override;

Q_SIGNALS:

    void signalRemovePending(const ItemInfo& info);

private Q_SLOTS:

    void slotStart()                override;
    void slotCancel()               override;
    void slotDone()                 override;      // cppcheck-suppress virtualCallInConstructor

    void slotParseAlbums();
    void slotAlbumParsed(const ItemInfoList&);
    void slotAdvance(const ItemInfo&);
    void slotOneAlbumIsComplete();

private:

    void init(SyncDirection direction);
    void parseList();
    void parsePicture();
    void processOneAlbum();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
