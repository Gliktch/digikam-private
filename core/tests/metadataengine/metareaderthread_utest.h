/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2016-08-14
 * Description : An unit test to read or write metadata through multi-core threads.
 *
 * SPDX-FileCopyrightText: 2016-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QUrl>
#include <QMap>
#include <QElapsedTimer>

// Local includes

#include "abstractunittest.h"
#include "metaenginesettingscontainer.h"
#include "dmetadatasettingscontainer.h"
#include "actionthreadbase.h"

using namespace Digikam;

// -------------------------------------------------------------------------
// Multi-threads manager for separated job

class MetaReaderThread : public ActionThreadBase
{
    Q_OBJECT

public:

    enum Direction
    {
        NOT_DEFINED            = -1,
        READ_INFO_FROM_FILE    = 0,   /// Read lead metadata info from file used to populate the core-database.
        READ_PREVIEW_FROM_FILE = 1,   /// Read preview from file to populate thumbs-database.
        WRITE_INFO_TO_SIDECAR  = 2    /// Write metadata info to sidecar.
    };

public:

    explicit MetaReaderThread(QObject* const parent);
    ~MetaReaderThread() override;

    void readMetadata(const QList<QUrl>& list,
                      Direction direction,
                      const MetaEngineSettingsContainer& settings,
                      const QString& temp);

    QString stats(const QStringList& mimeTypes);

    static QString directionToString(Direction direction);

public:

    QElapsedTimer m_timer;

protected Q_SLOTS:

    void slotJobFinished() override;
    void slotStats(const QUrl& url, bool p);

Q_SIGNALS:

    void done();

private:

    QMultiMap<QUrl, bool> m_stats;     // Map of type-mime, and file processed.
};

// -------------------------------------------------------------------------
// Separated job multi-threaded to run operations over file metadata

class Mytask : public ActionJob
{
    Q_OBJECT

public:

    explicit Mytask(QObject* const parent = nullptr);

public:

    QUrl                        url;
    MetaReaderThread::Direction direction;
    MetaEngineSettingsContainer settings;
    QString                     tempDir;

protected:

    void run() override;

Q_SIGNALS:

    void signalStats(const QUrl&, bool);
};

// -------------------------------------------------------------------------
// Unit-test class

class MetaReaderThreadTest : public AbstractUnitTest
{
    Q_OBJECT

public:

    explicit MetaReaderThreadTest(QObject* const parent = nullptr);

private:

    void runMetaReader(const QString& path,
                       const QStringList& mimeTypes,
                       MetaReaderThread::Direction direction,
                       const MetaEngineSettingsContainer& settings,
                       int threadsToUse);

private Q_SLOTS:

    void testMetaReaderThread();
};
