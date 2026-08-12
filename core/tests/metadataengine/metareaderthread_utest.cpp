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

#include "metareaderthread_utest.h"

// Qt includes

#include <QDirIterator>
#include <QApplication>
#include <QSignalSpy>
#include <QScopedPointer>
#include <QSettings>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dmetadata.h"
#include "metaengine_previews.h"

Mytask::Mytask(QObject* const parent)
    : ActionJob(parent),
      direction(MetaReaderThread::READ_INFO_FROM_FILE),
      settings ()
{
}

void Mytask::run()
{
    ActionJob::run();       // To customize thread name

    qCDebug(DIGIKAM_TESTS_LOG) << "Processing:" << url.path();
    bool processed = false;

    if (direction != MetaReaderThread::READ_PREVIEW_FROM_FILE)
    {
        QScopedPointer<DMetadata> meta(new DMetadata);
        meta->setSettings(settings);

        if (meta->load(url.toLocalFile()))
        {
            // NOTE: only process data here without to generate extra debug statements,
            //       for performance purpose.

            switch (direction)
            {
                case (MetaReaderThread::READ_INFO_FROM_FILE):
                {

                    // Get most important info used to populate the core-database

                    meta->getItemDimensions();
                    meta->getItemComments();
                    meta->getItemTitles();
                    meta->getCreatorContactInfo();
                    meta->getIptcCoreLocation();
                    meta->getIptcCoreSubjects();
                    meta->getPhotographInformation();
                    QStringList tmp;
                    meta->getItemTagsPath(tmp);
                    meta->getXmpKeywords();
                    meta->getXmpSubjects();
                    meta->getXmpSubCategories();
                    processed = true;
                    break;
                }

                default: // WRITE_INFO_TO_SIDECAR
                {
                    // Just create sidecar files with these info which will touch Exif, Iptc, and Xmp metadata

                    meta->setItemDimensions(QSize(256, 256));
                    meta->setImageDateTime(QDateTime::currentDateTime());
                    meta->setComments(QString::fromLatin1("MetaReaderThread").toLatin1());
                    meta->setItemRating(1);
                    meta->setItemPickLabel(2);
                    meta->setItemColorLabel(3);
                    meta->setItemTagsPath(QStringList() << QLatin1String("digiKam/Unit Tests/Metadata Engine/MetaReaderThread"));

                    // This stage will write a sidecar in temporary directory without to touch original file.

                    meta->save(tempDir + QString::fromUtf8("/%1").arg(url.path().remove(QLatin1Char('/'))));
                    processed = true;
                    break;
                }
            }
        }
    }
    else
    {
        MetaEnginePreviews previews(url.path());
        processed = !previews.isEmpty();
    }

    Q_EMIT signalStats(url, processed);

    qCDebug(DIGIKAM_TESTS_LOG) << "Processed:" << url.path();

    Q_EMIT signalDone();
}

// ----------------------------------------------------------------------------------------------------

MetaReaderThread::MetaReaderThread(QObject* const parent)
    : ActionThreadBase(parent)
{
    setObjectName(QLatin1String("MetaReaderThread"));
}

MetaReaderThread::~MetaReaderThread()
{
}

QString MetaReaderThread::directionToString(Direction direction)
{
    QString ret;

    switch (direction)
    {
        case (MetaReaderThread::READ_INFO_FROM_FILE):
        {
             ret = QLatin1String("Read info from file");
             break;
        }

        case (MetaReaderThread::READ_PREVIEW_FROM_FILE):
        {
             ret = QLatin1String("Read preview from file");
             break;
        }

        default: // WRITE_INFO_TO_SIDECAR
        {
             ret = QLatin1String("Write info to side-car");
             break;
        }
     }

     return ret;
}

void MetaReaderThread::readMetadata(const QList<QUrl>& list,
                                    Direction direction,
                                    const MetaEngineSettingsContainer& settings,
                                    const QString& temp)
{
    ActionJobCollection collection;

    for (const QUrl& url : std::as_const(list))
    {
        Mytask* const job = new Mytask();
        job->url          = url;
        job->direction    = direction;
        job->settings     = settings;
        job->tempDir      = temp;
        collection.insert(job, 0);

        connect(job, SIGNAL(signalStats(QUrl,bool)),
                this, SLOT(slotStats(QUrl,bool)));
    }

    appendJobs(collection);
    m_timer.start();
}

void MetaReaderThread::slotStats(const QUrl& url, bool p)
{
    m_stats.insert(url, p);
}

void MetaReaderThread::slotJobFinished()
{
    ActionThreadBase::slotJobFinished();

    qCDebug(DIGIKAM_TESTS_LOG) << "Pending items to process:" << pendingCount();
    qCDebug(DIGIKAM_TESTS_LOG) << "Elaspsed time in seconds:" << m_timer.elapsed() / 1000.0;

    if (isEmpty())
    {
        Q_EMIT done();
    }
}

QString MetaReaderThread::stats(const QStringList& mimeTypes)
{
    QString out;
    QStringList list;
    int count       = 0;
    const auto urls = m_stats.keys();

    for (const QUrl& url : urls)
    {
        list << QFileInfo(url.path()).suffix();
    }

    for (QString mt : std::as_const(mimeTypes))
    {
        mt.remove(QLatin1String("*."));
        count  = list.count(mt);
        count += list.count(mt.toUpper());

        if (count != 0)
        {
            out.append(QString::fromLatin1("%1(%2) ").arg(mt).arg(count));
        }
    }

    count = m_stats.values().count(false);
    out.append(QString::fromLatin1("Failed(%1 - %2%) ")
        .arg(count)
        .arg((m_stats.count() > 0) ? (count * 100.0 / m_stats.count())
                                   : 0)
    );

    return out;
}

