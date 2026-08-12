/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-05
 * Description : file action manager
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMutex>
#include <QSet>
#include <QTimer>

// Local includes

#include "fileactionmngr.h"
#include "fileworkeriface.h"
#include "fileactionimageinfolist.h"
#include "databaseworkeriface.h"
#include "metadatahub.h"
#include "parallelworkers.h"
#include "aitoolspipeline.h"

namespace Digikam
{

enum GroupAction
{
    AddToGroup,
    RemoveFromGroup,
    Ungroup
};

class Q_DECL_HIDDEN PrivateProgressItemCreator : public QObject,                            // clazy:exclude=ctor-missing-parent-argument
                                                 public FileActionProgressItemCreator
{
    Q_OBJECT

public:

    ProgressItem* createProgressItem(const QString& action) const override;
    void addProgressItem(ProgressItem* const item) override;

    QAtomicInt activeProgressItems;

Q_SIGNALS:

    void lastItemCompleted();

public Q_SLOTS:

    void slotProgressItemCompleted();
    void slotProgressItemCanceled(ProgressItem* item);
};

// -----------------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN FileActionMngr::Private : public QObject
{
    Q_OBJECT

public:

    explicit Private(FileActionMngr* const qq);
    ~Private() override;

Q_SIGNALS:

    void signalTasksFinished();

    // Inter-thread signals: connected to database worker slots
    void signalAddTags(const FileActionItemInfoList& infos, const QList<int>& tagIDs);
    void signalRemoveTags(const FileActionItemInfoList& infos, const QList<int>& tagIDs);
    void signalAssignPickLabel(const FileActionItemInfoList& infos, int pickId);
    void signalAssignColorLabel(const FileActionItemInfoList& infos, int colorId);
    void signalAssignRating(const FileActionItemInfoList& infos, int rating);
    void signalSetExifOrientation(const FileActionItemInfoList& infos, int orientation);
    void signalApplyMetadata(const FileActionItemInfoList& infos, DisjointMetadata* hub);
    void signalEditGroup(int groupAction, const ItemInfo& pick, const FileActionItemInfoList& infos);
    void signalTransform(const FileActionItemInfoList& infos, int orientation);
    void signalCopyAttributes(const FileActionItemInfoList& infos, const QStringList& derivedPaths);

public:

    // -- Signal-emitter glue code --

    void assignTags(const FileActionItemInfoList& infos, const QList<int>& tagIDs)
    {
        Q_EMIT signalAddTags(infos, tagIDs);
    }

    void removeTags(const FileActionItemInfoList& infos, const QList<int>& tagIDs)
    {
        Q_EMIT signalRemoveTags(infos, tagIDs);
    }

    void assignPickLabel(const FileActionItemInfoList& infos, int pickId)
    {
        Q_EMIT signalAssignPickLabel(infos, pickId);
    }

    void assignColorLabel(const FileActionItemInfoList& infos, int colorId)
    {
        Q_EMIT signalAssignColorLabel(infos, colorId);
    }

    void assignRating(const FileActionItemInfoList& infos, int rating)
    {
        Q_EMIT signalAssignRating(infos, rating);
    }

    void editGroup(int groupAction, const ItemInfo& stack, const FileActionItemInfoList& infos)
    {
        Q_EMIT signalEditGroup(groupAction, stack, infos);
    }

    void setExifOrientation(const FileActionItemInfoList& infos, int orientation)
    {
        Q_EMIT signalSetExifOrientation(infos, orientation);
    }

    void applyMetadata(const FileActionItemInfoList& infos, DisjointMetadata* hub)
    {
        Q_EMIT signalApplyMetadata(infos, hub);
    }

    void transform(const FileActionItemInfoList& infos, int orientation)
    {
        Q_EMIT signalTransform(infos, orientation);
    }

    void copyAttributes(const FileActionItemInfoList& infos, const QStringList& derivedPaths)
    {
        Q_EMIT signalCopyAttributes(infos, derivedPaths);
    }

public:

    // -- Workflow controlling --

    bool isActive() const;

    /// db worker will send info to file worker if returns true
    bool shallSendForWriting(qlonglong id);

    /// file worker calls this when receiving a task
    void startingToWrite(const QList<ItemInfo>& infos);

    void connectToDatabaseWorker();
    void connectDatabaseToFileWorker();

    PrivateProgressItemCreator* dbProgressCreator();
    PrivateProgressItemCreator* fileProgressCreator();

public Q_SLOTS:

    void slotImageDataChanged(const QString& path, bool removeThumbnails, bool notifyCache);
    void slotSleepTimer();
    void slotLastProgressItemCompleted();

public:

    QSet<qlonglong>                       scheduledToWrite;
    QString                               dbMessage;
    QString                               writerMessage;
    QMutex                                mutex;

    FileActionMngr*                       q               = nullptr;

    DatabaseWorkerInterface*              dbWorker        = nullptr;
    ParallelAdapter<FileWorkerInterface>* fileWorker;

    QTimer*                               sleepTimer      = nullptr;

    PrivateProgressItemCreator            dbProgress;
    PrivateProgressItemCreator            fileProgress;
};

} // namespace Digikam
