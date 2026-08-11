/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyprofilepublication.h"

// Qt includes

#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSet>
#include <QUuid>

// C++ includes

#include <utility>

#ifdef Q_OS_UNIX

// POSIX includes

#   include <fcntl.h>
#   include <unistd.h>

#endif

// KDE includes

#include <KConfigGroup>
#include <KSharedConfig>

// Local includes

#include "privacyprofileinspector.h"
#include "privacysqlitesnapshot.h"

namespace Digikam
{

namespace
{

const char ManifestFileName[] = "publication.json";
const char ManifestFormat[] = "digikam-private-profile-publication";
constexpr int ManifestVersion = 1;

QString cleanAbsolutePath(const QString& path)
{
    return QDir::cleanPath(QFileInfo(path).absoluteFilePath());
}

bool pathIsInside(const QString& path, const QString& directory)
{
    const QString cleanPath = cleanAbsolutePath(path);
    QString cleanDirectory = cleanAbsolutePath(directory);

    if (!cleanDirectory.endsWith(QDir::separator()))
    {
        cleanDirectory += QDir::separator();
    }

    return cleanPath.startsWith(cleanDirectory) &&
           (cleanPath != cleanDirectory.chopped(1));
}

void reportProgress(const PrivacyProfilePublication::Progress& progress,
                    const QString& step,
                    int current,
                    int total)
{
    if (progress)
    {
        progress(step, current, total);
    }
}

QByteArray fileHash(const QString& path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        return QByteArray();
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);

    while (!file.atEnd())
    {
        const QByteArray block = file.read(1024 * 1024);

        if (block.isEmpty() && file.error() != QFileDevice::NoError)
        {
            return QByteArray();
        }

        hash.addData(block);
    }

    return hash.result();
}

bool syncFile(const QString& path, QString* error)
{
#ifdef Q_OS_UNIX

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly) || (::fsync(file.handle()) != 0))
    {
        *error = QLatin1String("A profile file could not be flushed to storage");
        return false;
    }

#else

    Q_UNUSED(path);
    Q_UNUSED(error);

#endif

    return true;
}