// ----------------------------------------------------------------------------------------------------

QTEST_MAIN(MetaReaderThreadTest)

MetaReaderThreadTest::MetaReaderThreadTest(QObject* const parent)
    : AbstractUnitTest(parent)
{
}

void MetaReaderThreadTest::testMetaReaderThread()
{
    MetaEngineSettingsContainer settings;
    settings.useXMPSidecar4Reading        = false;
    settings.metadataWritingMode          = DMetadata::WRITE_TO_SIDECAR_ONLY;

    QString filters;
    supportedImageMimeTypes(QIODevice::ReadOnly, filters);              // By defaults, all mime-types supported by digiKam.
    int threadsToUse                      = 0;                          // By default all cpu cores.
    QString path                          = m_originalImageFolder;      // By default the unit-tests data dir.

    // Read configuration from ~/.config/MetaReaderThreadTest.conf
    // Template file can be found at core/tests/metadataengine/data/

    QSettings conf(QLatin1String("MetaReaderThreadTest"));
    qCDebug(DIGIKAM_TESTS_LOG) << "Check custom configuration file" << conf.fileName();

    if (!QFileInfo::exists(conf.fileName()))
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Configuration file do not exists.";
        qCDebug(DIGIKAM_TESTS_LOG) << "You can customize this unit-test to copy the template file from";
        qCDebug(DIGIKAM_TESTS_LOG) << m_originalImageFolder << "in your home directory...";
    }
    else
    {
        bool useConf = conf.value(QLatin1String("Enable"), 0).toInt();

        // If configuration file is used, only one unit-test will be processed accordingly with the settings.

        if (useConf)
        {
            qCDebug(DIGIKAM_TESTS_LOG) << "We will use custom configuration file with this unit-test...";

            threadsToUse        = conf.value(QLatin1String("ThreadsToUse"), 0).toInt();
            QString confFilters = conf.value(QLatin1String("Filters"), QString()).toString();

            if (!confFilters.isEmpty())
            {
                filters = confFilters;
            }

            QString confPath = conf.value(QLatin1String("Path"), QString()).toString();

            if (!confPath.isEmpty())
            {
                path = confPath;
            }

            MetaReaderThread::Direction direction = (MetaReaderThread::Direction)conf.value(QLatin1String("Direction"),
                                                    (int)MetaReaderThread::NOT_DEFINED).toInt();

            if (direction == MetaReaderThread::NOT_DEFINED)
            {
                direction = MetaReaderThread::READ_INFO_FROM_FILE;
            }

            runMetaReader(path, filters.split(QLatin1Char(' ')), direction, settings, threadsToUse);

            return;
        }
    }

    // Standard unit-tests

    QStringList mimeTypes = filters.split(QLatin1Char(' '));

    runMetaReader(path, mimeTypes, MetaReaderThread::READ_INFO_FROM_FILE,    settings, threadsToUse);
    runMetaReader(path, mimeTypes, MetaReaderThread::READ_PREVIEW_FROM_FILE, settings, threadsToUse);
    runMetaReader(path, mimeTypes, MetaReaderThread::WRITE_INFO_TO_SIDECAR,  settings, threadsToUse);
}

void MetaReaderThreadTest::runMetaReader(const QString& path,
                                         const QStringList& mimeTypes,
                                         MetaReaderThread::Direction direction,
                                         const MetaEngineSettingsContainer& settings,
                                         int threadsToUse)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "-- Start to process" << path << "------------------------------";

    QList<QUrl> list;
    QDirIterator it(path, mimeTypes,
                    QDir::Files,
                    QDirIterator::Subdirectories);

    while (it.hasNext())
    {
        QString path2 = it.next();
        list.append(QUrl::fromLocalFile(path2));
    }

    if (list.isEmpty())
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Files list to process is empty!";
        return;
    }

    MetaReaderThread* const thread = new MetaReaderThread(this);

    if (threadsToUse > 0)
    {
        thread->setMaximumNumberOfThreads(threadsToUse);
    }

    thread->readMetadata(list, direction, settings, m_tempDir.absolutePath());

    QSignalSpy spy(thread, SIGNAL(done()));

    thread->start();

    while (!spy.wait(1000));  // Time-out in milliseconds

    qCDebug(DIGIKAM_TESTS_LOG) << Qt::endl << "Scan have been completed:"                                                    << Qt::endl
             <<         "    Processing duration:" << thread->m_timer.elapsed() / 1000.0 << " seconds" << Qt::endl
             <<         "    Root path          :" << path                                             << Qt::endl
             <<         "    Number of files    :" << list.size()                                      << Qt::endl
             <<         "    Number of threads  :" << thread->maximumNumberOfThreads()                 << Qt::endl
             <<         "    Direction          :" << MetaReaderThread::directionToString(direction)   << Qt::endl
             <<         "    Type-mimes         :" << mimeTypes.join(QLatin1Char(' '))                 << Qt::endl
             <<         "    Engine settings    :" << settings                                         << Qt::endl
             <<         "    Statistics         :" << thread->stats(mimeTypes)                         << Qt::endl;

    thread->cancel();
    delete thread;
}

#include "moc_metareaderthread_utest.cpp"
