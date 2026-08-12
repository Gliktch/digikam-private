/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-09
 * Description : Thread actions manager for maintenance tools.
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2017-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>

// Local includes

#include "actionthreadbase.h"
#include "metadatasynchronizer.h"
#include "metadataremover.h"
#include "iteminfo.h"
#include "identity.h"

class QImage;

namespace Digikam
{

class ImageQualitySettings;
class MaintenanceData;


class MaintenanceThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit MaintenanceThread(QObject* const parent);
    ~MaintenanceThread()    override;

    void setUseMultiCore(const bool b);

    void syncMetadata(const ItemInfoList& items, MetadataSynchronizer::SyncDirection dir, bool tagsOnly);
    void removeMetadata(const ItemInfoList& items, MetadataRemover::RemoveAction action);
    void generateThumbs(const QStringList& paths);
    void generateFingerprints(const QList<qlonglong>& itemIds, bool rebuildAll);
/*
    void generateTags(const QStringList& paths, int modelType, const QStringList& langs);
*/
    void sortByImageQuality(const QStringList& paths, const ImageQualitySettings& quality);

    void computeDatabaseJunk(bool thumbsDb = false, bool facesDb = false, bool similarityDb = false);
    void cleanCoreDb(const QList<qlonglong>& imageIds);
    void cleanThumbsDb(const QList<int>& thumbnailIds);
    void cleanFacesDb(const QList<Identity>& staleIdentities);
    void cleanSimilarityDb(const QList<qlonglong>& imageIds);
    void shrinkDatabases();

    void cancel()           override;

    QString getThumbFingerprintPath();

Q_SIGNALS:

    /**
     * Emit when the task has started it's work.
     */
    void signalStarted();

    /**
     * Emit when an item have been processed. QImage can be used to pass item thumbnail processed.
     */
    void signalAdvanceList(const ItemInfo&, const QImage&, const QStringList&);
    void signalAdvanceInfo(const ItemInfo&, const QImage&);
    void signalAdvanceImage(const QImage&);
    void signalAdvanceProgress(const ItemInfo&, const QImage&, int);

    /**
     * Emit when an item was processed and on additional information is necessary.
     */
    void signalAdvance();

    /**
     * Emit when a items list have been fully processed.
     */
    void signalCompleted();

    /**
     * Signal to emit to sub-tasks to cancel processing.
     */
    void signalCanceled();

    /**
     * Signal to emit junk data for db cleaner.
     */
    void signalData(const QList<qlonglong>& staleImageIds,
                    const QList<int>& staleThumbIds,
                    const QList<Identity>& staleIdentities,
                    const QList<qlonglong>& staleSimilarityImageIds);

    /**
     * Signal to emit the count of additional items to process.
     */
    void signalAddItemsToProcess(int count);

    /**
     * Signal to remove pending item from lazy sync.
     */
    void signalRemovePending(const ItemInfo& info);

    /**
     * Signal to emit after processing with info if the processing was done
     * and if yes, without errors.
     */
    void signalFinished(bool done, bool errorFree);

private Q_SLOTS:

    void slotThreadFinished();

private:

    /**
     * This function generates from the given list
     * a list of lists with each embedded list having size
     * of at most chunkSize. If chunkSize is zero, the original
     * list is the only chunk.
     * @param toChunk The list to chunk
     * @param chunkSize The chunk size (0 for take everything)
     */
/*
    template<typename T>
    QList<QList<T> > chunkList(const QList<T>& toChunk, int chunkSize = 0)
    {
        QList<QList<T> > chunks;

        // Chunk size 0 means all

        if (chunkSize == 0)
        {
            chunks << toChunk;
            return chunks;
        }

        // Buffer the input list.

        QList<T> toChunkList = toChunk;
        QList<T> currentChunk;

        while (!toChunkList.isEmpty())
        {
            // Set the current chunk to the first n elements.

            currentChunk = toChunkList.mid(0,chunkSize);

            // Set the buffer list to the rest, i.e.
            // start at position n and take all from this position
            // If n is bigger than the size, an empty list is returned.
            // see qarraydata.cpp in Qt implementation.

            toChunkList  = toChunkList.mid(chunkSize);
            chunks << currentChunk;
        }

        return chunks;
    }

    int getChunkSize(int elementCount);
*/

private:

    MaintenanceData* const data = nullptr;
};

} // namespace Digikam