bool syncDirectory(const QString& path, QString* error)
{
#ifdef Q_OS_UNIX

    const QByteArray encodedPath = QFile::encodeName(path);
    const int descriptor = ::open(encodedPath.constData(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);

    if (descriptor < 0)
    {
        *error = QLatin1String("A profile directory could not be opened for flushing");
        return false;
    }

    const bool success = (::fsync(descriptor) == 0);
    ::close(descriptor);

    if (!success)
    {
        *error = QLatin1String("A profile directory could not be flushed to storage");
        return false;
    }

#else

    Q_UNUSED(path);
    Q_UNUSED(error);

#endif

    return true;
}

bool copyFile(const QString& source, const QString& destination, QString* error)
{
    const QFileInfo destinationInfo(destination);

    if (!QDir().mkpath(destinationInfo.absolutePath()))
    {
        *error = QLatin1String("A profile backup directory could not be created");
        return false;
    }

    if (QFileInfo::exists(destination) || !QFile::copy(source, destination))
    {
        *error = QLatin1String("A profile file could not be copied safely");
        return false;
    }

    return syncFile(destination, error) &&
           syncDirectory(destinationInfo.absolutePath(), error);
}

bool copyDirectory(const QString& sourceRoot,
                   const QString& destinationRoot,
                   const QSet<QString>& excludedPaths,
                   const PrivacyProfilePublication::Progress& progress,
                   QString* error)
{
    const QDir source(sourceRoot);

    if (!source.exists())
    {
        return QDir().mkpath(destinationRoot);
    }

    if (!QDir().mkpath(destinationRoot))
    {
        *error = QLatin1String("A profile backup directory could not be created");
        return false;
    }

    QStringList entries;
    QDirIterator counter(sourceRoot,
                         QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System,
                         QDirIterator::Subdirectories);

    while (counter.hasNext())
    {
        entries << cleanAbsolutePath(counter.next());
    }

    int current = 0;

    for (const QString& sourcePath : std::as_const(entries))
    {
        ++current;
        reportProgress(progress, QLatin1String("Back up current profile"),
                       current, entries.size());

        if (excludedPaths.contains(sourcePath))
        {
            continue;
        }

        const QFileInfo sourceInfo(sourcePath);
        const QString relativePath = source.relativeFilePath(sourcePath);
        const QString destinationPath = QDir(destinationRoot).filePath(relativePath);

        if (sourceInfo.isDir() && !sourceInfo.isSymLink())
        {
            if (!QDir().mkpath(destinationPath))
            {
                *error = QLatin1String("A profile backup directory could not be created");
                return false;
            }

            QFile::setPermissions(destinationPath, sourceInfo.permissions());
        }
        else if (sourceInfo.isSymLink())
        {
            if (!QDir().mkpath(QFileInfo(destinationPath).absolutePath()) ||
                !QFile::link(sourceInfo.symLinkTarget(), destinationPath))
            {
                *error = QLatin1String("A profile backup link could not be preserved");
                return false;
            }
        }
        else if (sourceInfo.isFile())
        {
            if (!copyFile(sourcePath, destinationPath, error))
            {
                return false;
            }
        }
        else
        {
            *error = QLatin1String("The profile contains an unsupported special file");
            return false;
        }
    }

    return true;
}

QString backupPathFor(const QString& sourcePath,
                      const QString& sourceRoot,
                      const QString& backupRoot,
                      const QString& externalName)
{
    if (pathIsInside(sourcePath, sourceRoot))
    {
        return QDir(backupRoot).filePath(
            QDir(sourceRoot).relativeFilePath(sourcePath));
    }

    return QDir(backupRoot).filePath(QLatin1String("external/") + externalName);
}

bool writeManifest(const QString& path, const QJsonObject& object, QString* error)
{
    QSaveFile file(path);

    if (!file.open(QIODevice::WriteOnly))
    {
        *error = file.errorString();
        return false;
    }

    if (file.write(QJsonDocument(object).toJson(QJsonDocument::Indented)) < 0)
    {
        *error = file.errorString();
        file.cancelWriting();
        return false;
    }

    if (!file.commit())
    {
        *error = file.errorString();
        return false;
    }

    return true;
}

bool readManifest(const QString& path, QJsonObject* object, QString* error)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        *error = file.errorString();
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);

    if ((parseError.error != QJsonParseError::NoError) || !document.isObject())
    {
        *error = QLatin1String("The profile publication journal is invalid");
        return false;
    }

    *object = document.object();

    if ((object->value(QLatin1String("format")).toString() !=
         QLatin1String(ManifestFormat)) ||
        (object->value(QLatin1String("version")).toInt() != ManifestVersion))
    {
        *error = QLatin1String("The profile publication journal format is unsupported");
        return false;
    }

    return true;
}

bool updatePhase(const QString& manifestPath,
                 QJsonObject* manifest,
                 const QString& phase,
                 QString* error)
{
    manifest->insert(QLatin1String("phase"), phase);
    return writeManifest(manifestPath, *manifest, error);
}

bool displaceFile(const QString& path, const QString& displacedDirectory, QString* error)
{
    if (!QFileInfo::exists(path))
    {
        return true;
    }

    if (!QDir().mkpath(displacedDirectory))
    {
        *error = QLatin1String("The displaced-profile directory could not be created");
        return false;
    }

    const QString destination = QDir(displacedDirectory).filePath(
        QFileInfo(path).fileName());

    if (QFileInfo::exists(destination))
    {
        *error = QLatin1String("A conflicting displaced profile file exists");
        return false;
    }

    if (!QFile::rename(path, destination))
    {
        *error = QLatin1String("An active profile file could not be moved into the recovery area");
        return false;
    }

    return syncDirectory(QFileInfo(path).absolutePath(), error) &&
           syncDirectory(displacedDirectory, error);
}

