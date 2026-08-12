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

#include "databaseworkeriface.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "collectionscanner.h"
#include "coredboperationgroup.h"
#include "iteminfotasksplitter.h"
#include "fileactionmngr_p.h"
#include "scancontroller.h"
#include "disjointmetadata.h"
#include "faceutils.h"

namespace Digikam
{

void FileActionMngrDatabaseWorker::assignTags(const FileActionItemInfoList& infos, const QList<int>& tagIDs)
{
    changeTags(infos, tagIDs, true);
}

void FileActionMngrDatabaseWorker::removeTags(const FileActionItemInfoList& infos, const QList<int>& tagIDs)
{
    changeTags(infos, tagIDs, false);
}

void FileActionMngrDatabaseWorker::changeTags(const FileActionItemInfoList& infos,
                                              const QList<int>& tagIDs, bool addOrRemove)
{
    DisjointMetadata hub;
    QList<ItemInfo> forWriting;

    {
        CoreDbOperationGroup group;
        group.setMaximumTime(200);

        for (const ItemInfo& info : std::as_const(infos))
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            hub.load(info);

            for (QList<int>::const_iterator tagIt = tagIDs.constBegin() ; tagIt != tagIDs.constEnd() ; ++tagIt)
            {
                if (addOrRemove)
                {
                    hub.setTag(*tagIt, DisjointMetadataDataFields::MetadataAvailable);
                }
                else
                {
                    hub.setTag(*tagIt, DisjointMetadataDataFields::MetadataInvalid);
                }
            }

            hub.write(info, DisjointMetadata::PartialWrite);

            if (hub.willWriteMetadata(DisjointMetadata::FullWriteIfChanged) &&
                d->shallSendForWriting(info.id()))
            {
                forWriting << info;
            }

            infos.dbProcessedOne();
            group.allowLift();
        }
    }

    // send for writing file metadata

    if (!forWriting.isEmpty())
    {
        FileActionItemInfoList forWritingTaskList = FileActionItemInfoList::continueTask(forWriting, infos.progress());
        forWritingTaskList.schedulingForWrite(i18n("Writing metadata to files"), d->fileProgressCreator());

        qCDebug(DIGIKAM_GENERAL_LOG) << "Scheduled to write";

        for (ItemInfoTaskSplitter splitter(forWritingTaskList) ; splitter.hasNext() ; )
        {
            Q_EMIT writeMetadata(FileActionItemInfoList(splitter.next()), MetadataHub::WRITE_TAGS);
        }
    }

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::assignPickLabel(const FileActionItemInfoList& infos, int pickId)
{
    DisjointMetadata hub;
    QList<ItemInfo> forWriting;

    {
        CoreDbOperationGroup group;
        group.setMaximumTime(200);

        for (const ItemInfo& info : std::as_const(infos))
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            hub.load(info);
            hub.setPickLabel(pickId);
            hub.write(info, DisjointMetadata::PartialWrite);

            if (
                hub.willWriteMetadata(DisjointMetadata::FullWriteIfChanged) &&
                d->shallSendForWriting(info.id())
               )
            {
                forWriting << info;
            }

            infos.dbProcessedOne();
            group.allowLift();
        }
    }

    // send for writing file metadata

    if (!forWriting.isEmpty())
    {
        FileActionItemInfoList forWritingTaskList = FileActionItemInfoList::continueTask(forWriting, infos.progress());
        forWritingTaskList.schedulingForWrite(i18n("Writing metadata to files"), d->fileProgressCreator());

        for (ItemInfoTaskSplitter splitter(forWritingTaskList) ; splitter.hasNext() ; )
        {
            Q_EMIT writeMetadata(FileActionItemInfoList(splitter.next()), MetadataHub::WRITE_PICKLABEL);
        }
    }

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::assignColorLabel(const FileActionItemInfoList& infos, int colorId)
{
    DisjointMetadata hub;
    QList<ItemInfo> forWriting;

    {
        CoreDbOperationGroup group;
        group.setMaximumTime(200);

        for (const ItemInfo& info : std::as_const(infos))
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            hub.load(info);
            hub.setColorLabel(colorId);
            hub.write(info, DisjointMetadata::PartialWrite);

            if (
                hub.willWriteMetadata(DisjointMetadata::FullWriteIfChanged) &&
                d->shallSendForWriting(info.id())
               )
            {
                forWriting << info;
            }

            infos.dbProcessedOne();
            group.allowLift();
        }
    }

    // send for writing file metadata

