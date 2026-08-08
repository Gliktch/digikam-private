/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacypublictransition.h"
#include "privacyposixstorage_p.h"

// C++ includes

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <utility>

// Qt includes

#include <QCryptographicHash>
#include <QByteArrayView>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>

#if defined(Q_OS_LINUX)

// POSIX includes

#   include <fcntl.h>
#   include <sys/stat.h>
#   include <sys/types.h>
#   include <unistd.h>

#endif

namespace Digikam
{

namespace
{

constexpr qsizetype IoChunkBytes = 1024 * 1024;

class ScopedFd
{
public:

    explicit ScopedFd(int descriptor = -1)
        : m_descriptor(descriptor)
    {
    }

    ~ScopedFd()
    {
#if defined(Q_OS_LINUX)
        if (m_descriptor >= 0)
        {
            ::close(m_descriptor);
        }
#endif
    }

    ScopedFd(ScopedFd&& other) noexcept
        : m_descriptor(other.release())
    {
    }

    ScopedFd& operator=(ScopedFd&& other) noexcept
    {
        if (this != &other)
        {
            reset(other.release());
        }

        return *this;
    }

    ScopedFd(const ScopedFd&)            = delete;
    ScopedFd& operator=(const ScopedFd&) = delete;

    int get() const
    {
        return m_descriptor;
    }

    int release()
    {
        const int descriptor = m_descriptor;
        m_descriptor = -1;
        return descriptor;
    }

    void reset(int descriptor = -1)
    {
#if defined(Q_OS_LINUX)
        if (m_descriptor >= 0)
        {
            ::close(m_descriptor);
        }
#endif
        m_descriptor = descriptor;
    }

private:

