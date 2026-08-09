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

enum class TransitionDiskState
{
    ExactPre,
    ExactPost,
    Ambiguous,
    Unknown
};

PrivacyPublicTransitionResult failure(PrivacyPublicTransitionError error,
                                      const QString& detail)
{
    PrivacyPublicTransitionResult result;
    result.error  = error;
    result.detail = detail;
    return result;
}

PrivacyPublicReplacementStageResult stageFailure(
    PrivacyPublicTransitionError error, const QString& detail)
{
    PrivacyPublicReplacementStageResult result;
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

struct ValidatedTransitionRequest
{
    const PrivacyJournalAsset*      asset         = nullptr;
    const PrivacyJournalObjectFact* installedFact = nullptr;
    const PrivacyJournalObjectFact* currentFact   = nullptr;
    bool                            install       = false;
    bool                            exchange      = false;
    bool                            remove        = false;
};

PrivacyPublicTransitionError validateTransitionRequest(
    const PrivacyPublicTransitionRequest& request,
    ValidatedTransitionRequest* const validated, QString* const detail)
{
    if (!validated || !detail)
    {
        return PrivacyPublicTransitionError::InvalidRequest;
    }

    QString journalDetail;

    if (!PrivacyTransactionJournalCodec::validate(request.journalRecord,
                                                   &journalDetail) ||
        (request.authoritativeJournalSha256.size() !=
         QCryptographicHash::hashLength(QCryptographicHash::Sha256)) ||
        ((request.journalRecord.stage !=
          PrivacyJournalStage::ProtectedCopyVerified) &&
         (request.journalRecord.stage != PrivacyJournalStage::Applying) &&
         (request.journalRecord.stage !=
          PrivacyJournalStage::PublicStateVerified)) ||
        request.itemUuid.isEmpty() || (request.role <= 0) ||
        (request.ordinal < 0) ||
        ((request.mode != PrivacyPublicTransitionMode::InstallAbsent) &&
         (request.mode != PrivacyPublicTransitionMode::ExchangePresent) &&
         (request.mode != PrivacyPublicTransitionMode::RemovePresent)) ||
        (request.installedUnixMode < -1) ||
        (request.installedUnixMode > 07777) ||
        !validFactMapping(request.journalRecord.transactionType,
                          request.currentFact, request.installedFact))
    {
        *detail = journalDetail.isEmpty()
                ? QStringLiteral(
                    "request or journal stage/fact mapping is invalid")
                : journalDetail;
        return PrivacyPublicTransitionError::InvalidRequest;
    }

    for (const PrivacyJournalAsset& candidate : request.journalRecord.assets)
    {
        if ((candidate.itemUuid == request.itemUuid) &&
            (candidate.role == request.role) &&
            (candidate.ordinal == request.ordinal))
        {
            validated->asset = &candidate;
            break;
        }
    }

    if (!validated->asset || validated->asset->stagedRelativePath.isEmpty() ||
        !presentFact(validated->asset->container) ||
        (validated->asset->container.linkCount != 1) ||
        (parentPath(validated->asset->publicRelativePath) !=
         parentPath(validated->asset->stagedRelativePath)) ||
        (baseName(validated->asset->stagedRelativePath) !=
         PrivacyPublicTransitionEngine::expectedStageFileName(
             request.journalRecord.transactionUuid, request.role,
             request.ordinal)))
    {
        *detail = QStringLiteral(
            "asset paths or verified protected-copy fact are invalid");
        return PrivacyPublicTransitionError::InvalidRequest;
    }

    validated->installedFact = factFor(*validated->asset,
                                       request.installedFact);
    validated->currentFact = factFor(*validated->asset, request.currentFact);
    validated->install = (request.mode ==
                          PrivacyPublicTransitionMode::InstallAbsent);
    validated->exchange = (request.mode ==
                           PrivacyPublicTransitionMode::ExchangePresent);
    validated->remove = (request.mode ==
                         PrivacyPublicTransitionMode::RemovePresent);

    if (!validated->installedFact || !validated->currentFact)
    {
        *detail = QStringLiteral(
            "journal does not contain required file facts");
        return PrivacyPublicTransitionError::InvalidRequest;
    }

    const bool validInstalledFact =
        (validated->install || validated->exchange)
            ? (presentFact(*validated->installedFact) &&
               (validated->installedFact->linkCount == 1))
            : (validated->installedFact->presence ==
               PrivacyJournalExpectedPresence::Absent);
    const bool validCurrentFact =
        (validated->exchange || validated->remove)
            ? presentFact(*validated->currentFact)
            : (presentFact(*validated->currentFact) ||
               (validated->currentFact->presence ==
                PrivacyJournalExpectedPresence::Absent));

    if (!validInstalledFact || !validCurrentFact ||
        (validated->remove && (request.installedUnixMode >= 0)))
    {
        *detail = QStringLiteral(
            "journal does not contain required present file facts");
        return PrivacyPublicTransitionError::InvalidRequest;
    }

    if ((request.journalRecord.transactionType !=
         PrivacyTransactionType::ProtectItem) &&
        presentFact(*validated->currentFact) &&
        (validated->currentFact->linkCount != 1))
    {
        *detail = QStringLiteral(
            "this transition type cannot relock or replace a multi-link public exposure");
        return PrivacyPublicTransitionError::HardlinkReconciliationRequired;
    }

    return PrivacyPublicTransitionError::None;
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

bool writeAll(int descriptor, const QByteArray& bytes)
{
    qsizetype offset = 0;

    while (offset < bytes.size())
    {
        const ssize_t written = ::write(
            descriptor, bytes.constData() + offset,
            static_cast<size_t>(bytes.size() - offset));

        if (written < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            return false;
        }

        if (written == 0)
        {
            return false;
        }

        offset += static_cast<qsizetype>(written);
    }

    return true;
}

bool hashStableOpenedFile(int descriptor, quint64 expectedDevice,
                          VerifiedFile* const verified)
{
    struct stat before = {};

    if ((::fstat(descriptor, &before) != 0) || !S_ISREG(before.st_mode) ||
        (static_cast<quint64>(before.st_dev) != expectedDevice) ||
        (before.st_uid != ::geteuid()) || (before.st_nlink != 1) ||
        ((before.st_mode & 0777) != 0600) || (before.st_size < 0))
    {
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    QByteArray buffer(IoChunkBytes, Qt::Uninitialized);
    qlonglong offset = 0;
    const qlonglong size = static_cast<qlonglong>(before.st_size);

    while (offset < size)
    {
        const qsizetype wanted = static_cast<qsizetype>(
            std::min<qlonglong>(buffer.size(), size - offset));
        const ssize_t count = ::pread(descriptor, buffer.data(),
                                      static_cast<size_t>(wanted), offset);

        if (count < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            return false;
        }

        if (count == 0)
        {
            return false;
        }

        hash.addData(QByteArrayView(buffer.constData(),
                                    static_cast<qsizetype>(count)));
        offset += static_cast<qlonglong>(count);
    }

    struct stat after = {};

    if ((::fstat(descriptor, &after) != 0) ||
        !sameStableFile(before, after))
    {
        return false;
    }

    verified->device    = static_cast<quint64>(before.st_dev);
    verified->inode     = static_cast<quint64>(before.st_ino);
    verified->linkCount = static_cast<quint64>(before.st_nlink);
    verified->size      = size;
    verified->sha256    = hash.result();
    return true;
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

PrivacyPublicReplacementStageResult
PrivacyPublicTransitionEngine::stageReplacement(
    const PrivacyPublicReplacementStageRequest& request,
    const QByteArray& bytes) const
{
    return stageReplacement(
        request,
        [&bytes](int descriptor, QString* const detail)
        {
#if defined(Q_OS_LINUX)
            if (writeAll(descriptor, bytes))
            {
                return true;
            }

            if (detail)
            {
                *detail = QStringLiteral("cannot write replacement bytes");
            }

            return false;
#else
            Q_UNUSED(descriptor);
            Q_UNUSED(detail);
            return false;
#endif
        });
}

PrivacyPublicReplacementStageResult
PrivacyPublicTransitionEngine::stageReplacement(
    const PrivacyPublicReplacementStageRequest& request,
    const StageProducer& producer) const
{
#if !defined(Q_OS_LINUX)
    Q_UNUSED(request);
    Q_UNUSED(producer);
    return stageFailure(PrivacyPublicTransitionError::AtomicPublicationUnavailable,
                        QStringLiteral("public replacement staging requires Linux"));
#else
    QString validationDetail;

    if (!producer ||
        !PrivacyTransactionJournalCodec::validate(request.journalRecord,
                                                   &validationDetail) ||
        (request.authoritativeJournalSha256.size() !=
         QCryptographicHash::hashLength(QCryptographicHash::Sha256)) ||
        (request.journalRecord.stage != PrivacyJournalStage::Prepared) ||
        request.itemUuid.isEmpty() || (request.role <= 0) ||
        (request.ordinal < 0))
    {
        return stageFailure(
            PrivacyPublicTransitionError::InvalidRequest,
            validationDetail.isEmpty()
                ? QStringLiteral("replacement staging request is invalid")
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
        (parentPath(asset->publicRelativePath) !=
         parentPath(asset->stagedRelativePath)) ||
        (baseName(asset->stagedRelativePath) !=
         expectedStageFileName(request.journalRecord.transactionUuid,
                               request.role, request.ordinal)))
    {
        return stageFailure(
            PrivacyPublicTransitionError::InvalidRequest,
            QStringLiteral("replacement stage path is not the expected same-parent name"));
    }

    PrivacyJournalError journalError = PrivacyJournalError::None;
    QString journalDetail;
    std::unique_ptr<PrivacyTransactionJournalStore> journalStore =
        PrivacyTransactionJournalStore::open(
            request.absoluteRootPath, request.rootExpectation,
            &journalError, &journalDetail);

    if (!journalStore)
    {
        return stageFailure(PrivacyPublicTransitionError::RootIdentityMismatch,
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
        (loaded.record.stage != PrivacyJournalStage::Prepared))
    {
        return stageFailure(
            PrivacyPublicTransitionError::JournalRejected,
            QStringLiteral("Prepared journal CAS is missing, stale, or uncertain"));
    }

    ScopedFd rootFd;
    quint64 rootDevice = 0;
    quint64 rootInode  = 0;

    if (!openPinnedRoot(request.absoluteRootPath, request.rootExpectation,
                        &rootFd, &rootDevice, &rootInode, &validationDetail))
    {
        return stageFailure(PrivacyPublicTransitionError::RootIdentityMismatch,
                            validationDetail);
    }

    PinnedParent parent;

    if (!openPinnedParent(rootFd.get(), rootDevice,
                          parentPath(asset->publicRelativePath),
                          &parent, &validationDetail))
    {
        return stageFailure(PrivacyPublicTransitionError::UnsafePath,
                            validationDetail);
    }

    const QByteArray stageName = baseName(asset->stagedRelativePath).toUtf8();
    ScopedFd staged(PrivacyPosixStorage::confinedOpenAt(
        parent.descriptor.get(), stageName,
        O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK,
        0600));

    if (staged.get() < 0)
    {
        const int openError = errno;
        const FileOpenStatus status = classifyFailedOpen(
            parent.descriptor.get(), stageName, rootDevice, true);
        const PrivacyPublicTransitionError error =
            (status == FileOpenStatus::Unsafe)
                ? PrivacyPublicTransitionError::UnsafePath
                : (openError == EEXIST)
                    ? PrivacyPublicTransitionError::UnexpectedExistingFile
                    : PrivacyPublicTransitionError::IoFailure;
        return stageFailure(error,
                            QStringLiteral("cannot exclusively create replacement stage"));
    }

    struct stat createdFacts = {};

    if ((::fchmod(staged.get(), 0600) != 0) ||
        (::fstat(staged.get(), &createdFacts) != 0) ||
        !S_ISREG(createdFacts.st_mode) ||
        (static_cast<quint64>(createdFacts.st_dev) != rootDevice) ||
        (createdFacts.st_uid != ::geteuid()) || (createdFacts.st_nlink != 1) ||
        ((createdFacts.st_mode & 0777) != 0600))
    {
        ::unlinkat(parent.descriptor.get(), stageName.constData(), 0);
        ::fsync(parent.descriptor.get());
        return stageFailure(PrivacyPublicTransitionError::UnsafePath,
                            QStringLiteral("created replacement stage is unsafe"));
    }

    const auto cleanup = [&]()
    {
        struct stat bound = {};

        if ((::fstatat(parent.descriptor.get(), stageName.constData(), &bound,
                       AT_SYMLINK_NOFOLLOW) == 0) &&
            (bound.st_dev == createdFacts.st_dev) &&
            (bound.st_ino == createdFacts.st_ino))
        {
            ::unlinkat(parent.descriptor.get(), stageName.constData(), 0);
            ::fsync(parent.descriptor.get());
        }
    };

    QString producerDetail;

    if (!producer(staged.get(), &producerDetail))
    {
        cleanup();
        return stageFailure(
            PrivacyPublicTransitionError::IoFailure,
            producerDetail.isEmpty()
                ? QStringLiteral("replacement producer failed")
                : producerDetail);
    }

    if (::fsync(staged.get()) != 0)
    {
        cleanup();
        return stageFailure(PrivacyPublicTransitionError::DurabilityUncertain,
                            QStringLiteral("cannot fsync replacement stage"));
    }

    VerifiedFile verified;

    if (!hashStableOpenedFile(staged.get(), rootDevice, &verified) ||
        !rootStillPinned(rootFd.get(), rootDevice, rootInode) ||
        !rootPathStillMatches(request.absoluteRootPath, rootDevice, rootInode) ||
        !parentStillReachable(rootFd.get(), rootDevice, parent) ||
        !nameStillBindsFile(parent.descriptor.get(), stageName, verified))
    {
        cleanup();
        return stageFailure(PrivacyPublicTransitionError::FileFactMismatch,
                            QStringLiteral("replacement stage changed during verification"));
    }

    if (::fsync(parent.descriptor.get()) != 0)
    {
        cleanup();
        return stageFailure(PrivacyPublicTransitionError::DurabilityUncertain,
                            QStringLiteral("cannot fsync replacement parent"));
    }

    PrivacyPublicReplacementStageResult result;
    result.stagedRelativePath = asset->stagedRelativePath;
    result.fact.presence      = PrivacyJournalExpectedPresence::Present;
    result.fact.size          = verified.size;
    result.fact.linkCount     = verified.linkCount;
    result.fact.sha256        = verified.sha256;
    return result;
#endif
}

PrivacyPublicTransitionResult PrivacyPublicTransitionEngine::execute(
    const PrivacyPublicTransitionRequest& request) const
{
    return executeOne(request, true);
}

PrivacyPublicTransitionResult PrivacyPublicTransitionEngine::executeOne(
    const PrivacyPublicTransitionRequest& request,
    bool advanceToPublicState) const
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
    ValidatedTransitionRequest validated;
    const PrivacyPublicTransitionError validationError =
        validateTransitionRequest(request, &validated, &validationDetail);

    if (validationError != PrivacyPublicTransitionError::None)
    {
        return failure(validationError, validationDetail);
    }

    const PrivacyJournalAsset* const asset = validated.asset;
    const PrivacyJournalObjectFact* const installedFact =
        validated.installedFact;
    const PrivacyJournalObjectFact* const currentFact = validated.currentFact;
    const bool install = validated.install;
    const bool exchange = validated.exchange;
    const bool remove = validated.remove;

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
        (loaded.record.stage != request.journalRecord.stage))
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
    struct DiskInspection
    {
        TransitionDiskState state = TransitionDiskState::Unknown;
        VerifiedFile        preStage;
        VerifiedFile        prePublic;
        VerifiedFile        postStage;
        VerifiedFile        postPublic;
        FileOpenStatus      preStageStatus  = FileOpenStatus::Missing;
        FileOpenStatus      prePublicStatus = FileOpenStatus::Missing;
        FileOpenStatus      postStageStatus = FileOpenStatus::Missing;
        FileOpenStatus      postPublicStatus = FileOpenStatus::Missing;
    };

    const auto inspectDisk = [&]()
    {
        DiskInspection inspection;

        bool exactPre = false;
        bool exactPost = false;

        if (exchange)
        {
            inspection.preStageStatus = openAndVerifyFile(
                parent.descriptor.get(), stageName, rootDevice, *installedFact,
                true, true, &inspection.preStage);
            inspection.prePublicStatus = openAndVerifyFile(
                parent.descriptor.get(), publicName, rootDevice, *currentFact,
                false, false, &inspection.prePublic);
            inspection.postPublicStatus = openAndVerifyFile(
                parent.descriptor.get(), publicName, rootDevice, *installedFact,
                false, false, &inspection.postPublic);
            inspection.postStageStatus = openAndVerifyFile(
                parent.descriptor.get(), stageName, rootDevice, *currentFact,
                false, false, &inspection.postStage);
            exactPre = ((inspection.preStageStatus == FileOpenStatus::Verified) &&
                        (inspection.prePublicStatus == FileOpenStatus::Verified));
            exactPost = ((inspection.postPublicStatus == FileOpenStatus::Verified) &&
                         (inspection.postStageStatus == FileOpenStatus::Verified));
        }
        else if (install)
        {
            inspection.preStageStatus = openAndVerifyFile(
                parent.descriptor.get(), stageName, rootDevice, *installedFact,
                true, true, &inspection.preStage);
            exactPre = ((inspection.preStageStatus == FileOpenStatus::Verified) &&
                        nameIsAbsent(parent.descriptor.get(), publicName));
            inspection.postPublicStatus = openAndVerifyFile(
                parent.descriptor.get(), publicName, rootDevice, *installedFact,
                false, false, &inspection.postPublic);
            exactPost = ((inspection.postPublicStatus == FileOpenStatus::Verified) &&
                         nameIsAbsent(parent.descriptor.get(), stageName));
        }
        else
        {
            inspection.prePublicStatus = openAndVerifyFile(
                parent.descriptor.get(), publicName, rootDevice, *currentFact,
                true, false, &inspection.prePublic);
            inspection.postStageStatus = openAndVerifyFile(
                parent.descriptor.get(), stageName, rootDevice, *currentFact,
                false, false, &inspection.postStage);
            exactPre = ((inspection.prePublicStatus == FileOpenStatus::Verified) &&
                        nameIsAbsent(parent.descriptor.get(), stageName));
            exactPost = ((inspection.postStageStatus == FileOpenStatus::Verified) &&
                         nameIsAbsent(parent.descriptor.get(), publicName));
        }

        inspection.state = (exactPre && exactPost)
                         ? TransitionDiskState::Ambiguous
                         : exactPre
                             ? TransitionDiskState::ExactPre
                             : exactPost
                                 ? TransitionDiskState::ExactPost
                                 : TransitionDiskState::Unknown;
        return inspection;
    };

    const auto inspectionFailure = [](const DiskInspection& inspection)
    {
        const bool stageVerified =
            ((inspection.preStageStatus == FileOpenStatus::Verified) ||
             (inspection.postStageStatus == FileOpenStatus::Verified));
        const bool publicVerified =
            ((inspection.prePublicStatus == FileOpenStatus::Verified) ||
             (inspection.postPublicStatus == FileOpenStatus::Verified));

        if (!stageVerified &&
            ((inspection.preStageStatus == FileOpenStatus::Unsafe) ||
             (inspection.preStageStatus == FileOpenStatus::Hardlinked) ||
             (inspection.postStageStatus == FileOpenStatus::Unsafe) ||
             (inspection.postStageStatus == FileOpenStatus::Hardlinked)))
        {
            return PrivacyPublicTransitionError::UnsafePath;
        }

        if (!publicVerified &&
            ((inspection.prePublicStatus == FileOpenStatus::Hardlinked) ||
             (inspection.postPublicStatus == FileOpenStatus::Hardlinked)))
        {
            return PrivacyPublicTransitionError::HardlinkReconciliationRequired;
        }

        if (!publicVerified &&
            ((inspection.prePublicStatus == FileOpenStatus::Unsafe) ||
             (inspection.postPublicStatus == FileOpenStatus::Unsafe)))
        {
            return PrivacyPublicTransitionError::UnsafePath;
        }

        if ((inspection.preStageStatus == FileOpenStatus::IoFailure) ||
            (inspection.prePublicStatus == FileOpenStatus::IoFailure) ||
            (inspection.postStageStatus == FileOpenStatus::IoFailure) ||
            (inspection.postPublicStatus == FileOpenStatus::IoFailure))
        {
            return PrivacyPublicTransitionError::IoFailure;
        }

        return PrivacyPublicTransitionError::FileFactMismatch;
    };

    DiskInspection inspection = inspectDisk();
    const PrivacyJournalStage initialStage = request.journalRecord.stage;
    PrivacyJournalRecord applyingRecord = request.journalRecord;
    QByteArray applyingHash = request.authoritativeJournalSha256;
    PrivacyPublicTransitionResult result;

    if (initialStage == PrivacyJournalStage::PublicStateVerified)
    {
        if ((inspection.state != TransitionDiskState::ExactPost) &&
            (inspection.state != TransitionDiskState::Ambiguous))
        {
            return failure(inspectionFailure(inspection),
                           QStringLiteral("PublicStateVerified journal does not match exact installed state"));
        }

        struct stat installedMetadata = {};

        if ((request.installedUnixMode >= 0) &&
            ((::fstat(inspection.postPublic.descriptor.get(),
                      &installedMetadata) != 0) ||
             ((installedMetadata.st_mode & 07777) !=
              static_cast<mode_t>(request.installedUnixMode))))
        {
            return failure(PrivacyPublicTransitionError::FileFactMismatch,
                           QStringLiteral("PublicStateVerified public mode is not exact"));
        }

        if (!rootStillPinned(rootFd.get(), rootDevice, rootInode) ||
            !rootPathStillMatches(request.absoluteRootPath, rootDevice, rootInode) ||
            !parentStillReachable(rootFd.get(), rootDevice, parent) ||
            ((install || exchange) &&
             !nameStillBindsFile(parent.descriptor.get(), publicName,
                                 inspection.postPublic)) ||
            (remove && !nameIsAbsent(parent.descriptor.get(), publicName)) ||
            ((exchange || remove) &&
             !nameStillBindsFile(parent.descriptor.get(), stageName,
                                 inspection.postStage)) ||
            (install && !nameIsAbsent(parent.descriptor.get(), stageName)))
        {
            return failure(PrivacyPublicTransitionError::RootIdentityMismatch,
                           QStringLiteral("PublicStateVerified path binding changed during readback"));
        }

        result.finalJournalSha256 = request.authoritativeJournalSha256;
        result.installedVerified  = true;
        result.displacedVerified  = exchange || remove;
        result.displacedRelativePath = (exchange || remove)
                                     ? asset->stagedRelativePath
                                     : QString();
        result.displacedRelativePaths = result.displacedRelativePath.isEmpty()
                                      ? QStringList()
                                      : QStringList { result.displacedRelativePath };
        return result;
    }

    if (initialStage == PrivacyJournalStage::ProtectedCopyVerified)
    {
        if ((inspection.state != TransitionDiskState::ExactPre) &&
            (inspection.state != TransitionDiskState::Ambiguous))
        {
            return failure(inspectionFailure(inspection),
                           QStringLiteral("ProtectedCopyVerified journal does not match exact pre-mutation state"));
        }

        if (faulted(PrivacyPublicTransitionFaultPoint::AfterInitialVerification))
        {
            return failure(PrivacyPublicTransitionError::FaultInjected,
                           QStringLiteral("fault injected after initial file verification"));
        }

        const int durableSourceFd = remove
                                  ? inspection.prePublic.descriptor.get()
                                  : inspection.preStage.descriptor.get();

        if (::fsync(durableSourceFd) != 0)
        {
            return failure(PrivacyPublicTransitionError::IoFailure,
                           QStringLiteral("cannot fsync staged replacement"));
        }

        if (faulted(PrivacyPublicTransitionFaultPoint::AfterStagedFsync))
        {
            return failure(PrivacyPublicTransitionError::FaultInjected,
                           QStringLiteral("fault injected after staged fsync"));
        }

        applyingRecord.stage = PrivacyJournalStage::Applying;

        if (!journalStore->compareAndUpdate(
                applyingRecord, request.authoritativeJournalSha256,
                &applyingHash, &journalError, &journalDetail))
        {
            return failure(PrivacyPublicTransitionError::JournalAdvanceFailed,
                           journalDetail);
        }

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

        inspection = inspectDisk();

        if ((inspection.state != TransitionDiskState::ExactPre) &&
            (inspection.state != TransitionDiskState::Ambiguous))
        {
            result.error  = inspectionFailure(inspection);
            result.detail = QStringLiteral("file state changed after Applying journal advancement");
            return result;
        }
    }
    else
    {
        result.applyingJournalSha256 = applyingHash;

        if (inspection.state == TransitionDiskState::Unknown)
        {
            result.error  = inspectionFailure(inspection);
            result.detail = QStringLiteral("Applying replay found mixed or unknown public bytes");
            return result;
        }
    }

    bool mutateNamespace = (inspection.state == TransitionDiskState::ExactPre) ||
                           ((initialStage == PrivacyJournalStage::ProtectedCopyVerified) &&
                            (inspection.state == TransitionDiskState::Ambiguous));

    if (mutateNamespace)
    {
        const int durableSourceFd = remove
                                  ? inspection.prePublic.descriptor.get()
                                  : inspection.preStage.descriptor.get();

        if (::fsync(durableSourceFd) != 0)
        {
            result.error  = PrivacyPublicTransitionError::IoFailure;
            result.detail = QStringLiteral("immediate pre-mutation stage fsync failed");
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
            ((install || exchange) &&
             !nameStillBindsFile(parent.descriptor.get(), stageName,
                                 inspection.preStage)) ||
            (remove && !nameIsAbsent(parent.descriptor.get(), stageName)) ||
            ((exchange || remove) &&
             !nameStillBindsFile(parent.descriptor.get(), publicName,
                                 inspection.prePublic)) ||
            (install && !nameIsAbsent(parent.descriptor.get(), publicName)))
        {
            result.error  = PrivacyPublicTransitionError::RootIdentityMismatch;
            result.detail = QStringLiteral("parent or file binding changed immediately before mutation");
            return result;
        }

        bool atomicUnavailable = false;

        const QByteArray& fromName = remove ? publicName : stageName;
        const QByteArray& toName   = remove ? stageName : publicName;

        if (!PrivacyPosixStorage::atomicRenameAt(
                parent.descriptor.get(), fromName, toName,
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

        if (faulted(PrivacyPublicTransitionFaultPoint::AfterNamespaceMutation))
        {
            result.error  = PrivacyPublicTransitionError::DurabilityUncertain;
            result.detail = QStringLiteral("fault injected after namespace mutation before parent fsync");
            return result;
        }
    }

    if (exchange || remove)
    {
        result.displacedRelativePath = asset->stagedRelativePath;
        result.displacedRelativePaths << asset->stagedRelativePath;
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
        if (!result.namespaceMutated)
        {
            result.error  = PrivacyPublicTransitionError::ReconciliationRequired;
            result.detail = reason + QStringLiteral("; replay will not mutate unknown bytes");
            return result;
        }

        if (faulted(PrivacyPublicTransitionFaultPoint::BeforeRollback))
        {
            result.error  = PrivacyPublicTransitionError::RollbackUncertain;
            result.detail = reason + QStringLiteral("; rollback fault injected before rename");
            return result;
        }

        if (request.installedUnixMode >= 0)
        {
            VerifiedFile installedBeforeRollback;
            struct stat installedMetadata = {};

            if ((openAndVerifyFile(parent.descriptor.get(), publicName,
                                   rootDevice, *installedFact, true, false,
                                   &installedBeforeRollback) !=
                 FileOpenStatus::Verified) ||
                (::fchmod(installedBeforeRollback.descriptor.get(), 0600) != 0) ||
                (::fsync(installedBeforeRollback.descriptor.get()) != 0) ||
                (::fstat(installedBeforeRollback.descriptor.get(),
                         &installedMetadata) != 0) ||
                ((installedMetadata.st_mode & 0777) != 0600) ||
                !nameStillBindsFile(parent.descriptor.get(), publicName,
                                    installedBeforeRollback))
            {
                result.error  = PrivacyPublicTransitionError::RollbackUncertain;
                result.detail = reason + QStringLiteral(
                    "; installed stage permissions cannot be reset before rollback");
                return result;
            }
        }

        bool unavailable = false;
        const QByteArray& rollbackFrom = remove ? stageName : publicName;
        const QByteArray& rollbackTo   = remove ? publicName : stageName;
        const bool renamed = PrivacyPosixStorage::atomicRenameAt(
            parent.descriptor.get(), rollbackFrom, rollbackTo,
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
        const bool restorationVerified = exchange || remove
            ? ((openAndVerifyFile(parent.descriptor.get(), publicName,
                                  rootDevice, *currentFact, false, false,
                                  &restored) == FileOpenStatus::Verified) &&
               (!remove || nameIsAbsent(parent.descriptor.get(), stageName)))
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

    if ((remove && !nameIsAbsent(parent.descriptor.get(), publicName)) ||
        (!remove &&
         (openAndVerifyFile(parent.descriptor.get(), publicName, rootDevice,
                            *installedFact, false, false,
                            &installed) != FileOpenStatus::Verified)))
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

    if (request.installedUnixMode >= 0)
    {
        struct stat installedMetadata = {};

        if ((::fchmod(installed.descriptor.get(),
                      static_cast<mode_t>(request.installedUnixMode)) != 0) ||
            (::fsync(installed.descriptor.get()) != 0) ||
            (::fstat(installed.descriptor.get(), &installedMetadata) != 0) ||
            ((installedMetadata.st_mode & 07777) !=
             static_cast<mode_t>(request.installedUnixMode)) ||
            !nameStillBindsFile(parent.descriptor.get(), publicName, installed))
        {
            return rollback(QStringLiteral("installed public mode cannot be applied exactly"));
        }
    }

    if (exchange || remove)
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

    if (!advanceToPublicState)
    {
        result.finalJournalSha256 = applyingHash;
        result.error = PrivacyPublicTransitionError::None;
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

PrivacyPublicTransitionResult PrivacyPublicTransitionEngine::executeBatch(
    const QList<PrivacyPublicTransitionRequest>& requests) const
{
#if !defined(Q_OS_LINUX)
    Q_UNUSED(requests);
    return failure(PrivacyPublicTransitionError::AtomicPublicationUnavailable,
                   QStringLiteral("public transition requires Linux renameat2"));
#else
    if (requests.isEmpty() ||
        (requests.size() > PrivacyTransactionJournalCodec::MaximumAssetCount))
    {
        return failure(PrivacyPublicTransitionError::InvalidRequest,
                       QStringLiteral("batch transition request count is invalid"));
    }

    if (requests.size() == 1)
    {
        return execute(requests.constFirst());
    }

    const PrivacyPublicTransitionRequest& first = requests.constFirst();
    const QByteArray canonicalJournal = PrivacyTransactionJournalCodec::encode(
        first.journalRecord);
    const PrivacyJournalStage initialStage = first.journalRecord.stage;

    if (canonicalJournal.isEmpty() ||
        (requests.size() != first.journalRecord.assets.size()) ||
        ((initialStage != PrivacyJournalStage::ProtectedCopyVerified) &&
         (initialStage != PrivacyJournalStage::Applying) &&
         (initialStage != PrivacyJournalStage::PublicStateVerified)))
    {
        return failure(PrivacyPublicTransitionError::InvalidRequest,
                       QStringLiteral("batch must cover every exact journal asset"));
    }

    const auto sameExpectation = [](const PrivacyJournalRootExpectation& left,
                                    const PrivacyJournalRootExpectation& right)
    {
        return ((left.rootUuid == right.rootUuid) &&
                (left.markerUuid == right.markerUuid) &&
                (left.identitySha256 == right.identitySha256) &&
                (left.device == right.device) &&
                (left.inode == right.inode));
    };
    QSet<QString> requestedIdentities;

    for (const PrivacyPublicTransitionRequest& request : requests)
    {
        ValidatedTransitionRequest validated;
        QString memberDetail;
        const PrivacyPublicTransitionError memberError =
            validateTransitionRequest(request, &validated, &memberDetail);

        if (memberError != PrivacyPublicTransitionError::None)
        {
            return failure(memberError, memberDetail);
        }

        const QString identity = QStringLiteral("%1:%2:%3")
                                     .arg(request.itemUuid)
                                     .arg(request.role)
                                     .arg(request.ordinal);

        if ((request.absoluteRootPath != first.absoluteRootPath) ||
            !sameExpectation(request.rootExpectation,
                             first.rootExpectation) ||
            (request.authoritativeJournalSha256 !=
             first.authoritativeJournalSha256) ||
            (PrivacyTransactionJournalCodec::encode(request.journalRecord) !=
             canonicalJournal) ||
            requestedIdentities.contains(identity))
        {
            return failure(PrivacyPublicTransitionError::InvalidRequest,
                           QStringLiteral("batch transition facts are not one exact journal"));
        }

        requestedIdentities.insert(identity);
    }

    for (const PrivacyJournalAsset& asset : first.journalRecord.assets)
    {
        const QString identity = QStringLiteral("%1:%2:%3")
                                     .arg(asset.itemUuid)
                                     .arg(asset.role)
                                     .arg(asset.ordinal);

        if (!requestedIdentities.contains(identity))
        {
            return failure(PrivacyPublicTransitionError::InvalidRequest,
                           QStringLiteral("batch omits a journal asset"));
        }
    }

    PrivacyJournalError journalError = PrivacyJournalError::None;
    QString journalDetail;
    std::unique_ptr<PrivacyTransactionJournalStore> journalStore =
        PrivacyTransactionJournalStore::open(
            first.absoluteRootPath, first.rootExpectation,
            &journalError, &journalDetail);

    if (!journalStore)
    {
        return failure(PrivacyPublicTransitionError::RootIdentityMismatch,
                       journalDetail);
    }

    const PrivacyJournalLoadResult loaded = journalStore->load(
        first.journalRecord.transactionUuid);

    if ((loaded.disposition != PrivacyJournalLoadDisposition::Loaded) ||
        !loaded.authoritative || !loaded.hasRecord ||
        (loaded.sha256 != first.authoritativeJournalSha256) ||
        (loaded.canonicalBytes != canonicalJournal) ||
        (loaded.record.stage != initialStage))
    {
        return failure(PrivacyPublicTransitionError::JournalRejected,
                       QStringLiteral("batch journal is missing, stale, or uncertain"));
    }

    PrivacyJournalRecord applyingRecord = first.journalRecord;
    QByteArray applyingHash = first.authoritativeJournalSha256;
    PrivacyPublicTransitionResult result;

    if (initialStage == PrivacyJournalStage::ProtectedCopyVerified)
    {
        applyingRecord.stage = PrivacyJournalStage::Applying;

        if (!journalStore->compareAndUpdate(
                applyingRecord, first.authoritativeJournalSha256,
                &applyingHash, &journalError, &journalDetail))
        {
            return failure(PrivacyPublicTransitionError::JournalAdvanceFailed,
                           journalDetail);
        }

        result.applyingJournalSha256 = applyingHash;

        if (m_faultHook &&
            m_faultHook(PrivacyPublicTransitionFaultPoint::AfterApplyingJournal))
        {
            result.error = PrivacyPublicTransitionError::FaultInjected;
            result.detail = QStringLiteral(
                "fault injected after batch Applying journal stage");
            return result;
        }
    }
    else
    {
        result.applyingJournalSha256 = applyingHash;
    }

    for (const PrivacyPublicTransitionRequest& request : requests)
    {
        PrivacyPublicTransitionRequest applyingRequest = request;
        applyingRequest.journalRecord = applyingRecord;
        applyingRequest.authoritativeJournalSha256 = applyingHash;
        PrivacyPublicTransitionResult member = executeOne(applyingRequest,
                                                          false);
        result.namespaceMutated = result.namespaceMutated ||
                                  member.namespaceMutated;
        result.installedVerified = result.installedVerified ||
                                   member.installedVerified;
        result.displacedVerified = result.displacedVerified ||
                                   member.displacedVerified;
        result.displacedRelativePaths << member.displacedRelativePaths;

        if (result.displacedRelativePath.isEmpty() &&
            !member.displacedRelativePath.isEmpty())
        {
            result.displacedRelativePath = member.displacedRelativePath;
        }

        if (!member.succeeded())
        {
            result.error = member.error;
            result.detail = member.detail + QStringLiteral(
                "; batch journal remains Applying for exact replay");
            return result;
        }
    }

    if (initialStage == PrivacyJournalStage::PublicStateVerified)
    {
        result.finalJournalSha256 = first.authoritativeJournalSha256;
        result.error = PrivacyPublicTransitionError::None;
        return result;
    }

    PrivacyJournalRecord finalRecord = applyingRecord;
    finalRecord.stage = PrivacyJournalStage::PublicStateVerified;

    if (!journalStore->compareAndUpdate(finalRecord, applyingHash,
                                        &result.finalJournalSha256,
                                        &journalError, &journalDetail))
    {
        result.error = PrivacyPublicTransitionError::ReconciliationRequired;
        result.detail = QStringLiteral(
            "batch namespace is verified but final journal advancement failed: %1")
                            .arg(journalDetail);
        return result;
    }

    if (m_faultHook &&
        m_faultHook(PrivacyPublicTransitionFaultPoint::AfterPublicStateJournal))
    {
        result.error = PrivacyPublicTransitionError::FaultInjected;
        result.detail = QStringLiteral(
            "fault injected after batch PublicStateVerified journal");
        return result;
    }

    result.error = PrivacyPublicTransitionError::None;
    return result;
#endif
}

} // namespace Digikam
