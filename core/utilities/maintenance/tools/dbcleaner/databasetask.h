/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-01-29
 * Description : Thread actions task for database cleanup.
 *
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
#include "identity.h"

namespace Digikam
{

class LoadingDescription;
class MaintenanceData;

class DatabaseTask : public ActionJob
{
    Q_OBJECT

public:

    enum Mode
    {
        Unknown,
        ComputeDatabaseJunk,
        CleanCoreDb,
        CleanThumbsDb,
        CleanRecognitionDb,
        CleanSimilarityDb,
        ShrinkDatabases
    };

public:

    explicit DatabaseTask();
    ~DatabaseTask()     override;

    void setMode(Mode mode);
    void setMaintenanceData(MaintenanceData* const data = nullptr);

    void computeDatabaseJunk(bool thumbsDb = false,
                             bool facesDb = false,
                             bool similarityDb = false);

Q_SIGNALS:

    void signalFinished();
    void signalFinishedWithFeedback(bool done, bool errorFree);

    void signalData(const QList<qlonglong>& staleImageIds,
                    const QList<int>& staleThumbIds,
                    const QList<Identity>& staleIdentities,
                    const QList<qlonglong>& staleSimilarityImageIds);

    /**
     * @brief Signal to emit the count of additional items to process.
     */
    void signalAddItemsToProcess(int count);

protected:

    void run()          override;

private:

    /// @note disabled
    DatabaseTask(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