bool publishFile(const QString& candidatePath,
                 const QString& targetPath,
                 const QString& displacedDirectory,
                 QString* error)
{
    const QByteArray candidateHash = fileHash(candidatePath);

    if (candidateHash.isEmpty())
    {
        *error = QLatin1String("A staged profile file could not be verified");
        return false;
    }

    if (fileHash(targetPath) == candidateHash)
    {
        return true;
    }

    const QString partialPath = targetPath + QLatin1String(".digikam-private-import-partial");

    if (QFileInfo::exists(partialPath) && (fileHash(partialPath) != candidateHash))
    {
        *error = QLatin1String("A conflicting partial profile publication exists");
        return false;
    }

    if (!QFileInfo::exists(partialPath) && !copyFile(candidatePath, partialPath, error))
    {
        return false;
    }

    if (!displaceFile(targetPath, displacedDirectory, error) ||
        !displaceFile(targetPath + QLatin1String("-wal"), displacedDirectory, error) ||
        !displaceFile(targetPath + QLatin1String("-shm"), displacedDirectory, error))
    {
        return false;
    }

    if (!QDir().mkpath(QFileInfo(targetPath).absolutePath()) ||
        !QFile::rename(partialPath, targetPath) ||
        (fileHash(targetPath) != candidateHash) ||
        !syncFile(targetPath, error) ||
        !syncDirectory(QFileInfo(targetPath).absolutePath(), error))
    {
        *error = QLatin1String("A staged profile file could not be published atomically");
        return false;
    }

    return true;
}

bool prepareCandidateConfig(const QString& sourceSettingsPath,
                            const PrivacyProfilePaths& target,
                            const QString& candidateConfigPath,
                            QString* error)
{
    const QString source = QFileInfo::exists(sourceSettingsPath)
                         ? sourceSettingsPath
                         : target.configFilePath;

    if (QFileInfo::exists(source))
    {
        if (!copyFile(source, candidateConfigPath, error))
        {
            return false;
        }
    }
    else
    {
        QSaveFile empty(candidateConfigPath);

        if (!QDir().mkpath(QFileInfo(candidateConfigPath).absolutePath()) ||
            !empty.open(QIODevice::WriteOnly) || !empty.commit())
        {
            *error = QLatin1String("The candidate settings file could not be created");
            return false;
        }
    }

    const KSharedConfig::Ptr config = KSharedConfig::openConfig(
        candidateConfigPath, KConfig::SimpleConfig);
    KConfigGroup group(config, QLatin1String("Database Settings"));
    group.writeEntry(QLatin1String("Database Type"), QStringLiteral("QSQLITE"));
    group.writePathEntry(QLatin1String("Database Name"),
                         QFileInfo(target.coreDatabasePath).absolutePath());
    group.writePathEntry(QLatin1String("Database Name Thumbnails"),
                         QFileInfo(target.thumbnailDatabasePath).absolutePath());
    group.writePathEntry(QLatin1String("Database Name Face"), target.dataHome);
    group.writePathEntry(QLatin1String("Database Name Similarity"), target.dataHome);
    group.writeEntry(QLatin1String("Database WAL Mode"), true);
    KConfigGroup importGroup(config, QLatin1String("Private Profile Import"));
    importGroup.writeEntry(QLatin1String("ShowOnStartup"), false);
    group.sync();
    importGroup.sync();
    config->sync();

    return true;
}

} // namespace

bool PrivacyProfilePaths::isValid() const
{
    const QStringList directories = {
        configHome, dataHome, cacheHome, stateHome, transactionHome
    };

    for (const QString& path : directories)
    {
        if (path.isEmpty() || !QFileInfo(path).isAbsolute())
        {
            return false;
        }
    }

    if (configFilePath.isEmpty() || coreDatabasePath.isEmpty() ||
        thumbnailDatabasePath.isEmpty() ||
        !QFileInfo(configFilePath).isAbsolute() ||
        !QFileInfo(coreDatabasePath).isAbsolute() ||
        !QFileInfo(thumbnailDatabasePath).isAbsolute())
    {
        return false;
    }

    return !pathIsInside(transactionHome, configHome) &&
           !pathIsInside(transactionHome, dataHome) &&
           !pathIsInside(transactionHome, cacheHome) &&
           !pathIsInside(transactionHome, stateHome);
}

