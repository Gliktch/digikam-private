/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// C++ includes

#include <cerrno>
#include <utility>

// POSIX includes

#if defined(__linux__)
#   include <fcntl.h>
#   include <poll.h>
#   include <sys/stat.h>
#   include <sys/syscall.h>
#   include <unistd.h>
#endif

// Qt includes

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStringList>
#include <QUuid>

// Local includes

#include "privacystillitemtransaction.h"

using namespace Digikam;

namespace
{

constexpr int ParentPollMilliseconds = 2000;

void notifyFailure(const QString& detail)
{
    QProcess::startDetached(
        QStringLiteral("notify-send"),
        { QStringLiteral("-t"), QStringLiteral("0"),
          QStringLiteral("digiKam Private needs attention"),
          QStringLiteral("Compatibility relock could not be completed: ") +
              detail });
}

bool parsePositiveLongLong(const QString& value, qlonglong* const result)
{
    bool okay = false;
    const qlonglong parsed = value.toLongLong(&okay);

    if (!okay || (parsed <= 0) || !result)
    {
        return false;
    }

    *result = parsed;
    return true;
}

bool parsePositiveUnsignedLongLong(const QString& value,
                                   quint64* const result)
{
    bool okay = false;
    const quint64 parsed = value.toULongLong(&okay);

    if (!okay || (parsed == 0) || !result)
    {
        return false;
    }

    *result = parsed;
    return true;
}

bool journalIsComplete(const PrivacyStorageRoot& root,
                       const PrivacyJournalRootExpectation& expectation,
                       const QString& transactionUuid)
{
    PrivacyJournalError error = PrivacyJournalError::None;
    QString detail;
    std::unique_ptr<PrivacyTransactionJournalStore> store =
        PrivacyTransactionJournalStore::open(
            root.configuredPath, expectation, &error, &detail);

    if (!store)
    {
        return false;
    }

    const PrivacyJournalLoadResult loaded = store->load(transactionUuid);
    return ((loaded.disposition == PrivacyJournalLoadDisposition::Loaded) &&
            loaded.authoritative && loaded.hasRecord &&
            (loaded.record.stage == PrivacyJournalStage::Complete));
}

PrivacyStillItemTransactionResult relockAllCompatibility(
    const PrivacyStorageRoot& root,
    const PrivacyJournalRootExpectation& expectation)
{
    PrivacyStillItemTransactionResult aggregate;
    aggregate.status = PrivacyStillItemTransactionStatus::CompatibilityRelocked;
    PrivacyJournalError error = PrivacyJournalError::None;
    QString detail;
    std::unique_ptr<PrivacyTransactionJournalStore> store =
        PrivacyTransactionJournalStore::open(
            root.configuredPath, expectation, &error, &detail);

    if (!store)
    {
        aggregate.status = PrivacyStillItemTransactionStatus::RootUnavailable;
        aggregate.detail = detail.isEmpty()
                         ? QStringLiteral("the collection root is unavailable")
                         : detail;
        return aggregate;
    }

    QStringList transactionUuids;

    if (!store->transactionUuids(&transactionUuids, &error, &detail))
    {
        aggregate.status = PrivacyStillItemTransactionStatus::JournalFailure;
        aggregate.detail = detail.isEmpty()
                         ? QStringLiteral("the transaction journals cannot be enumerated")
                         : detail;
        return aggregate;
    }

    QStringList failures;

    for (const QString& transactionUuid : std::as_const(transactionUuids))
    {
        const PrivacyJournalLoadResult loaded = store->load(transactionUuid);

        if ((loaded.disposition != PrivacyJournalLoadDisposition::Loaded) ||
            !loaded.authoritative || !loaded.hasRecord)
        {
            failures << transactionUuid;
            continue;
        }

        if ((loaded.record.transactionType !=
             PrivacyTransactionType::CompatibilityUnlock) ||
            (loaded.record.stage == PrivacyJournalStage::Complete))
        {
            continue;
        }

        const PrivacyStillItemTransactionResult relocked =
            PrivacyCompatibilityExposureGuardEngine::relock(
                root, expectation, transactionUuid);

        if (!relocked.succeeded())
        {
            failures << transactionUuid;

            if (relocked.status ==
                PrivacyStillItemTransactionStatus::ReconciliationRequired)
            {
                aggregate.status = relocked.status;
            }
        }
    }

    if (!failures.isEmpty())
    {
        if (aggregate.status ==
            PrivacyStillItemTransactionStatus::CompatibilityRelocked)
        {
            aggregate.status = PrivacyStillItemTransactionStatus::RecoveryRequired;
        }

        aggregate.detail = QStringLiteral(
            "%1 Compatibility journal(s) require application recovery")
                               .arg(failures.size());
    }

    return aggregate;
}

bool acknowledgeReady(const QString& path, const QString& token)
{
#if !defined(__linux__)
    Q_UNUSED(path);
    Q_UNUSED(token);
    return false;
#else
    if (path.isEmpty() || !QDir::isAbsolutePath(path) ||
        (QDir::cleanPath(path) != path) || token.isEmpty())
    {
        return false;
    }

    const QByteArray encodedPath = QFile::encodeName(path);
    const QByteArray bytes = token.toUtf8();
    const int descriptor = ::open(
        encodedPath.constData(), O_WRONLY | O_TRUNC | O_CLOEXEC | O_NOFOLLOW);
    struct stat status = {};

    if ((descriptor < 0) || (::fstat(descriptor, &status) != 0) ||
        !S_ISREG(status.st_mode) || (status.st_uid != ::getuid()) ||
        (status.st_nlink != 1) || ((status.st_mode & 0777) != 0600))
    {
        if (descriptor >= 0)
        {
            ::close(descriptor);
        }

        return false;
    }

    qsizetype written = 0;

    while (written < bytes.size())
    {
        const ssize_t count = ::write(
            descriptor, bytes.constData() + written,
            static_cast<size_t>(bytes.size() - written));

        if (count < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            ::close(descriptor);
            return false;
        }

        written += count;
    }

    const bool durable = (::fsync(descriptor) == 0);
    ::close(descriptor);
    return durable;
#endif
}

int runGuard(const PrivacyStorageRoot& root,
             const PrivacyJournalRootExpectation& expectation,
             const QString& transactionUuid, bool allCompatibility,
             qlonglong parentPid,
             const QString& readyPath, const QString& readyToken)
{
#if !defined(__linux__) || !defined(SYS_pidfd_open)
    Q_UNUSED(root);
    Q_UNUSED(expectation);
    Q_UNUSED(transactionUuid);
    Q_UNUSED(allCompatibility);
    Q_UNUSED(parentPid);
    Q_UNUSED(readyPath);
    Q_UNUSED(readyToken);
    return 10;
#else
    const int parentFd = static_cast<int>(
        ::syscall(SYS_pidfd_open, static_cast<pid_t>(parentPid), 0));

    if ((parentFd < 0) && (errno != ESRCH))
    {
        return 11;
    }

    if (!acknowledgeReady(readyPath, readyToken))
    {
        if (parentFd >= 0)
        {
            ::close(parentFd);
        }

        return 13;
    }

    if (parentFd >= 0)
    {
        struct pollfd descriptor = {};
        descriptor.fd = parentFd;
        descriptor.events = POLLIN;

        for (;;)
        {
            const int polled = ::poll(&descriptor, 1, ParentPollMilliseconds);

            if (polled > 0)
            {
                break;
            }

            if ((polled < 0) && (errno != EINTR))
            {
                ::close(parentFd);
                return 12;
            }

            if (!allCompatibility && (polled == 0) &&
                journalIsComplete(root, expectation, transactionUuid))
            {
                const PrivacyStillItemTransactionResult settled =
                    PrivacyCompatibilityExposureGuardEngine::relock(
                        root, expectation, transactionUuid);

                if (settled.succeeded())
                {
                    ::close(parentFd);
                    return 0;
                }
            }
        }

        ::close(parentFd);
    }

    const PrivacyStillItemTransactionResult relocked = allCompatibility
        ? relockAllCompatibility(root, expectation)
        : PrivacyCompatibilityExposureGuardEngine::relock(
              root, expectation, transactionUuid);

    if (relocked.succeeded())
    {
        return 0;
    }

    notifyFailure(relocked.detail);
    return (relocked.status ==
            PrivacyStillItemTransactionStatus::ReconciliationRequired)
         ? 20
         : 21;
#endif
}

} // namespace

