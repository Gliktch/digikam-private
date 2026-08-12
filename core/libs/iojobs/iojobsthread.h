/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-15
 * Description : IO Jobs thread for file system jobs
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2018 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "actionthreadbase.h"
#include "digikam_export.h"
#include "dtrashiteminfo.h"

namespace Digikam
{

class IOJob;
class IOJobData;

class DIGIKAM_GUI_EXPORT IOJobsThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit IOJobsThread(QObject* const parent);
    ~IOJobsThread() override;

    /**
     * @brief Starts a number of jobs to copy or move source files to destination
     *
     * @param data: IOJobsData container
     */
    void copyOrMove(IOJobData* const data);

    /**
     * @brief Starts a number of jobs to delete multiple files
     *
     * @param data: IOJobsData container
     */
    void deleteFiles(IOJobData* const data);

    /**
     * @brief Starts one job to rename a file to a new name
     *
     * @param data: IOJobsData container
     */
    void renameFile(IOJobData* const data);

    /**
     * @brief Starts a job for listing trash items in a collection
     *
     * @param collectionPath
     */
    void listDTrashItems(const QString& collectionPath);

    /**
     * @brief creates a job for every item to restore back to album
     *
     * @param data: IOJobsData container
     */
    void restoreDTrashItems(IOJobData* const data);

    /**
     * @brief creates a job for every item to delete from collection trash
     *
     * @param data: IOJobsData container
     */
    void emptyDTrashItems(IOJobData* const data);

    /**
     * @brief creates a job for count trash items from all collections
     */
    void buildTrashCounters();

    /**
     * @brief isCanceled
     *
     * @return true if the thread was interrupted
     */
    bool isCanceled() const;

    /**
     * @brief hasErrors
     *
     * @return true if string list was not empty
     */
    bool hasErrors() const;

    /**
     * @return the current errors list
     */
    QStringList& errorsList() const;

    /**
     * @return the current data job instance
     */
    IOJobData* jobData() const;

public Q_SLOTS:

    /**
     * @brief cancels thread execution
     */
    void slotCancel();

Q_SIGNALS:

    void signalFinished();

    void signalOneProccessed(const QUrl& url);
    void signalRenameFailed(const QUrl& url);

    void collectionTrashItemInfo(const DTrashItemInfo& trashItemInfo);
    void signalTrashCountersMap(const QMap<QString, int>& counterMap);

private:

    /**
     * @brief connects the job with signals/slots
     *
     * @param job to be connected
     */
    void connectOneJob(IOJob* const j);

private Q_SLOTS:

    /**
     * @brief connected to all active jobs and checks if the job
     *        list has finished to report that thread is finished
     */
    void slotOneJobFinished();

    /**
     * @brief A slot to receive the error from the job
     *
     * @param errString: string to be appended
     */
    void slotError(const QString& errString);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