    int m_descriptor = -1;
};

struct ParentComponent
{
    QByteArray name;
    quint64    device = 0;
    quint64    inode  = 0;
};

struct PinnedParent
{
    ScopedFd               descriptor;
    QList<ParentComponent> components;
    quint64                device = 0;
    quint64                inode  = 0;
};

enum class FileOpenStatus
{
    Missing,
    Verified,
    Unsafe,
    Hardlinked,
    IoFailure,
    FactMismatch
};

struct VerifiedFile
{
    ScopedFd   descriptor;
    quint64    device = 0;
    quint64    inode  = 0;
    quint64    linkCount = 0;
    qlonglong  size   = -1;
    QByteArray sha256;
};

PrivacyPublicTransitionResult failure(PrivacyPublicTransitionError error,
                                      const QString& detail)
{
    PrivacyPublicTransitionResult result;
    result.error  = error;
    result.detail = detail;
    return result;
}

bool presentFact(const PrivacyJournalObjectFact& fact)
{
    return ((fact.presence == PrivacyJournalExpectedPresence::Present) &&
            (fact.size >= 0) && (fact.linkCount >= 1) &&
            (fact.sha256.size() == QCryptographicHash::hashLength(
                QCryptographicHash::Sha256)));
}

const PrivacyJournalObjectFact* factFor(
    const PrivacyJournalAsset& asset,
    PrivacyPublicTransitionFactKind kind)
{
    switch (kind)
    {
        case PrivacyPublicTransitionFactKind::Original:
            return &asset.original;

        case PrivacyPublicTransitionFactKind::Proxy:
            return &asset.proxy;
    }

    return nullptr;
}

bool validFactMapping(PrivacyTransactionType type,
                      PrivacyPublicTransitionFactKind current,
                      PrivacyPublicTransitionFactKind installed)
{
    switch (type)
    {
        case PrivacyTransactionType::ProtectItem:
        case PrivacyTransactionType::CompatibilityRelock:
            return ((current == PrivacyPublicTransitionFactKind::Original) &&
                    (installed == PrivacyPublicTransitionFactKind::Proxy));

        case PrivacyTransactionType::UnprotectItem:
        case PrivacyTransactionType::CompatibilityUnlock:
            return ((current == PrivacyPublicTransitionFactKind::Proxy) &&
                    (installed == PrivacyPublicTransitionFactKind::Original));

        default:
            return false;
    }
}

QString parentPath(const QString& relativePath)
{
    const qsizetype slash = relativePath.lastIndexOf(QLatin1Char('/'));
    return (slash < 0) ? QString() : relativePath.first(slash);
}

QString baseName(const QString& relativePath)
{
    const qsizetype slash = relativePath.lastIndexOf(QLatin1Char('/'));
    return (slash < 0) ? relativePath : relativePath.sliced(slash + 1);
}

#if defined(Q_OS_LINUX)

bool sameStableFile(const struct stat& left, const struct stat& right)
{
    return ((left.st_dev == right.st_dev) &&
            (left.st_ino == right.st_ino) &&
            (left.st_mode == right.st_mode) &&
            (left.st_nlink == right.st_nlink) &&
            (left.st_size == right.st_size) &&
            (left.st_mtim.tv_sec == right.st_mtim.tv_sec) &&
            (left.st_mtim.tv_nsec == right.st_mtim.tv_nsec) &&
            (left.st_ctim.tv_sec == right.st_ctim.tv_sec) &&
            (left.st_ctim.tv_nsec == right.st_ctim.tv_nsec));
}

bool openPinnedRoot(const QString& path,
                    const PrivacyJournalRootExpectation& expectation,
                    ScopedFd* const root, quint64* const device,
                    quint64* const inode, QString* const detail)
{
    if (path.isEmpty() || !QDir::isAbsolutePath(path) ||
        (QDir::cleanPath(path) != path))
    {
        *detail = QStringLiteral("root path is not absolute and canonical");
        return false;
    }

    const QFileInfo info(path);

    if (!info.isDir() || info.isSymLink() ||
        (info.canonicalFilePath() != info.absoluteFilePath()))
    {
        *detail = QStringLiteral("root path is missing or symlinked");
        return false;
    }

    const QByteArray encoded = QFile::encodeName(path);
    root->reset(::open(encoded.constData(),
                       O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));

    if (root->get() < 0)
    {
        *detail = QStringLiteral("cannot open root descriptor");
        return false;
    }

    struct stat facts = {};

    if ((::fstat(root->get(), &facts) != 0) || !S_ISDIR(facts.st_mode) ||
        (static_cast<quint64>(facts.st_dev) != expectation.device) ||
        (static_cast<quint64>(facts.st_ino) != expectation.inode))
    {
        *detail = QStringLiteral("opened root does not match expected identity");
        return false;
    }

    *device = static_cast<quint64>(facts.st_dev);
    *inode  = static_cast<quint64>(facts.st_ino);
    return true;
}

bool rootStillPinned(int rootFd, quint64 device, quint64 inode)
{
    struct stat facts = {};
    return ((::fstat(rootFd, &facts) == 0) && S_ISDIR(facts.st_mode) &&
            (static_cast<quint64>(facts.st_dev) == device) &&
            (static_cast<quint64>(facts.st_ino) == inode));
}

bool rootPathStillMatches(const QString& path, quint64 device, quint64 inode)
{
    const QFileInfo info(path);

    if (!info.isDir() || info.isSymLink() ||
        (info.canonicalFilePath() != info.absoluteFilePath()))
    {
        return false;
    }

    const QByteArray encoded = QFile::encodeName(path);
    ScopedFd descriptor(::open(encoded.constData(),
                               O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));
    struct stat facts = {};
    return ((descriptor.get() >= 0) && (::fstat(descriptor.get(), &facts) == 0) &&
            S_ISDIR(facts.st_mode) &&
            (static_cast<quint64>(facts.st_dev) == device) &&
            (static_cast<quint64>(facts.st_ino) == inode));
}

bool openPinnedParent(int rootFd, quint64 rootDevice,
                      const QString& relativeParent,
                      PinnedParent* const parent, QString* const detail)
{
    ScopedFd current(::dup(rootFd));

    if (current.get() < 0)
    {
        *detail = QStringLiteral("cannot duplicate root descriptor");
        return false;
    }

    const QStringList components = relativeParent.isEmpty()
                                 ? QStringList()
                                 : relativeParent.split(QLatin1Char('/'));

    for (const QString& component : components)
    {
        const QByteArray encoded = component.toUtf8();
        ScopedFd next(PrivacyPosixStorage::confinedOpenAt(
            current.get(), encoded,
            O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));

        if (next.get() < 0)
        {
            *detail = QStringLiteral("cannot safely open public parent component");
            return false;
        }

        struct stat facts = {};

        if ((::fstat(next.get(), &facts) != 0) || !S_ISDIR(facts.st_mode) ||
            (static_cast<quint64>(facts.st_dev) != rootDevice))
        {
            *detail = QStringLiteral("public parent crosses a filesystem or is not a directory");
            return false;
        }

        ParentComponent pinned;
        pinned.name   = encoded;
        pinned.device = static_cast<quint64>(facts.st_dev);
        pinned.inode  = static_cast<quint64>(facts.st_ino);
        parent->components.append(pinned);
        current.reset(next.release());
    }

    struct stat parentFacts = {};

    if ((::fstat(current.get(), &parentFacts) != 0) ||
        !S_ISDIR(parentFacts.st_mode) ||
        (static_cast<quint64>(parentFacts.st_dev) != rootDevice))
    {
        *detail = QStringLiteral("public parent identity is invalid");
        return false;
    }

    parent->device = static_cast<quint64>(parentFacts.st_dev);
    parent->inode  = static_cast<quint64>(parentFacts.st_ino);
    parent->descriptor.reset(current.release());
    return true;
}

bool parentStillReachable(int rootFd, quint64 rootDevice,
                          const PinnedParent& parent)
{
    ScopedFd current(::dup(rootFd));

    if (current.get() < 0)
    {
        return false;
    }

    for (const ParentComponent& component : parent.components)
    {
        ScopedFd next(PrivacyPosixStorage::confinedOpenAt(
            current.get(), component.name,
            O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));

        if (next.get() < 0)
        {
            return false;
        }

        struct stat facts = {};

        if ((::fstat(next.get(), &facts) != 0) || !S_ISDIR(facts.st_mode) ||
            (static_cast<quint64>(facts.st_dev) != rootDevice) ||
            (static_cast<quint64>(facts.st_dev) != component.device) ||
            (static_cast<quint64>(facts.st_ino) != component.inode))
        {
            return false;
        }

        current.reset(next.release());
    }

    struct stat finalFacts = {};
    return ((::fstat(current.get(), &finalFacts) == 0) &&
            S_ISDIR(finalFacts.st_mode) &&
            (static_cast<quint64>(finalFacts.st_dev) == parent.device) &&
            (static_cast<quint64>(finalFacts.st_ino) == parent.inode));
}

FileOpenStatus classifyFailedOpen(int directoryFd, const QByteArray& name,
                                  quint64 expectedDevice, bool strict0600)
{
    if (errno == ENOENT)
    {
        return FileOpenStatus::Missing;
    }

    struct stat facts = {};

    if (::fstatat(directoryFd, name.constData(), &facts,
                  AT_SYMLINK_NOFOLLOW) == 0)
    {
        if (!S_ISREG(facts.st_mode) ||
            (static_cast<quint64>(facts.st_dev) != expectedDevice) ||
            (facts.st_uid != ::geteuid()) || (facts.st_nlink != 1) ||
            (strict0600 && ((facts.st_mode & 0777) != 0600)))
        {
            return FileOpenStatus::Unsafe;
        }
    }

    return FileOpenStatus::IoFailure;
}

FileOpenStatus openAndVerifyFile(int directoryFd, const QByteArray& name,
                                 quint64 expectedDevice,
                                 const PrivacyJournalObjectFact& expected,
                                 bool writable, bool strict0600,
                                 VerifiedFile* const verified)
{
    const int flags = (writable ? O_RDWR : O_RDONLY) | O_CLOEXEC |
                      O_NOFOLLOW | O_NONBLOCK;
    ScopedFd descriptor(PrivacyPosixStorage::confinedOpenAt(directoryFd, name,
                                                            flags));

    if (descriptor.get() < 0)
    {
        return classifyFailedOpen(directoryFd, name, expectedDevice, strict0600);
    }

    struct stat before = {};

    if ((::fstat(descriptor.get(), &before) != 0) ||
        !S_ISREG(before.st_mode) ||
        (static_cast<quint64>(before.st_dev) != expectedDevice) ||
        (before.st_uid != ::geteuid()) || (before.st_nlink < 1) ||
        (strict0600 && ((before.st_mode & 0777) != 0600)))
    {
        return FileOpenStatus::Unsafe;
    }

    if (static_cast<quint64>(before.st_nlink) != expected.linkCount)
    {
        return FileOpenStatus::Hardlinked;
    }

    if (!presentFact(expected) ||
        (static_cast<qlonglong>(before.st_size) != expected.size))
    {
        return FileOpenStatus::FactMismatch;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    QByteArray buffer(IoChunkBytes, Qt::Uninitialized);
    qlonglong offset = 0;

    while (offset < expected.size)
    {
        const qsizetype wanted = static_cast<qsizetype>(
            std::min<qlonglong>(buffer.size(), expected.size - offset));
        const ssize_t count = ::pread(descriptor.get(), buffer.data(),
                                      static_cast<size_t>(wanted), offset);

        if (count < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            return FileOpenStatus::IoFailure;
        }

        if (count == 0)
        {
            return FileOpenStatus::FactMismatch;
        }

        hash.addData(QByteArrayView(buffer.constData(),
                                    static_cast<qsizetype>(count)));
        offset += static_cast<qlonglong>(count);
    }

    struct stat after = {};

    if ((::fstat(descriptor.get(), &after) != 0) ||
        !sameStableFile(before, after) || (hash.result() != expected.sha256))
    {
        return FileOpenStatus::FactMismatch;
    }

    verified->device     = static_cast<quint64>(before.st_dev);
    verified->inode      = static_cast<quint64>(before.st_ino);
    verified->linkCount  = static_cast<quint64>(before.st_nlink);
    verified->size       = static_cast<qlonglong>(before.st_size);
    verified->sha256     = hash.result();
    verified->descriptor.reset(descriptor.release());
    return FileOpenStatus::Verified;
}

bool nameStillBindsFile(int directoryFd, const QByteArray& name,
                        const VerifiedFile& verified)
{
    struct stat facts = {};
    return ((::fstatat(directoryFd, name.constData(), &facts,
                       AT_SYMLINK_NOFOLLOW) == 0) &&
            S_ISREG(facts.st_mode) &&
            (static_cast<quint64>(facts.st_nlink) == verified.linkCount) &&
            (static_cast<quint64>(facts.st_dev) == verified.device) &&
            (static_cast<quint64>(facts.st_ino) == verified.inode));
}

bool nameIsAbsent(int directoryFd, const QByteArray& name)
{
    struct stat facts = {};

    if (::fstatat(directoryFd, name.constData(), &facts,
                  AT_SYMLINK_NOFOLLOW) == 0)
    {
        return false;
    }

    return (errno == ENOENT);
}

#endif // Q_OS_LINUX

} // namespace

void PrivacyPublicTransitionEngine::setFaultHook(const FaultHook& hook)
{
    m_faultHook = hook;
}

QString PrivacyPublicTransitionEngine::expectedStageFileName(
    const QString& transactionUuid, int role, int ordinal)
{
    if (PrivacyTransactionJournalCodec::relativeJournalPath(transactionUuid).isEmpty() ||
        (role <= 0) || (ordinal < 0))
    {
        return {};
    }

    return QStringLiteral(".digikam-private-transition-%1-%2-%3.stage")
        .arg(transactionUuid).arg(role).arg(ordinal);
}

PrivacyPublicTransitionResult PrivacyPublicTransitionEngine::execute(
    const PrivacyPublicTransitionRequest& request) const
{
#if !defined(Q_OS_LINUX)
    Q_UNUSED(request);
    return failure(PrivacyPublicTransitionError::AtomicPublicationUnavailable,
                   QStringLiteral("public transition requires Linux renameat2"));
#else
    const auto faulted = [this](PrivacyPublicTransitionFaultPoint point)
    {
        return (m_faultHook && m_faultHook(point));
    };

    QString validationDetail;

    if (!PrivacyTransactionJournalCodec::validate(request.journalRecord,
                                                   &validationDetail) ||
        (request.authoritativeJournalSha256.size() !=
         QCryptographicHash::hashLength(QCryptographicHash::Sha256)) ||
        (request.journalRecord.stage !=
         PrivacyJournalStage::ProtectedCopyVerified) ||
        (request.itemUuid.isEmpty()) || (request.role <= 0) ||
        (request.ordinal < 0) ||
        ((request.mode != PrivacyPublicTransitionMode::InstallAbsent) &&
         (request.mode != PrivacyPublicTransitionMode::ExchangePresent)) ||
        !validFactMapping(request.journalRecord.transactionType,
                          request.currentFact, request.installedFact))
    {
        return failure(PrivacyPublicTransitionError::InvalidRequest,
                       validationDetail.isEmpty()
                           ? QStringLiteral("request or journal stage/fact mapping is invalid")
                           : validationDetail);
    }

    const PrivacyJournalAsset* asset = nullptr;

    for (const PrivacyJournalAsset& candidate : request.journalRecord.assets)
    {
        if ((candidate.itemUuid == request.itemUuid) &&
            (candidate.role == request.role) &&
            (candidate.ordinal == request.ordinal))
        {
            asset = &candidate;
            break;
        }
    }

    if (!asset || asset->stagedRelativePath.isEmpty() ||
        !presentFact(asset->container) || (asset->container.linkCount != 1) ||
        (parentPath(asset->publicRelativePath) !=
         parentPath(asset->stagedRelativePath)) ||
        (baseName(asset->stagedRelativePath) !=
         expectedStageFileName(request.journalRecord.transactionUuid,
                               request.role, request.ordinal)))
    {
        return failure(PrivacyPublicTransitionError::InvalidRequest,
                       QStringLiteral("asset paths or verified protected-copy fact are invalid"));
    }

    const PrivacyJournalObjectFact* const installedFact =
        factFor(*asset, request.installedFact);
    const PrivacyJournalObjectFact* const currentFact =
        factFor(*asset, request.currentFact);

    if (!installedFact || !currentFact || !presentFact(*installedFact) ||
        !presentFact(*currentFact) || (installedFact->linkCount != 1))
    {
        return failure(PrivacyPublicTransitionError::InvalidRequest,
                       QStringLiteral("journal does not contain required present file facts"));
    }

    if ((request.journalRecord.transactionType !=
         PrivacyTransactionType::ProtectItem) &&
        (currentFact->linkCount != 1))
    {
        return failure(PrivacyPublicTransitionError::HardlinkReconciliationRequired,
                       QStringLiteral("this transition type cannot relock or replace a multi-link public exposure"));
    }

    PrivacyJournalError journalError = PrivacyJournalError::None;
    QString journalDetail;
    std::unique_ptr<PrivacyTransactionJournalStore> journalStore =
        PrivacyTransactionJournalStore::open(
            request.absoluteRootPath, request.rootExpectation,
            &journalError, &journalDetail);

    if (!journalStore)
    {
        return failure(PrivacyPublicTransitionError::RootIdentityMismatch,
                       journalDetail);
    }

    const PrivacyJournalLoadResult loaded = journalStore->load(
        request.journalRecord.transactionUuid);
    const QByteArray suppliedCanonical = PrivacyTransactionJournalCodec::encode(
        request.journalRecord);

    if ((loaded.disposition != PrivacyJournalLoadDisposition::Loaded) ||
        !loaded.authoritative || !loaded.hasRecord ||
        (loaded.sha256 != request.authoritativeJournalSha256) ||
        (loaded.canonicalBytes != suppliedCanonical) ||
        (loaded.record.stage != PrivacyJournalStage::ProtectedCopyVerified))
    {
        return failure(PrivacyPublicTransitionError::JournalRejected,
                       QStringLiteral("journal CAS is missing, stale, uncertain, or at the wrong stage"));
    }

    if (faulted(PrivacyPublicTransitionFaultPoint::AfterJournalValidated))
    {
        return failure(PrivacyPublicTransitionError::FaultInjected,
                       QStringLiteral("fault injected after journal validation"));
    }

    ScopedFd rootFd;
    quint64 rootDevice = 0;
    quint64 rootInode  = 0;

    if (!openPinnedRoot(request.absoluteRootPath, request.rootExpectation,
                        &rootFd, &rootDevice, &rootInode, &validationDetail))
    {
        return failure(PrivacyPublicTransitionError::RootIdentityMismatch,
                       validationDetail);
    }

    if (faulted(PrivacyPublicTransitionFaultPoint::AfterRootOpened))
    {
        return failure(PrivacyPublicTransitionError::FaultInjected,
                       QStringLiteral("fault injected after root open"));
    }

    PinnedParent parent;

    if (!openPinnedParent(rootFd.get(), rootDevice,
                          parentPath(asset->publicRelativePath),
                          &parent, &validationDetail))
    {
        return failure(PrivacyPublicTransitionError::UnsafePath,
                       validationDetail);
    }

    const QByteArray publicName = baseName(asset->publicRelativePath).toUtf8();
    const QByteArray stageName  = baseName(asset->stagedRelativePath).toUtf8();
    VerifiedFile staged;
    FileOpenStatus stagedStatus = openAndVerifyFile(
        parent.descriptor.get(), stageName, rootDevice, *installedFact,
        true, true, &staged);

    if (stagedStatus != FileOpenStatus::Verified)
    {
        const PrivacyPublicTransitionError error =
            (stagedStatus == FileOpenStatus::Missing)
                ? PrivacyPublicTransitionError::MissingExpectedFile
                : (stagedStatus == FileOpenStatus::FactMismatch)
                    ? PrivacyPublicTransitionError::FileFactMismatch
                    : ((stagedStatus == FileOpenStatus::Unsafe) ||
                       (stagedStatus == FileOpenStatus::Hardlinked))
                        ? PrivacyPublicTransitionError::UnsafePath
                        : PrivacyPublicTransitionError::IoFailure;
        return failure(error, QStringLiteral("staged replacement is missing, unsafe, or mismatched"));
    }

    VerifiedFile current;

    if (request.mode == PrivacyPublicTransitionMode::ExchangePresent)
    {
        const FileOpenStatus currentStatus = openAndVerifyFile(
            parent.descriptor.get(), publicName, rootDevice, *currentFact,
            false, false, &current);

        if (currentStatus != FileOpenStatus::Verified)
        {
            const PrivacyPublicTransitionError error =
                (currentStatus == FileOpenStatus::Missing)
                    ? PrivacyPublicTransitionError::MissingExpectedFile
                    : (currentStatus == FileOpenStatus::FactMismatch)
                        ? PrivacyPublicTransitionError::FileFactMismatch
                        : (currentStatus == FileOpenStatus::Hardlinked)
                            ? PrivacyPublicTransitionError::HardlinkReconciliationRequired
                        : (currentStatus == FileOpenStatus::Unsafe)
                            ? PrivacyPublicTransitionError::UnsafePath
                            : PrivacyPublicTransitionError::IoFailure;
            return failure(error, QStringLiteral("current public file is missing, unsafe, or mismatched"));
        }
    }
    else if (!nameIsAbsent(parent.descriptor.get(), publicName))
    {
        return failure(PrivacyPublicTransitionError::UnexpectedExistingFile,
                       QStringLiteral("reserved public name is not absent"));
    }

    if (faulted(PrivacyPublicTransitionFaultPoint::AfterInitialVerification))
    {
        return failure(PrivacyPublicTransitionError::FaultInjected,
                       QStringLiteral("fault injected after initial file verification"));
    }

    if (::fsync(staged.descriptor.get()) != 0)
    {
        return failure(PrivacyPublicTransitionError::IoFailure,
                       QStringLiteral("cannot fsync staged replacement"));
    }

    if (faulted(PrivacyPublicTransitionFaultPoint::AfterStagedFsync))
    {
        return failure(PrivacyPublicTransitionError::FaultInjected,
                       QStringLiteral("fault injected after staged fsync"));
    }

    PrivacyJournalRecord applyingRecord = request.journalRecord;
    applyingRecord.stage = PrivacyJournalStage::Applying;
    QByteArray applyingHash;

    if (!journalStore->compareAndUpdate(
            applyingRecord, request.authoritativeJournalSha256,
            &applyingHash, &journalError, &journalDetail))
    {
        return failure(PrivacyPublicTransitionError::JournalAdvanceFailed,
                       journalDetail);
    }

    PrivacyPublicTransitionResult result;
    result.applyingJournalSha256 = applyingHash;

    if (faulted(PrivacyPublicTransitionFaultPoint::AfterApplyingJournal))
    {
        result.error  = PrivacyPublicTransitionError::FaultInjected;
        result.detail = QStringLiteral("fault injected after Applying journal stage");
        return result;
    }

    const PrivacyJournalLoadResult applyingLoaded = journalStore->load(
        request.journalRecord.transactionUuid);

    if ((applyingLoaded.disposition != PrivacyJournalLoadDisposition::Loaded) ||
        !applyingLoaded.authoritative ||
        (applyingLoaded.sha256 != applyingHash) ||
        (applyingLoaded.record.stage != PrivacyJournalStage::Applying) ||
        !rootStillPinned(rootFd.get(), rootDevice, rootInode) ||
        !rootPathStillMatches(request.absoluteRootPath, rootDevice, rootInode) ||
        !parentStillReachable(rootFd.get(), rootDevice, parent))
    {
        result.error  = PrivacyPublicTransitionError::RootIdentityMismatch;
        result.detail = QStringLiteral("root, parent, or Applying journal changed before mutation");
        return result;
    }

    staged = VerifiedFile();
    stagedStatus = openAndVerifyFile(parent.descriptor.get(), stageName,
                                     rootDevice, *installedFact,
                                     true, true, &staged);
    current = VerifiedFile();
    FileOpenStatus currentStatus = FileOpenStatus::Missing;

    if (request.mode == PrivacyPublicTransitionMode::ExchangePresent)
    {
        currentStatus = openAndVerifyFile(parent.descriptor.get(), publicName,
                                          rootDevice, *currentFact,
                                          false, false, &current);
    }

    if ((request.mode == PrivacyPublicTransitionMode::ExchangePresent) &&
        (currentStatus == FileOpenStatus::Hardlinked))
    {
        result.error  = PrivacyPublicTransitionError::HardlinkReconciliationRequired;
        result.detail = QStringLiteral("public source gained an unresolved hardlink alias");
        return result;
    }

    if ((stagedStatus != FileOpenStatus::Verified) ||
        ((request.mode == PrivacyPublicTransitionMode::ExchangePresent) &&
         (currentStatus != FileOpenStatus::Verified)) ||
        ((request.mode == PrivacyPublicTransitionMode::InstallAbsent) &&
         !nameIsAbsent(parent.descriptor.get(), publicName)) ||
        (::fsync(staged.descriptor.get()) != 0))
    {
        result.error  = PrivacyPublicTransitionError::FileFactMismatch;
        result.detail = QStringLiteral("immediate pre-mutation file verification failed");
        return result;
    }

    if (faulted(PrivacyPublicTransitionFaultPoint::BeforeNamespaceMutation))
    {
        result.error  = PrivacyPublicTransitionError::FaultInjected;
        result.detail = QStringLiteral("fault injected before namespace mutation");
        return result;
    }

    if (!rootStillPinned(rootFd.get(), rootDevice, rootInode) ||
        !rootPathStillMatches(request.absoluteRootPath, rootDevice, rootInode) ||
        !parentStillReachable(rootFd.get(), rootDevice, parent) ||
        !nameStillBindsFile(parent.descriptor.get(), stageName, staged) ||
        ((request.mode == PrivacyPublicTransitionMode::ExchangePresent) &&
         !nameStillBindsFile(parent.descriptor.get(), publicName, current)) ||
        ((request.mode == PrivacyPublicTransitionMode::InstallAbsent) &&
         !nameIsAbsent(parent.descriptor.get(), publicName)))
    {
        result.error  = PrivacyPublicTransitionError::RootIdentityMismatch;
        result.detail = QStringLiteral("parent or file binding changed immediately before mutation");
        return result;
    }

    bool atomicUnavailable = false;
    const bool exchange = (request.mode == PrivacyPublicTransitionMode::ExchangePresent);

    if (!PrivacyPosixStorage::atomicRenameAt(
            parent.descriptor.get(), stageName, publicName,
            exchange ? PrivacyPosixStorage::AtomicRenameMode::Exchange
                     : PrivacyPosixStorage::AtomicRenameMode::NoReplace,
            &atomicUnavailable))
    {
        result.error = atomicUnavailable
                     ? PrivacyPublicTransitionError::AtomicPublicationUnavailable
                     : PrivacyPublicTransitionError::PublicationConflict;
        result.detail = atomicUnavailable
                      ? QStringLiteral("required renameat2 operation is unavailable")
                      : QStringLiteral("atomic public-name mutation failed");
        return result;
    }

    result.namespaceMutated = true;

    if (exchange)
    {
        result.displacedRelativePath = asset->stagedRelativePath;
    }

    if (faulted(PrivacyPublicTransitionFaultPoint::AfterNamespaceMutation))
    {
        result.error  = PrivacyPublicTransitionError::DurabilityUncertain;
        result.detail = QStringLiteral("fault injected after namespace mutation before parent fsync");
        return result;
    }

    if (::fsync(parent.descriptor.get()) != 0)
    {
        result.error  = PrivacyPublicTransitionError::DurabilityUncertain;
        result.detail = QStringLiteral("parent fsync failed after namespace mutation");
        return result;
    }

    if (faulted(PrivacyPublicTransitionFaultPoint::AfterParentFsync))
    {
        result.error  = PrivacyPublicTransitionError::DurabilityUncertain;
        result.detail = QStringLiteral("fault injected after parent fsync before readback");
        return result;
    }

    if (!rootStillPinned(rootFd.get(), rootDevice, rootInode) ||
        !rootPathStillMatches(request.absoluteRootPath, rootDevice, rootInode) ||
        !parentStillReachable(rootFd.get(), rootDevice, parent))
    {
        result.error  = PrivacyPublicTransitionError::DurabilityUncertain;
        result.detail = QStringLiteral("public parent moved during namespace mutation");
        return result;
    }

    const auto rollback = [&](const QString& reason)
    {
        if (faulted(PrivacyPublicTransitionFaultPoint::BeforeRollback))
        {
            result.error  = PrivacyPublicTransitionError::RollbackUncertain;
            result.detail = reason + QStringLiteral("; rollback fault injected before rename");
            return result;
        }

        bool unavailable = false;
        const bool renamed = PrivacyPosixStorage::atomicRenameAt(
            parent.descriptor.get(), publicName, stageName,
            exchange ? PrivacyPosixStorage::AtomicRenameMode::Exchange
                     : PrivacyPosixStorage::AtomicRenameMode::NoReplace,
            &unavailable);

        if (!renamed)
        {
            result.error  = PrivacyPublicTransitionError::RollbackUncertain;
            result.detail = reason + QStringLiteral("; atomic rollback failed");
            return result;
        }

        if (faulted(PrivacyPublicTransitionFaultPoint::AfterRollback))
        {
            result.error  = PrivacyPublicTransitionError::RollbackUncertain;
            result.detail = reason + QStringLiteral("; rollback rename is not fsynced");
            return result;
        }

        if (::fsync(parent.descriptor.get()) != 0)
        {
            result.error  = PrivacyPublicTransitionError::RollbackUncertain;
            result.detail = reason + QStringLiteral("; rollback parent fsync failed");
            return result;
        }

        VerifiedFile restored;
        const bool restorationVerified = exchange
            ? (openAndVerifyFile(parent.descriptor.get(), publicName,
                                 rootDevice, *currentFact, false, false,
                                 &restored) == FileOpenStatus::Verified)
            : nameIsAbsent(parent.descriptor.get(), publicName);

        if (!restorationVerified)
        {
            result.error  = PrivacyPublicTransitionError::RollbackUncertain;
            result.detail = reason + QStringLiteral("; rolled-back public state cannot be verified");
            return result;
        }

        result.namespaceMutated = false;
        result.installedVerified = false;
        result.displacedVerified = false;
        result.error  = PrivacyPublicTransitionError::RollbackSucceeded;
        result.detail = reason + QStringLiteral("; previous public state restored durably");

        if (faulted(PrivacyPublicTransitionFaultPoint::AfterRollbackFsync))
        {
            result.detail += QStringLiteral(" (fault injected after durable rollback)");
        }

        return result;
    };

    VerifiedFile installed;

    if (openAndVerifyFile(parent.descriptor.get(), publicName, rootDevice,
                          *installedFact, false, false,
                          &installed) != FileOpenStatus::Verified)
    {
        return rollback(QStringLiteral("installed public bytes do not match journal fact"));
    }

    result.installedVerified = true;

    if (faulted(PrivacyPublicTransitionFaultPoint::AfterInstalledVerification))
    {
        result.error  = PrivacyPublicTransitionError::DurabilityUncertain;
        result.detail = QStringLiteral("fault injected after installed-byte verification");
        return result;
    }

    if (exchange)
    {
        VerifiedFile displaced;

        if (openAndVerifyFile(parent.descriptor.get(), stageName, rootDevice,
                              *currentFact, false, false,
                              &displaced) != FileOpenStatus::Verified)
        {
            return rollback(QStringLiteral("displaced public bytes do not match journal fact"));
        }

        result.displacedVerified = true;
    }
    else if (!nameIsAbsent(parent.descriptor.get(), stageName))
    {
        return rollback(QStringLiteral("staged name unexpectedly survives no-replace install"));
    }

    if (faulted(PrivacyPublicTransitionFaultPoint::AfterDisplacedVerification))
    {
        result.error  = PrivacyPublicTransitionError::DurabilityUncertain;
        result.detail = QStringLiteral("fault injected after complete namespace verification");
        return result;
    }

    PrivacyJournalRecord finalRecord = applyingRecord;
    finalRecord.stage = PrivacyJournalStage::PublicStateVerified;

    if (!journalStore->compareAndUpdate(finalRecord, applyingHash,
                                        &result.finalJournalSha256,
                                        &journalError, &journalDetail))
    {
        result.error  = PrivacyPublicTransitionError::ReconciliationRequired;
        result.detail = QStringLiteral("public namespace is durably verified but final journal advancement failed: %1")
                            .arg(journalDetail);
        return result;
    }

    if (faulted(PrivacyPublicTransitionFaultPoint::AfterPublicStateJournal))
    {
        result.error  = PrivacyPublicTransitionError::FaultInjected;
        result.detail = QStringLiteral("fault injected after durable PublicStateVerified journal");
        return result;
    }

    result.error = PrivacyPublicTransitionError::None;
    return result;
#endif
}

} // namespace Digikam
