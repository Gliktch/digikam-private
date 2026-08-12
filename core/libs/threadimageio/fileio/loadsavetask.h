/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : image file IO threaded interface.
 *
 * SPDX-FileCopyrightText: 2005-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QEvent>

// Local includes

#include "dimg.h"
#include "dimgloaderobserver.h"
#include "loadingdescription.h"
#include "loadingcache.h"

namespace Digikam
{

class LoadSaveThread;

class LoadSaveTask
{
public:

    enum TaskType
    {
        TaskTypeLoading,
        TaskTypeSaving
    };

public:

    explicit LoadSaveTask(LoadSaveThread* const thread);
    virtual ~LoadSaveTask() = default;

public:

    virtual void execute()                    = 0;
    virtual TaskType type()                   = 0;

    virtual void progressInfo(float progress) = 0;
    virtual bool continueQuery()              = 0;

protected:

    LoadSaveThread* m_thread = nullptr;

private:

    /// @note disabled
    LoadSaveTask(const LoadSaveTask&)            = delete;
    LoadSaveTask& operator=(const LoadSaveTask&) = delete;
};

//---------------------------------------------------------------------------------------------------

class LoadingTask : public LoadSaveTask,
                    public DImgLoaderObserver
{
public:

    enum LoadingTaskStatus
    {
        LoadingTaskStatusLoading,
        LoadingTaskStatusPreloading,
        LoadingTaskStatusStopping
    };

public:

    explicit LoadingTask(LoadSaveThread* const thread,
                         const LoadingDescription& description,
                         LoadingTaskStatus loadingTaskStatus = LoadingTaskStatusLoading);
    ~LoadingTask() override = default;

    LoadingTaskStatus status()                     const;
    QString filePath()                             const;

    const LoadingDescription& loadingDescription() const;

    // LoadSaveTask

    void execute()                    override;
    TaskType type()                   override;

    // DImgLoaderObserver

    void progressInfo(float progress) override;
    bool continueQuery()              override;

    void setStatus(LoadingTaskStatus status);

protected:

    LoadingDescription         m_loadingDescription;
    volatile LoadingTaskStatus m_loadingTaskStatus = LoadingTaskStatusLoading;

private:

    /// @note disabled
    LoadingTask(const LoadingTask&)            = delete;
    LoadingTask& operator=(const LoadingTask&) = delete;
};

//---------------------------------------------------------------------------------------------------

class SharedLoadingTask : public LoadingTask,
                          public LoadingProcess,
                          public LoadingProcessListener
{
public:

    explicit SharedLoadingTask(LoadSaveThread* const thread,
                               const LoadingDescription& description,
                               LoadSaveThread::AccessMode mode = LoadSaveThread::AccessModeReadWrite,
                               LoadingTaskStatus loadingTaskStatus = LoadingTaskStatusLoading);

    void execute()                                                      override;
    void progressInfo(float progress)                                   override;

    bool needsPostProcessing()                  const;
    bool comparePostProcessing(DImg* const img) const;
    virtual void postProcess();

    // LoadingProcess

    bool completed()                            const                   override;
    QString cacheKey()                          const                   override;
    void addListener(LoadingProcessListener* const listener)            override;
    void removeListener(LoadingProcessListener* const listener)         override;
    void notifyNewLoadingProcess(LoadingProcess* const process,
                                 const LoadingDescription& description) override;

    // LoadingProcessListener

    bool querySendNotifyEvent()                 const                   override;
    void setResult(const LoadingDescription& loadingDescription,
                   const DImg& img)                                     override;
    LoadSaveNotifier* loadSaveNotifier()        const                   override;
    LoadSaveThread::AccessMode accessMode()     const                   override;

    DImg img()                                  const;

protected:

    volatile bool                  m_completed  = false;
    LoadSaveThread::AccessMode     m_accessMode = LoadSaveThread::AccessModeReadWrite;
    QList<LoadingProcessListener*> m_listeners;
    DImg                           m_img;

private:

    /// @note disabled
    SharedLoadingTask(const SharedLoadingTask&)            = delete;
    SharedLoadingTask& operator=(const SharedLoadingTask&) = delete;
};

//---------------------------------------------------------------------------------------------------

class SavingTask : public LoadSaveTask,
                   public DImgLoaderObserver
{
public:

    enum SavingTaskStatus
    {
        SavingTaskStatusSaving,
        SavingTaskStatusStopping
    };

public:

    explicit SavingTask(LoadSaveThread* const thread,
                        const DImg& img,
                        const QString& filePath,
                        const QString& format);

    SavingTaskStatus status() const;
    QString filePath()        const;

public:

    void execute()                    override;
    TaskType type()                   override;

    void progressInfo(float progress) override;
    bool continueQuery()              override;

    void setStatus(SavingTaskStatus status);

private:

    QString                   m_filePath;
    QString                   m_format;
    DImg                      m_img;
    volatile SavingTaskStatus m_savingTaskStatus = SavingTaskStatusSaving;

private:

    /// @note disabled
    SavingTask(const SavingTask&)            = delete;
    SavingTask& operator=(const SavingTask&) = delete;
};

} // namespace Digikam