PrivacyProfilePublicationResult PrivacyProfilePublication::prepare(
    const PrivacyProfileImportStageResult& staged,
    const PrivacyProfilePaths& target,
    const QString& sourceSettingsPath,
    const Progress& progress)
{
    PrivacyProfilePublicationResult result;

    if (!staged.success || !staged.candidateSummary.isPrivateProfile() ||
        (staged.candidateSummary.incompletePrivacyTransactionCount > 0))
    {
        result.error = QLatin1String("The staged profile candidate is not publishable");
        return result;
    }

    if (!target.isValid())
    {
        result.error = QLatin1String("The active private profile paths are invalid");
        return result;
    }

    result.transactionUuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    result.transactionDirectory = QDir(target.transactionHome).filePath(
        QLatin1String("pending-") + result.transactionUuid);
    result.backupDirectory = QDir(result.transactionDirectory).filePath(
        QLatin1String("backup"));
    const QString preparingDirectory = QDir(target.transactionHome).filePath(
        QLatin1String("preparing-") + result.transactionUuid);
    const QString preparingBackupDirectory = QDir(preparingDirectory).filePath(
        QLatin1String("backup"));
    const QString preparingCandidateDirectory = QDir(preparingDirectory).filePath(
        QLatin1String("candidate"));
    const QString candidateDirectory = QDir(result.transactionDirectory).filePath(
        QLatin1String("candidate"));

    if (!QDir().mkpath(preparingCandidateDirectory) ||
        !QDir().mkpath(preparingBackupDirectory))
    {
        result.error = QLatin1String("The profile publication transaction could not be created");
        return result;
    }

    const QString preparingCandidateCore = QDir(preparingCandidateDirectory).filePath(
        QLatin1String("digikam4.db"));
    const QString preparingCandidateThumbs = QDir(preparingCandidateDirectory).filePath(
        QLatin1String("thumbnails-digikam.db"));
    const QString preparingCandidateConfig = QDir(preparingCandidateDirectory).filePath(
        QLatin1String("digikamrc"));
    const QString candidateCore = QDir(candidateDirectory).filePath(QLatin1String("digikam4.db"));
    const QString candidateThumbs = QDir(candidateDirectory).filePath(
        QLatin1String("thumbnails-digikam.db"));
    const QString candidateConfig = QDir(candidateDirectory).filePath(QLatin1String("digikamrc"));

    if (!copyFile(staged.candidateCoreDatabasePath, preparingCandidateCore, &result.error) ||
        (PrivacyProfileInspector::inspectCoreDatabase(preparingCandidateCore).schemaKind !=
         PrivacyProfileSchemaKind::PrivateP1))
    {
        if (result.error.isEmpty())
        {
            result.error = QLatin1String("The copied publication candidate is invalid");
        }

        return result;
    }

    if (staged.thumbnailDatabaseIncluded &&
        !copyFile(staged.candidateThumbnailDatabasePath, preparingCandidateThumbs, &result.error))
    {
        return result;
    }

    if (!prepareCandidateConfig(sourceSettingsPath, target, preparingCandidateConfig,
                                &result.error) ||
        !syncFile(preparingCandidateConfig, &result.error))
    {
        return result;
    }

    KSharedConfig::openConfig()->sync();
    const QSet<QString> excluded = {
        cleanAbsolutePath(target.coreDatabasePath),
        cleanAbsolutePath(target.coreDatabasePath + QLatin1String("-wal")),
        cleanAbsolutePath(target.coreDatabasePath + QLatin1String("-shm")),
        cleanAbsolutePath(target.thumbnailDatabasePath),
        cleanAbsolutePath(target.thumbnailDatabasePath + QLatin1String("-wal")),
        cleanAbsolutePath(target.thumbnailDatabasePath + QLatin1String("-shm"))
    };
    const struct
    {
        QString source;
        QString name;
    } roots[] = {
        { target.configHome, QLatin1String("config") },
        { target.dataHome,   QLatin1String("data") },
        { target.cacheHome,  QLatin1String("cache") },
        { target.stateHome,  QLatin1String("state") }
    };

    for (const auto& root : roots)
    {
        if (!copyDirectory(root.source,
                           QDir(preparingBackupDirectory).filePath(root.name),
                           excluded, progress, &result.error))
        {
            return result;
        }
    }

    const QString preparingBackupCore = backupPathFor(
        target.coreDatabasePath, target.dataHome,
        QDir(preparingBackupDirectory).filePath(QLatin1String("data")),
        QLatin1String("digikam4.db"));
    const QString preparingBackupThumbs = backupPathFor(
        target.thumbnailDatabasePath, target.dataHome,
        QDir(preparingBackupDirectory).filePath(QLatin1String("data")),
        QLatin1String("thumbnails-digikam.db"));
    const QString backupCore = backupPathFor(
        target.coreDatabasePath, target.dataHome,
        QDir(result.backupDirectory).filePath(QLatin1String("data")),
        QLatin1String("digikam4.db"));
    const QString backupThumbs = backupPathFor(
        target.thumbnailDatabasePath, target.dataHome,
        QDir(result.backupDirectory).filePath(QLatin1String("data")),
        QLatin1String("thumbnails-digikam.db"));
    const QString backupConfig = backupPathFor(
        target.configFilePath, target.configHome,
        QDir(result.backupDirectory).filePath(QLatin1String("config")),
        QLatin1String("digikamrc"));

    if (QFileInfo::exists(target.coreDatabasePath))
    {
        const PrivacySqliteSnapshotResult snapshot = PrivacySqliteSnapshot::create(
            target.coreDatabasePath, preparingBackupCore);

        if (!snapshot.success)
        {
            result.error = snapshot.error;
            return result;
        }
    }

    if (QFileInfo::exists(target.thumbnailDatabasePath))
    {
        const PrivacySqliteSnapshotResult snapshot = PrivacySqliteSnapshot::create(
            target.thumbnailDatabasePath, preparingBackupThumbs);

        if (!snapshot.success)
        {
            result.error = snapshot.error;
            return result;
        }
    }

    QJsonObject manifest {
        { QLatin1String("format"), QLatin1String(ManifestFormat) },
        { QLatin1String("version"), ManifestVersion },
        { QLatin1String("uuid"), result.transactionUuid },
        { QLatin1String("createdAt"),
          QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs) },
        { QLatin1String("phase"), QLatin1String("ready") },
        { QLatin1String("candidateCore"), candidateCore },
        { QLatin1String("candidateThumbs"),
          staged.thumbnailDatabaseIncluded ? candidateThumbs : QString() },
        { QLatin1String("candidateConfig"), candidateConfig },
        { QLatin1String("targetCore"), target.coreDatabasePath },
        { QLatin1String("targetThumbs"), target.thumbnailDatabasePath },
        { QLatin1String("targetConfig"), target.configFilePath },
        { QLatin1String("targetDataHome"), target.dataHome },
        { QLatin1String("backupDirectory"), result.backupDirectory },
        { QLatin1String("backupConfig"), backupConfig },
        { QLatin1String("backupCore"), backupCore },
        { QLatin1String("backupThumbs"), backupThumbs }
    };
    const QString manifestPath = QDir(preparingDirectory).filePath(
        QLatin1String(ManifestFileName));

    if (!writeManifest(manifestPath, manifest, &result.error) ||
        !syncDirectory(preparingDirectory, &result.error) ||
        QFileInfo::exists(result.transactionDirectory) ||
        !QFile::rename(preparingDirectory, result.transactionDirectory) ||
        !syncDirectory(target.transactionHome, &result.error))
    {
        if (result.error.isEmpty())
        {
            result.error = QLatin1String("The prepared profile publication could not be committed");
        }

        return result;
    }

    result.success = true;
    reportProgress(progress, QLatin1String("Profile backup and publication plan ready"), 1, 1);
    return result;
}

