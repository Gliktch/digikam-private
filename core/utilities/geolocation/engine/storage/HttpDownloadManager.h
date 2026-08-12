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

/**
 * The HttpDownloadManager manages http downloads.
 */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "MarbleGlobal.h"
#include "digikam_export.h"

class QUrl;

namespace Marble
{

class DownloadPolicy;
class StoragePolicy;

/**
 * @short This class manages scheduled downloads.
 *
 * The downloadmanager offers a maximum number of active jobs and a
 * limit for pending jobs.  it also takes care that the job queue
 * won't be polluted by jobs that timed out already.
 *
 * @author Torsten Rahn
 */

class DIGIKAM_EXPORT HttpDownloadManager : public QObject
{
    Q_OBJECT

public:

    /**
     * Creates a new http download manager.
     *
     * @note HttpDownloadManager doesn't take ownership of @p policy.
     *
     * @param policy The storage policy for this manager.
     */
    explicit HttpDownloadManager(StoragePolicy* policy);

    /**
     * Destroys the http download manager.
     */
    ~HttpDownloadManager() override;

    /**
     * Switches loading on/off, useful for offline mode.
     */
    void setDownloadEnabled(const bool enable);
    void addDownloadPolicy(const DownloadPolicy&);

    static QByteArray userAgent(const QString& platform, const QString& plugin);

public Q_SLOTS:

    /**
     * Adds a new job with a sourceUrl, destination file name and given id.
     */
    void addJob(const QUrl& sourceUrl, const QString& destFilename, const QString& id,
                const DownloadUsage usage);

Q_SIGNALS:

    void downloadComplete(const QString&, const QString&);

    /**
     * This signal is emitted if a file is downloaded and the data argument
     * contains the files content. The HttpDownloadManager takes care to save
     * it using the given storage policy.
     */
    void downloadComplete(const QByteArray& data, const QString& initiatorId);

    /**
     * Signal is emitted when a new job is added to the queue.
     */
    void jobAdded();

    /**
     * Signal is emitted when a job is removed from the queue.
     * The job might be completed or blacklisted.
     */
    void jobRemoved();

    /**
      * A job was queued, activated or removed (finished, failed)
      */
    void progressChanged(int active, int queued);

private:

    Q_DISABLE_COPY(HttpDownloadManager)

    Q_PRIVATE_SLOT(d, void finishJob(const QByteArray&, const QString&, const QString& id))
    Q_PRIVATE_SLOT(d, void requeue())
    Q_PRIVATE_SLOT(d, void startRetryTimer())

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Marble
