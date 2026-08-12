/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-05
 * Description : file action manager task list
 *
 * SPDX-FileCopyrightText: 2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAtomicPointer>
#include <QList>
#include <QExplicitlySharedDataPointer>
#include <QDebug>

// Local includes

#include "iteminfo.h"
#include "progressmanager.h"

namespace Digikam
{

class FileActionProgressItemCreator
{
public:

    FileActionProgressItemCreator()                                       = default;
    virtual ~FileActionProgressItemCreator()                              = default;
    virtual ProgressItem* createProgressItem(const QString& action) const = 0;
    virtual void addProgressItem(ProgressItem* const item)                = 0;

private:

    Q_DISABLE_COPY(FileActionProgressItemCreator)
};

// -------------------------------------------------------------------------------------------------------------------

class TwoProgressItemsContainer :  public QSharedData
{
protected:

    QAtomicPointer<ProgressItem> firstItem;
    QAtomicPointer<ProgressItem> secondItem;

protected:

    // Note: It is currently not safe to schedule after the framework had a chance to
    // advance all already scheduled items. For this, we'd need to add a mechanism (flag to block completion?)

    void scheduleOnProgressItem(QAtomicPointer<ProgressItem>& ptr, int total,
                                const QString& action, FileActionProgressItemCreator* const creator);
    void advance(QAtomicPointer<ProgressItem>& ptr, int n);
};

// -------------------------------------------------------------------------------------------------------------------

class FileActionProgressItemContainer : public QObject,
                                        public TwoProgressItemsContainer
{
    Q_OBJECT

public:

    FileActionProgressItemContainer() = default;
    void schedulingForDB(int numberOfInfos, const QString& action, FileActionProgressItemCreator* const creator);
    void dbProcessed(int numberOfInfos);
    void dbFinished();
    void schedulingForWrite(int numberOfInfos, const QString& action, FileActionProgressItemCreator* const creator);
    void written(int numberOfInfos);
    void finishedWriting();

Q_SIGNALS:

    void signalWrittingDone();

private:

    /// @note disabled
    explicit FileActionProgressItemContainer(QObject*);
};

// -------------------------------------------------------------------------------------------------------------------

class FileActionItemInfoList : public QList<ItemInfo>
{
public:

    FileActionItemInfoList()  = default;

    FileActionItemInfoList(const FileActionItemInfoList& copy)
        : QList    (copy),
          container(copy.container)
    {
    }

    ~FileActionItemInfoList() = default;

public:

    static FileActionItemInfoList create(const QList<ItemInfo>& list);
    static FileActionItemInfoList continueTask(const QList<ItemInfo>& list,
                                               FileActionProgressItemContainer* const container);

    FileActionProgressItemContainer* progress() const
    {
        return container.data();
    }

    /// before sending to db worker
    void schedulingForDB(int numberOfInfos,
                         const QString& action,
                         FileActionProgressItemCreator* const creator)
    {
        progress()->schedulingForDB(numberOfInfos, action, creator);
    }

    void schedulingForDB(const QString& action,
                         FileActionProgressItemCreator* const creator)
    {
        schedulingForDB(size(), action, creator);
    }

    /// db worker progress info
    void dbProcessedOne()               const { dbProcessed(1);                         }
    void dbProcessed(int numberOfInfos) const { progress()->dbProcessed(numberOfInfos); }
    void dbFinished()                   const { progress()->dbFinished();               }

    /// db worker calls this before sending to file worker
    void schedulingForWrite(int numberOfInfos,
                            const QString& action,
                            FileActionProgressItemCreator* const creator) const
    {
        progress()->schedulingForWrite(numberOfInfos, action, creator);
    }

    void schedulingForWrite(const QString& action,
                            FileActionProgressItemCreator* const creator) const
    {
        schedulingForWrite(size(), action, creator);
    }

    /// file worker calls this when finished
    void writtenToOne()             const    { written(1);                             }
    void written(int numberOfInfos) const    { progress()->written(numberOfInfos);     }
    void finishedWriting()          const    { progress()->finishedWriting();          }

public:

    QExplicitlySharedDataPointer<FileActionProgressItemContainer> container;

private:

    explicit FileActionItemInfoList(const QList<ItemInfo>& list)
        : QList<ItemInfo>(list)
    {
    }
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::FileActionItemInfoList)