    if (!forWriting.isEmpty())
    {
        FileActionItemInfoList forWritingTaskList = FileActionItemInfoList::continueTask(forWriting, infos.progress());
        forWritingTaskList.schedulingForWrite(i18n("Writing metadata to files"), d->fileProgressCreator());

        for (ItemInfoTaskSplitter splitter(forWritingTaskList) ; splitter.hasNext() ; )
        {
            Q_EMIT writeMetadata(FileActionItemInfoList(splitter.next()), MetadataHub::WRITE_COLORLABEL);
        }
    }

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::assignRating(const FileActionItemInfoList& infos, int rating)
{
    DisjointMetadata hub;
    QList<ItemInfo> forWriting;
    rating = qMin(RatingMax, qMax(RatingMin, rating));

    {
        CoreDbOperationGroup group;
        group.setMaximumTime(200);

        for (const ItemInfo& info : std::as_const(infos))
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            hub.load(info);
            hub.setRating(rating);
            hub.write(info, DisjointMetadata::PartialWrite);

            if (
                hub.willWriteMetadata(DisjointMetadata::FullWriteIfChanged) &&
                d->shallSendForWriting(info.id())
               )
            {
                forWriting << info;
            }

            infos.dbProcessedOne();
            group.allowLift();
        }
    }

    // send for writing file metadata

    if (!forWriting.isEmpty())
    {
        FileActionItemInfoList forWritingTaskList = FileActionItemInfoList::continueTask(forWriting, infos.progress());
        forWritingTaskList.schedulingForWrite(i18n("Writing metadata to files"), d->fileProgressCreator());

        for (ItemInfoTaskSplitter splitter(forWritingTaskList) ; splitter.hasNext() ; )
        {
            Q_EMIT writeMetadata(FileActionItemInfoList(splitter.next()), MetadataHub::WRITE_RATING);
        }
    }

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::editGroup(int groupAction, const ItemInfo& stack, const FileActionItemInfoList& infos)
{
    {
        CoreDbOperationGroup group;
        group.setMaximumTime(200);

        for (const ItemInfo& constInfo : std::as_const(infos))
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            ItemInfo info(constInfo);

            switch (groupAction)
            {
                case AddToGroup:
                {
                    info.addToGroup(stack);
                    break;
                }

                case RemoveFromGroup:
                {
                    info.removeFromGroup();
                    break;
                }

                case Ungroup:
                {
                    info.clearGroup();
                    break;
                }
            }

            infos.dbProcessedOne();
            group.allowLift();
        }
    }

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::setExifOrientation(const FileActionItemInfoList& infos, int orientation)
{
    {
        CoreDbOperationGroup group;
        group.setMaximumTime(200);

        for (ItemInfo info : std::as_const(infos))
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            // Adjust Faces

            FaceUtils().rotateFaces(info.id(), info.dimensions(),
                                    info.orientation(), orientation);

            info.setOrientation(orientation);
        }
    }

    infos.dbProcessed(infos.count());
    infos.schedulingForWrite(infos.count(), i18n("Revising Exif Orientation tags"), d->fileProgressCreator());

    for (ItemInfoTaskSplitter splitter(infos) ; splitter.hasNext() ; )
    {
        Q_EMIT writeOrientationToFiles(FileActionItemInfoList(splitter.next()), orientation);
    }

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::applyMetadata(const FileActionItemInfoList& infos,
                                                 DisjointMetadata* hub)
{
    {
        CoreDbOperationGroup group;
        group.setMaximumTime(200);

        for (const ItemInfo& info : std::as_const(infos))
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            // apply to database

            hub->write(info);
            infos.dbProcessedOne();
            group.allowLift();
        }
    }

    if (hub->willWriteMetadata(DisjointMetadata::FullWriteIfChanged))
    {
        int flags = hub->changedFlags();

        // don't filter by shallSendForWriting here; we write from the hub, not from freshly loaded data

        infos.schedulingForWrite(infos.size(), i18n("Writing metadata to files"), d->fileProgressCreator());

        for (ItemInfoTaskSplitter splitter(infos) ; splitter.hasNext() ; )
        {
            Q_EMIT writeMetadata(FileActionItemInfoList(splitter.next()), flags);
        }
    }

    delete hub;
    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::copyAttributes(const FileActionItemInfoList& infos, const QStringList& derivedPaths)
{
    if (infos.size() == 1)
    {
        QList<ItemInfo> destInfos;

        for (const QString& path : std::as_const(derivedPaths))
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            ItemInfo dest = ScanController::instance()->scannedInfo(path);
            CollectionScanner::copyFileProperties(infos.first(), dest);
            destInfos << dest;
        }

        infos.dbProcessedOne();

        FileActionItemInfoList writeInfos = FileActionItemInfoList::create(destInfos);

        writeInfos.schedulingForWrite(writeInfos.size(), i18n("Writing metadata to files"), d->fileProgressCreator());

        for (ItemInfoTaskSplitter splitter(writeInfos) ; splitter.hasNext() ; )
        {
            Q_EMIT writeMetadata(FileActionItemInfoList(splitter.next()), MetadataHub::WRITE_ALL);
        }
    }

    infos.dbFinished();
}

} // namespace Digikam

#include "moc_databaseworkeriface.cpp"
