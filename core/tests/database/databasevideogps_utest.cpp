/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2026-05-18
 * Description : a unit-test to verify that GPS coordinates written
 *               to a video XMP sidecar are updated in the database.
 *
 * SPDX-FileCopyrightText: 2026 by Kristian Karl <kristian dot hermann dot karl at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "databasevideogps_utest.h"

// Qt includes

#include <QFileInfo>
#include <QScopedPointer>
#include <QDir>

// Local includes

#include "dmetadata.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "collectionscanner.h"
#include "iteminfo.h"
#include "itemposition.h"
#include "metaengine.h"
#include "metaenginesettings.h"
#include "digikam_debug.h"

using namespace Digikam;

// GPS coordinates: 59.32694°N, 18.07167°E
static const double TARGET_LATITUDE  = 59.32694;
static const double TARGET_LONGITUDE = 18.07167;
static const double GPS_TOLERANCE    = 0.001;

QTEST_GUILESS_MAIN(DatabaseVideoGpsTest)

QString DatabaseVideoGpsTest::tempFileName(const QString& purpose) const
{
    return QLatin1String("digikamtests-") +
           QLatin1String(metaObject()->className()) +
           QLatin1Char('-') + purpose +
           QLatin1Char('-') + QTime::currentTime().toString();
}

QString DatabaseVideoGpsTest::tempFilePath(const QString& purpose) const
{
    return QDir::tempPath() + QLatin1Char('/') + tempFileName(purpose);
}

void DatabaseVideoGpsTest::initTestCase()
{
    MetaEngine::initializeExiv2();

    // Locate the source video shipped with digiKam (15 KB, no GPS metadata).

    QString sourceVideo = QLatin1String(VIDEO_SRC_DIR)
                        + QLatin1String("/video-digikam.mp4");

    QFileInfo fi(sourceVideo);
    QVERIFY2(fi.exists(), qPrintable(
        QString::fromLatin1("Test video not found: %1").arg(sourceVideo)));

    // Copy the video into a temporary collection directory so that writing
    // the XMP sidecar does not modify the source tree.

    collectionPath = tempFilePath(QLatin1String("collection"));
    QDir().mkpath(collectionPath);

    videoFilePath   = collectionPath + QLatin1String("/video-digikam.mp4");
    sidecarFilePath = videoFilePath  + QLatin1String(".xmp");

    QVERIFY2(QFile::copy(sourceVideo, videoFilePath),
             "Cannot copy test video to temp collection");

    // Embed GPS 0,0 into the video file so it matches the real-world scenario
    // where the video already has GPS coordinates in its container metadata.
    // The bug only manifests when the video has existing GPS that conflicts
    // with the sidecar.

    {
        QScopedPointer<DMetadata> embedMeta(new DMetadata);
        embedMeta->setMetadataWritingMode(MetaEngine::WRITE_TO_FILE_ONLY);
        embedMeta->setWriteWithExifTool(true);
        embedMeta->setUpdateFileTimeStamp(true);
        QVERIFY(embedMeta->load(videoFilePath));
        QVERIFY(embedMeta->setGPSInfo(nullptr, 0.0, 0.0));
        QVERIFY2(embedMeta->applyChanges(),
                 "Cannot embed GPS 0,0 into test video");
    }

    qCDebug(DIGIKAM_TESTS_LOG) << "Video file:" << videoFilePath;

    // Configure metadata settings:
    //   - write to sidecar only
    //   - read sidecars
    //   - ExifTool backend
    //   - save GPS position

    MetaEngineSettingsContainer set;
    set.metadataWritingMode   = MetaEngine::WRITE_TO_SIDECAR_ONLY;
    set.useXMPSidecar4Reading = true;
    set.writeWithExifTool     = true;
    set.readWithExifTool      = true;
    set.savePosition          = true;
    set.updateFileTimeStamp   = true;
    set.rescanImageIfModified = true;
    MetaEngineSettings::instance()->setSettings(set);

    // Create a temporary SQLite database.

    dbFile = tempFilePath(QLatin1String("database"));
    DbEngineParameters params(QLatin1String("QSQLITE"),
                              dbFile,
                              QLatin1String("QSQLITE"),
                              dbFile);
    CoreDbAccess::setParameters(params, CoreDbAccess::MainApplication);
    QVERIFY(CoreDbAccess::checkReadyForUse(nullptr));
    QVERIFY(QFile(dbFile).exists());

    // Add the temp collection and scan it.

    CollectionManager::instance()->addLocation(
        QUrl::fromLocalFile(collectionPath));
    CollectionScanner().completeScan();

    // Find the video in the database.

    QList<AlbumShortInfo> albums = CoreDbAccess().db()->getAlbumShortInfos();
    QVERIFY(!albums.isEmpty());

    for (const AlbumShortInfo& album : std::as_const(albums))
    {
        QStringList urls = CoreDbAccess().db()->getItemURLsInAlbum(album.id);

        for (const QString& file : std::as_const(urls))
        {
            if (file.endsWith(QLatin1String("video-digikam.mp4")))
            {
                ids << ItemInfo::fromLocalFile(file).id();
            }
        }
    }

    QVERIFY2(!ids.isEmpty(),
             "video-digikam.mp4 not found in scanned collection");
    QVERIFY(!ids.contains(-1));

    qCDebug(DIGIKAM_TESTS_LOG) << "Video image ID:" << ids.first();
}