PrivacyProfilePublicationResult PrivacyProfilePublication::applyPending(
    const QString& transactionHome)
{
    PrivacyProfilePublicationResult result;
    const QDir home(transactionHome);
    const QStringList transactions = home.entryList(
        QStringList { QLatin1String("pending-*") }, QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name);
    QStringList pending;

    for (const QString& name : transactions)
    {
        const QString path = home.filePath(name + QLatin1Char('/') +
                                           QLatin1String(ManifestFileName));
        QJsonObject manifest;
        QString error;

        if (!readManifest(path, &manifest, &error))
        {
            result.error = error;
            return result;
        }

        if (manifest.value(QLatin1String("phase")).toString() != QLatin1String("complete"))
        {
            pending << name;
        }
    }

    if (pending.isEmpty())
    {
        result.success = true;
        return result;
    }

    if (pending.size() != 1)
    {
        result.error = QLatin1String("More than one profile publication is pending");
        return result;
    }

    result.transactionDirectory = home.filePath(pending.constFirst());
    const QString manifestPath = QDir(result.transactionDirectory).filePath(
        QLatin1String(ManifestFileName));
    QJsonObject manifest;

    if (!readManifest(manifestPath, &manifest, &result.error))
    {
        return result;
    }

    result.transactionUuid = manifest.value(QLatin1String("uuid")).toString();
    result.backupDirectory = manifest.value(QLatin1String("backupDirectory")).toString();
    const QString displaced = QDir(result.transactionDirectory).filePath(
        QLatin1String("displaced"));
    QString phase = manifest.value(QLatin1String("phase")).toString();

    if (phase == QLatin1String("ready"))
    {
        const QString candidateCore = manifest.value(QLatin1String("candidateCore")).toString();
        const PrivacyProfileSummary candidate =
            PrivacyProfileInspector::inspectCoreDatabase(candidateCore);

        if (!candidate.isPrivateProfile() || !candidate.integrityOk ||
            (candidate.incompletePrivacyTransactionCount > 0) ||
            !publishFile(candidateCore,
                         manifest.value(QLatin1String("targetCore")).toString(),
                         displaced, &result.error) ||
            !updatePhase(manifestPath, &manifest, QLatin1String("core-published"),
                         &result.error))
        {
            return result;
        }

        phase = QLatin1String("core-published");
    }

    if (phase == QLatin1String("core-published"))
    {
        const QString candidateThumbs =
            manifest.value(QLatin1String("candidateThumbs")).toString();
        const QString targetThumbs = manifest.value(QLatin1String("targetThumbs")).toString();

        if ((!candidateThumbs.isEmpty() &&
             !publishFile(candidateThumbs, targetThumbs, displaced, &result.error)) ||
            (candidateThumbs.isEmpty() &&
             (!displaceFile(targetThumbs, displaced, &result.error) ||
              !displaceFile(targetThumbs + QLatin1String("-wal"), displaced, &result.error) ||
              !displaceFile(targetThumbs + QLatin1String("-shm"), displaced, &result.error))))
        {
            return result;
        }

        const QString dataHome = manifest.value(QLatin1String("targetDataHome")).toString();

        if (!displaceFile(QDir(dataHome).filePath(QLatin1String("recognition.db")),
                          displaced, &result.error) ||
            !displaceFile(QDir(dataHome).filePath(QLatin1String("similarity.db")),
                          displaced, &result.error) ||
            !updatePhase(manifestPath, &manifest, QLatin1String("databases-published"),
                         &result.error))
        {
            return result;
        }

        phase = QLatin1String("databases-published");
    }

    if (phase == QLatin1String("databases-published"))
    {
        if (!publishFile(manifest.value(QLatin1String("candidateConfig")).toString(),
                         manifest.value(QLatin1String("targetConfig")).toString(),
                         displaced, &result.error) ||
            !updatePhase(manifestPath, &manifest, QLatin1String("complete"),
                         &result.error))
        {
            return result;
        }
    }

    result.success = true;
    return result;
}