int main(int argc, char** argv)
{
    QCoreApplication application(argc, argv);
    QCoreApplication::setApplicationName(
        QStringLiteral("digikam-private-guard"));
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("digiKam Private Compatibility exposure guard"));
    parser.addHelpOption();
    const QCommandLineOption parentPid(
        QStringLiteral("parent-pid"), QStringLiteral("Parent process ID"),
        QStringLiteral("pid"));
    const QCommandLineOption rootPath(
        QStringLiteral("root-path"), QStringLiteral("Collection root path"),
        QStringLiteral("path"));
    const QCommandLineOption rootUuid(
        QStringLiteral("root-uuid"), QStringLiteral("Collection root UUID"),
        QStringLiteral("uuid"));
    const QCommandLineOption rootMarkerUuid(
        QStringLiteral("root-marker-uuid"),
        QStringLiteral("Optional collection root marker UUID"),
        QStringLiteral("uuid"));
    const QCommandLineOption rootIdentity(
        QStringLiteral("root-identity"),
        QStringLiteral("Base64 collection identity"),
        QStringLiteral("base64"));
    const QCommandLineOption albumRootId(
        QStringLiteral("album-root-id"), QStringLiteral("Album root ID"),
        QStringLiteral("id"));
    const QCommandLineOption rootDevice(
        QStringLiteral("root-device"), QStringLiteral("Root device number"),
        QStringLiteral("device"));
    const QCommandLineOption rootInode(
        QStringLiteral("root-inode"), QStringLiteral("Root inode number"),
        QStringLiteral("inode"));
    const QCommandLineOption transactionUuid(
        QStringLiteral("transaction-uuid"),
        QStringLiteral("Compatibility Unlock transaction UUID"),
        QStringLiteral("uuid"));
    const QCommandLineOption allCompatibility(
        QStringLiteral("all-compatibility"),
        QStringLiteral("Guard every Compatibility Unlock journal on this root"));
    const QCommandLineOption readyFile(
        QStringLiteral("ready-file"),
        QStringLiteral("Secure parent readiness file"),
        QStringLiteral("path"));
    const QCommandLineOption readyToken(
        QStringLiteral("ready-token"),
        QStringLiteral("One-time parent readiness token"),
        QStringLiteral("uuid"));
    parser.addOptions({ parentPid, rootPath, rootUuid, rootMarkerUuid,
                        rootIdentity, albumRootId, rootDevice, rootInode,
                        transactionUuid, allCompatibility, readyFile, readyToken });
    parser.process(application);

    qlonglong parsedParentPid = 0;
    qlonglong parsedAlbumRootId = 0;
    quint64 parsedDevice = 0;
    quint64 parsedInode = 0;
    const QByteArray identityData = QByteArray::fromBase64(
        parser.value(rootIdentity).toLatin1(),
        QByteArray::AbortOnBase64DecodingErrors);
    const QString configuredPath = QDir::cleanPath(parser.value(rootPath));
    const QString readyPath = QDir::cleanPath(parser.value(readyFile));
    const QUuid parsedReadyToken(parser.value(readyToken));
    const bool guardAllCompatibility = parser.isSet(allCompatibility);
    const QUuid parsedTransactionUuid(parser.value(transactionUuid));

    if (!parsePositiveLongLong(parser.value(parentPid), &parsedParentPid) ||
        !parsePositiveLongLong(parser.value(albumRootId),
                               &parsedAlbumRootId) ||
        !parsePositiveUnsignedLongLong(parser.value(rootDevice),
                                       &parsedDevice) ||
        !parsePositiveUnsignedLongLong(parser.value(rootInode),
                                       &parsedInode) ||
        identityData.isEmpty() || configuredPath.isEmpty() ||
        !QDir::isAbsolutePath(configuredPath) || readyPath.isEmpty() ||
        !QDir::isAbsolutePath(readyPath) ||
        parsedReadyToken.isNull() ||
        (guardAllCompatibility == !parsedTransactionUuid.isNull()) ||
        (!guardAllCompatibility &&
         (parsedTransactionUuid.toString(QUuid::WithoutBraces).toLower() !=
          parser.value(transactionUuid))) ||
        (parsedReadyToken.toString(QUuid::WithoutBraces).toLower() !=
         parser.value(readyToken)))
    {
        return 2;
    }

    PrivacyStorageRoot root;
    root.uuid = parser.value(rootUuid);
    root.kind = PrivacyStorageRootKind::AlbumRoot;
    root.albumRootId = static_cast<int>(parsedAlbumRootId);
    root.configuredPath = configuredPath;
    root.markerUuid = parser.value(rootMarkerUuid);
    root.identityVersion = 1;
    root.identityData = identityData;
    root.schemaVersion = 1;
    root.createdAt = QDateTime::currentDateTimeUtc();
    PrivacyJournalRootExpectation expectation;
    expectation.rootUuid = root.uuid;
    expectation.markerUuid = root.markerUuid;
    expectation.identitySha256 = QCryptographicHash::hash(
        root.identityData, QCryptographicHash::Sha256);
    expectation.device = parsedDevice;
    expectation.inode = parsedInode;

    if (!root.isValid())
    {
        return 3;
    }

    return runGuard(root, expectation, parser.value(transactionUuid),
                    guardAllCompatibility, parsedParentPid, readyPath,
                    parser.value(readyToken));
}
