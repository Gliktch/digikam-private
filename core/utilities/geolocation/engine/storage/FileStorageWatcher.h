/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QThread>
#include <QMutex>
#include <QMultiMap>
#include <QDateTime>

namespace Marble
{

// Worker object that lives inside the new Thread
class FileStorageWatcherThread : public QObject
{
    Q_OBJECT

public:

    explicit FileStorageWatcherThread(const QString& dataDirectory, QObject* parent = nullptr);

    ~FileStorageWatcherThread() override;

    quint64 cacheLimit();

Q_SIGNALS:

    /**
     * Is emitted when a variable has changed.
     */
    void variableChanged();

public Q_SLOTS:

    /**
     * Sets the limit of the cache in @p bytes.
    */
    void setCacheLimit(quint64 bytes);

    /**
     * Add @p bytes to the current cache size.
     * So FileStorageWatcher is aware of the current cache size.
     */
    void addToCurrentSize(qint64 bytes);

    /**
     * Setting current cache size to 0.
     */
    void resetCurrentSize();

    /**
     * Stop doing things that take a long time to quit.
     */
    void prepareQuit();

    /**
     * Getting the current size of the data stored on the disc
     */
    void getCurrentCacheSize();

private Q_SLOTS:

    /**
     * Ensures that the cache doesn't exceed limits.
     */
    void ensureCacheSize();

private:

    Q_DISABLE_COPY(FileStorageWatcherThread)

    /**
     * Returns true if it is necessary to delete files.
     */
    bool keepDeleting() const;

    QString m_dataDirectory;
    QMultiMap<QDateTime, QString> m_filesCache;
    quint64 m_cacheLimit;
    quint64 m_cacheSoftLimit;
    quint64 m_currentCacheSize;
    int     m_filesDeleted;
    bool    m_deleting;
    QMutex  m_limitMutex;
    bool    m_willQuit;
};

// Lives inside main thread
class FileStorageWatcher : public QThread
{
    Q_OBJECT

public:

    /**
     * Creates a new FileStorageWatcher, which is a thread watching the
     * space Marble takes on the hard drive and deletes files if necessary.
     *
     * @param dataDirectory The directory where the data is stored
     * @param parent The parent of the object.
     */
    explicit FileStorageWatcher(const QString& dataDirectory = QString(), QObject* parent = nullptr);

    ~FileStorageWatcher() override;

    /**
     * Returns the limit of the cache in bytes.
     */
    quint64 cacheLimit();

public Q_SLOTS:

    /**
     * Sets the limit of the cache in @p bytes.
     */
    void setCacheLimit(quint64 bytes);

    /**
     * Add @p bytes to the current cache size.
     * So FileStorageWatcher is aware of the current cache size.
     */
    void addToCurrentSize(qint64 bytes);

    /**
     * Setting current cache size to 0.
     */
    void resetCurrentSize();

Q_SIGNALS:

    void sizeChanged(qint64 bytes);
    void cleared();

protected:

    /**
     * The function being called at starting Thread.
     * The thread is started by QThread::start().
     */
    void run() override;

private:

    Q_DISABLE_COPY(FileStorageWatcher)

private:

    QString                   m_dataDirectory;
    FileStorageWatcherThread* m_thread      = nullptr;
    QMutex*                   m_limitMutex  = nullptr;
    quint64                   m_limit       = 0;
    bool                      m_started     = false;
    bool                      m_quitting    = false;
};

} // namespace Marble