PrivacyProfilePublicationResult PrivacyProfilePublication::applyPendingFromEnvironment()
{
    const QString transactionHome = QFile::decodeName(
        qgetenv("DIGIKAM_PRIVATE_TRANSACTION_HOME"));

    if (transactionHome.isEmpty())
    {
        PrivacyProfilePublicationResult result;
        result.success = true;
        return result;
    }

    return applyPending(transactionHome);
}

QList<PrivacyProfileBackup> PrivacyProfilePublication::restorableBackups(
    const QString& transactionHome)
{
    QList<PrivacyProfileBackup> backups;
    const QDir home(transactionHome);
    const QStringList transactions = home.entryList(
        QStringList { QLatin1String("pending-*") }, QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Time | QDir::Reversed);

    for (const QString& name : transactions)
    {
        const QString transactionDirectory = home.filePath(name);
        QJsonObject manifest;
        QString error;

        if (!readManifest(QDir(transactionDirectory).filePath(
                              QLatin1String(ManifestFileName)),
                          &manifest, &error) ||
            (manifest.value(QLatin1String("phase")).toString() !=
             QLatin1String("complete")))
        {
            continue;
        }

        PrivacyProfileBackup backup;
        backup.transactionUuid = manifest.value(QLatin1String("uuid")).toString();
        backup.transactionDirectory = transactionDirectory;
        backup.backupDirectory = manifest.value(QLatin1String("backupDirectory")).toString();
        backup.configFilePath = manifest.value(QLatin1String("backupConfig")).toString();
        backup.coreDatabasePath = manifest.value(QLatin1String("backupCore")).toString();
        backup.thumbnailDatabasePath = manifest.value(QLatin1String("backupThumbs")).toString();
        backup.createdAt = QDateTime::fromString(
            manifest.value(QLatin1String("createdAt")).toString(), Qt::ISODateWithMs);
        backup.summary = PrivacyProfileInspector::inspectCoreDatabase(
            backup.coreDatabasePath, backup.configFilePath,
            backup.thumbnailDatabasePath);

        if (backup.summary.isPrivateProfile() && backup.summary.integrityOk &&
            (backup.summary.incompletePrivacyTransactionCount == 0) &&
            QFileInfo::exists(backup.configFilePath))
        {
            backups << backup;
        }
    }

    return backups;
}

PrivacyProfilePublicationResult PrivacyProfilePublication::prepareRestore(
    const PrivacyProfileBackup& backup,
    const PrivacyProfilePaths& target,
    const Progress& progress)
{
    PrivacyProfileImportStageResult staged;
    staged.success = backup.summary.isPrivateProfile() && backup.summary.integrityOk &&
                     (backup.summary.incompletePrivacyTransactionCount == 0) &&
                     QFileInfo::exists(backup.coreDatabasePath) &&
                     QFileInfo::exists(backup.configFilePath);
    staged.candidateCoreDatabasePath = backup.coreDatabasePath;
    staged.candidateSummary = backup.summary;
    staged.thumbnailDatabaseIncluded = QFileInfo::exists(backup.thumbnailDatabasePath);
    staged.candidateThumbnailDatabasePath = staged.thumbnailDatabaseIncluded
                                          ? backup.thumbnailDatabasePath
                                          : QString();

    if (!staged.success)
    {
        PrivacyProfilePublicationResult result;
        result.error = QLatin1String("The selected profile backup is not restorable");
        return result;
    }

    return prepare(staged, target, backup.configFilePath, progress);
}

} // namespace Digikam