void DatabaseVideoGpsTest::cleanupTestCase()
{
    QFile(dbFile).remove();
    QDir(collectionPath).removeRecursively();
}

void DatabaseVideoGpsTest::cleanup()
{
    QFile::remove(sidecarFilePath);
}

/**
 * Reproduces the bug: when WRITE_TO_SIDECAR_ONLY is configured with ExifTool
 * backend, writing updated GPS coordinates to a video's XMP sidecar does not
 * update the ImagePositions table in the database.
 *
 * Steps:
 *  1. Verify the original GPS in the database is 0,0.
 *  2. Write new GPS coordinates (59.32694°N, 18.07167°E) to the
 *     video's XMP sidecar.
 *  3. Verify the sidecar contains the correct GPS.
 *  4. Rescan the video file so the database picks up the sidecar change.
 *  5. Verify that the database ImagePositions table now has the new coordinates.
 *
 * Expected: latitude ~59.327, longitude ~18.072.
 * Actual (bug): latitude and longitude are still 0.
 */
void DatabaseVideoGpsTest::testVideoGpsSidecarToDatabase()
{
    // ---- Step 1: verify original GPS is 0,0 in the database ----

    ItemPosition posBefore(ids.first());

    qCDebug(DIGIKAM_TESTS_LOG) << "DB latitude before:"  << posBefore.latitudeNumber();
    qCDebug(DIGIKAM_TESTS_LOG) << "DB longitude before:" << posBefore.longitudeNumber();

    QVERIFY2(qAbs(posBefore.latitudeNumber())  < GPS_TOLERANCE,
             "Initial latitude should be ~0");
    QVERIFY2(qAbs(posBefore.longitudeNumber()) < GPS_TOLERANCE,
             "Initial longitude should be ~0");

    // ---- Step 2: write new GPS to XMP sidecar ----

    QScopedPointer<DMetadata> meta(new DMetadata);
    meta->setSettings(MetaEngineSettings::instance()->settings());

    QVERIFY2(meta->load(videoFilePath),
             qPrintable(QString::fromLatin1("Cannot load metadata for %1")
                        .arg(videoFilePath)));

    bool gpsSet = meta->setGPSInfo(nullptr, TARGET_LATITUDE, TARGET_LONGITUDE);
    QVERIFY2(gpsSet, "setGPSInfo failed");

    QVERIFY2(meta->applyChanges(),
             "applyChanges failed — sidecar was not written");

    // ---- Step 3: verify sidecar exists and contains the correct GPS ----

    QFileInfo sidecarInfo(sidecarFilePath);
    QVERIFY2(sidecarInfo.exists(),
             qPrintable(QString::fromLatin1("Sidecar not created: %1")
                        .arg(sidecarFilePath)));

    QScopedPointer<DMetadata> sidecarMeta(new DMetadata);
    QVERIFY(sidecarMeta->load(sidecarFilePath));

    double sidecarAlt = 0.0, sidecarLat = 0.0, sidecarLon = 0.0;
    QVERIFY(sidecarMeta->getGPSInfo(sidecarAlt, sidecarLat, sidecarLon));

    qCDebug(DIGIKAM_TESTS_LOG) << "Sidecar latitude:"  << sidecarLat;
    qCDebug(DIGIKAM_TESTS_LOG) << "Sidecar longitude:" << sidecarLon;

    QVERIFY2(qAbs(sidecarLat - TARGET_LATITUDE)  < GPS_TOLERANCE,
             "Sidecar latitude does not match target");
    QVERIFY2(qAbs(sidecarLon - TARGET_LONGITUDE) < GPS_TOLERANCE,
             "Sidecar longitude does not match target");

    // ---- Step 4: rescan the video file ----

    CollectionScanner().scanFile(videoFilePath, CollectionScanner::Rescan);

    // ---- Step 5: verify the database has the new GPS coordinates ----

    ItemPosition posAfter(ids.first());

    qCDebug(DIGIKAM_TESTS_LOG) << "DB latitude after:"  << posAfter.latitudeNumber();
    qCDebug(DIGIKAM_TESTS_LOG) << "DB longitude after:" << posAfter.longitudeNumber();

    QVERIFY2(qAbs(posAfter.latitudeNumber()  - TARGET_LATITUDE)  < GPS_TOLERANCE,
             qPrintable(QString::fromLatin1(
                 "Database latitude not updated. Expected ~%1, got %2")
                 .arg(TARGET_LATITUDE)
                 .arg(posAfter.latitudeNumber())));

    QVERIFY2(qAbs(posAfter.longitudeNumber() - TARGET_LONGITUDE) < GPS_TOLERANCE,
             qPrintable(QString::fromLatin1(
                 "Database longitude not updated. Expected ~%1, got %2")
                 .arg(TARGET_LONGITUDE)
                 .arg(posAfter.longitudeNumber())));
}

#include "moc_databasevideogps_utest.cpp"
