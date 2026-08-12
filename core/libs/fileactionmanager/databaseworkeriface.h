/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-18
 * Description : database worker interface
 *
 * SPDX-FileCopyrightText: 2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>

// Local includes

#include "fileactionmngr.h"
#include "fileactionimageinfolist.h"
#include "iteminfo.h"
#include "workerobject.h"

namespace Digikam
{

class MetadataHub;

class DatabaseWorkerInterface : public WorkerObject      // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT

public Q_SLOTS:

    virtual void assignTags(const FileActionItemInfoList&, const QList<int>&)          {};
    virtual void removeTags(const FileActionItemInfoList&, const QList<int>&)          {};
    virtual void assignPickLabel(const FileActionItemInfoList&, int)                   {};
    virtual void assignColorLabel(const FileActionItemInfoList&, int)                  {};
    virtual void assignRating(const FileActionItemInfoList&, int)                      {};
    virtual void editGroup(int, const ItemInfo&, const FileActionItemInfoList&)        {};
    virtual void setExifOrientation(const FileActionItemInfoList&, int)                {};
    virtual void applyMetadata(const FileActionItemInfoList&, DisjointMetadata*)       {};
    virtual void copyAttributes(const FileActionItemInfoList&, const QStringList&)     {};

Q_SIGNALS:

    void writeMetadataToFiles(FileActionItemInfoList infos);
    void writeOrientationToFiles(FileActionItemInfoList infos, int orientation);
    void writeMetadata(FileActionItemInfoList infos, int flag);
};

// ------------------------------------------------------------------------------------

class FileActionMngrDatabaseWorker : public DatabaseWorkerInterface
{
    Q_OBJECT

public:

    explicit FileActionMngrDatabaseWorker(FileActionMngr::Private* const dd)
        : d(dd)
    {
    }

public:

    void assignTags(const FileActionItemInfoList& infos, const QList<int>& tagIDs)             override;
    void removeTags(const FileActionItemInfoList& infos, const QList<int>& tagIDs)             override;
    void assignPickLabel(const FileActionItemInfoList& infos, int pickId)                      override;
    void assignColorLabel(const FileActionItemInfoList& infos, int colorId)                    override;
    void assignRating(const FileActionItemInfoList& infos, int rating)                         override;
    void editGroup(int groupAction, const ItemInfo& pick, const FileActionItemInfoList& infos) override;
    void setExifOrientation(const FileActionItemInfoList& infos, int orientation)              override;
    void applyMetadata(const FileActionItemInfoList& infos, DisjointMetadata* hub)             override;
    void copyAttributes(const FileActionItemInfoList& infos, const QStringList& derivedPaths)  override;

private:

    void changeTags(const FileActionItemInfoList& infos, const QList<int>& tagIDs, bool addOrRemove);

private:

    FileActionMngr::Private* const d = nullptr;
};

} // namespace Digikam
