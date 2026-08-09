/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Description : privacy-aware item access broker.
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyitemaccessbroker_p.h"

// C++ includes

#include <algorithm>

// Qt includes

#include <QAbstractButton>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSet>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QThread>
#include <QtEndian>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikamapp.h"
#include "iteminfo.h"
#include "privacyactionpolicy.h"
#include "privacycachetransition.h"
#include "privacycategorysessionowner.h"
#include "privacyleaseregistry.h"
#include "privacypreparedaccessregistry.h"
#include "privacyrepository.h"
#include "privacyruntime.h"
#include "privacysourceresolver.h"

namespace Digikam
{

namespace
{

DItemAccessFileFacts fileFactsForPath(const QString& path)
{
    const QFileInfo info(path);
    DItemAccessFileFacts facts;

    if (!info.exists() || !info.isFile() || !info.lastModified().isValid())
    {
        return facts;
    }

    facts.modificationDate = info.lastModified().toUTC();
    facts.permissions      = info.permissions();
    facts.available        = true;
    return facts.isValid() ? facts : DItemAccessFileFacts();
}

DItemAccessFileFacts fileFactsForAsset(const PrivacyAsset& asset)
{
    DItemAccessFileFacts facts;

    if (!asset.originalModificationDate.isValid() ||
        (asset.portableAttributes.size() != 4))
    {
        return facts;
    }

    const quint32 mode = qFromBigEndian<quint32>(
        asset.portableAttributes.constData());

    if ((mode & ~quint32(07777)) != 0)
    {
        return facts;
    }

    if (mode & 0400) facts.permissions |= QFileDevice::ReadOwner;
    if (mode & 0200) facts.permissions |= QFileDevice::WriteOwner;
    if (mode & 0100) facts.permissions |= QFileDevice::ExeOwner;
    if (mode & 0040) facts.permissions |= QFileDevice::ReadGroup;
    if (mode & 0020) facts.permissions |= QFileDevice::WriteGroup;
    if (mode & 0010) facts.permissions |= QFileDevice::ExeGroup;
    if (mode & 0004) facts.permissions |= QFileDevice::ReadOther;
    if (mode & 0002) facts.permissions |= QFileDevice::WriteOther;
    if (mode & 0001) facts.permissions |= QFileDevice::ExeOther;

    facts.modificationDate = asset.originalModificationDate.toUTC();
    facts.available        = true;
    return facts.isValid() ? facts : DItemAccessFileFacts();
}

enum class AccessResolution
{
    Unlock,
    Proxy,
    Exclude,
    Cancel
};

void wipe(QString& value)
{
    value.fill(QChar::Null);
    value.clear();
}

PrivacyActionKind actionKindFor(DItemAccessPurpose purpose)
{
    switch (purpose)
    {
        case DItemAccessPurpose::Export:
            return PrivacyActionKind::Export;

        case DItemAccessPurpose::Print:
            return PrivacyActionKind::Print;

        case DItemAccessPurpose::View:
            return PrivacyActionKind::Preview;

        case DItemAccessPurpose::BatchProcess:
            return PrivacyActionKind::BatchProcess;

        case DItemAccessPurpose::ExternalOpen:
            return PrivacyActionKind::ExternalOpen;

        case DItemAccessPurpose::DragOrClipboard:
            return PrivacyActionKind::DragOrClipboard;

        case DItemAccessPurpose::MetadataWrite:
            return PrivacyActionKind::MetadataWrite;
    }

    return static_cast<PrivacyActionKind>(0);
}

PrivacyRequestedSource requestedSourceFor(DItemAccessSource source)
{
    switch (source)
    {
        case DItemAccessSource::PublicRepresentation:
            return PrivacyRequestedSource::PublicProxy;

        case DItemAccessSource::InternalOriginal:
            return PrivacyRequestedSource::InternalOriginal;

        case DItemAccessSource::WritableCheckout:
            return PrivacyRequestedSource::WritableCheckout;

        case DItemAccessSource::PublicOriginal:
            return PrivacyRequestedSource::PublicOriginal;
    }

    return static_cast<PrivacyRequestedSource>(0);
}

PrivacyMutationPolicy mutationPolicyFor(DItemAccessMutation mutation)
{
    switch (mutation)
    {
        case DItemAccessMutation::ReadOnly:
            return PrivacyMutationPolicy::ReadOnly;

        case DItemAccessMutation::MayCreateOutputs:
            return PrivacyMutationPolicy::MayCreateOutputs;

        case DItemAccessMutation::MayModifyInputs:
            return PrivacyMutationPolicy::CommitProtectedAsset;
    }

    return static_cast<PrivacyMutationPolicy>(0);
}

bool isReadyDisposition(PrivacyActionPolicyDisposition disposition)
{
    return ((disposition ==
             PrivacyActionPolicyDisposition::UnprotectedPassThrough) ||
            (disposition ==
             PrivacyActionPolicyDisposition::ReadyWithProxy) ||
            (disposition ==
             PrivacyActionPolicyDisposition::ReadyWithInternalOriginal) ||
            (disposition ==
             PrivacyActionPolicyDisposition::ReadyWithWritableCheckout));
}

QWidget* accessDialogParent()
{
    if (QApplication::activeModalWidget())
    {
        return QApplication::activeModalWidget();
    }

    if (QApplication::activeWindow())
    {
        return QApplication::activeWindow();
    }

    return DigikamApp::instance();
}

AccessResolution promptAccessResolution(
    const DItemAccessRequest& request,
    const PrivacyActionPolicyResult& policy)
{
    if (!qApp || (QThread::currentThread() != qApp->thread()))
    {
        return AccessResolution::Cancel;
    }

    const int blockedItemCount = std::count_if(
        policy.items.cbegin(), policy.items.cend(),
        [](const PrivacyActionPolicyItem& item)
        {
            return !isReadyDisposition(item.disposition);
        });

    QMessageBox box(accessDialogParent());
    box.setIcon(QMessageBox::Question);
    box.setWindowTitle(i18nc("@title:window", "Private Items"));
    box.setText(i18np(
        "This operation includes one protected item whose requested source "
        "is not currently available.",
        "This operation includes %1 protected items whose requested sources "
        "are not currently available.", blockedItemCount));
    box.setInformativeText(
        i18np("Choose how digiKam should prepare the affected category. Items "
              "that cannot be prepared will not be handed to the tool.",
              "Choose how digiKam should prepare the %1 affected categories. "
              "Items that cannot be prepared will not be handed to the tool.",
              policy.affectedCategoryUuids.size()));

    QAbstractButton* unlockButton  = nullptr;
    QAbstractButton* proxyButton   = nullptr;
    QAbstractButton* excludeButton = nullptr;

    if (policy.canUnlockCategories)
    {
        unlockButton = box.addButton(
            i18nc("@action:button", "Unlock and Use Originals"),
            QMessageBox::AcceptRole);
    }

    if (request.allowPlaceholderFallback && policy.canContinueWithProxy)
    {
        proxyButton = box.addButton(
            i18nc("@action:button", "Use Placeholders"),
            QMessageBox::ActionRole);
    }

    if (request.allowPartialSelection && policy.canExcludeAffected)
    {
        excludeButton = box.addButton(
            i18nc("@action:button", "Exclude Unavailable Items"),
            QMessageBox::DestructiveRole);
    }

    box.addButton(QMessageBox::Cancel);
    box.setDefaultButton(qobject_cast<QPushButton*>(unlockButton
                                                   ? unlockButton
                                                   : proxyButton));
    box.exec();

    if (box.clickedButton() == unlockButton)
    {
        return AccessResolution::Unlock;
    }

    if (box.clickedButton() == proxyButton)
    {
        return AccessResolution::Proxy;
    }

    if (box.clickedButton() == excludeButton)
    {
        return AccessResolution::Exclude;
    }

    return AccessResolution::Cancel;
}

void relockCategories(
    const QSharedPointer<PrivacyCategorySessionOwner>& sessions,
    const QStringList& categoryUuids)
{
    if (!sessions)
    {
        return;
    }

    for (auto it = categoryUuids.crbegin() ; it != categoryUuids.crend() ; ++it)
    {
        (void)sessions->lockCategory(*it);
    }
}

bool unlockCategories(const QStringList& categoryUuids)
{
    const QSharedPointer<PrivacyCategorySessionOwner> sessions =
        PrivacyStartupRecovery::categorySessions();

    if (!sessions)
    {
        return false;
    }

    QStringList newlyUnlocked;

    for (const QString& categoryUuid : categoryUuids)
    {
        if (sessions->ownsSecret(categoryUuid))
        {
            continue;
        }

        const PrivacyCategory category = PrivacyRepository().category(categoryUuid);
        const QString categoryName = category.isValid() ? category.name : categoryUuid;

        for (;;)
        {
            bool accepted = false;
            QString password = QInputDialog::getText(
                accessDialogParent(),
                i18nc("@title:window", "Unlock Private Items"),
                i18nc("@label", "Password for %1:", categoryName),
                QLineEdit::Password, QString(), &accepted);

            if (!accepted)
            {
                wipe(password);
                relockCategories(sessions, newlyUnlocked);
                return false;
            }

            PrivacyCategorySessionResult result;

            try
            {
                result = sessions->unlockCategory(categoryUuid, password);
            }
            catch (...)
            {
                wipe(password);
                QMessageBox::warning(
                    accessDialogParent(),
                    i18nc("@title:window", "Unlock Failed"),
                    i18n("The private category could not be unlocked safely. "
                         "digiKam will use only the available fallback."));
                relockCategories(sessions, newlyUnlocked);
                return false;
            }

            wipe(password);

            if (result.succeeded())
            {
                newlyUnlocked << categoryUuid;
                break;
            }

            if (result.status == PrivacyCategorySessionStatus::InvalidPassword)
            {
                QMessageBox::warning(
                    accessDialogParent(),
                    i18nc("@title:window", "Unlock Failed"),
                    i18n("That password was not accepted. Try again or cancel "
                         "to use the available fallback."));
                continue;
            }

            QMessageBox::warning(
                accessDialogParent(),
                i18nc("@title:window", "Unlock Failed"),
                i18n("The private category could not be unlocked safely. "
                     "digiKam will use only the available fallback."));
            relockCategories(sessions, newlyUnlocked);
            return false;
        }
    }

    return true;
}

bool pathIsInside(const QString& path, const QString& root)
{
    const QString cleanPath = QDir::cleanPath(path);
    const QString cleanRoot = QDir::cleanPath(root);

    return (!cleanRoot.isEmpty() &&
            ((cleanPath == cleanRoot) ||
             cleanPath.startsWith(cleanRoot + QDir::separator())));
}

bool matchesRootRelativePath(
    const QString& path,
    const QString& rootUuid,
    const QString& relativePath,
    const PrivacyRepositorySnapshot& snapshot)
{
    if (rootUuid.isEmpty() || relativePath.isEmpty() ||
        QDir::isAbsolutePath(relativePath) ||
        relativePath.contains(QChar::Null))
    {
        return false;
    }

    for (const PrivacyStorageRoot& root : snapshot.storageRoots)
    {
        if ((root.uuid != rootUuid) || root.configuredPath.isEmpty())
        {
            continue;
        }

        const QString candidate = QDir::cleanPath(
            QDir(root.configuredPath).absoluteFilePath(relativePath));

        if (pathIsInside(candidate, root.configuredPath) &&
            (QDir::cleanPath(path) == candidate))
        {
            return true;
        }
    }

    return false;
}

bool isReservedPrivacyPath(const QString& path,
                           const PrivacyRepositorySnapshot& snapshot)
{
    const QString cleanPath = QDir::cleanPath(path);

    if (cleanPath.endsWith(QLatin1String(".digikam-private.zip"),
                           Qt::CaseInsensitive))
    {
        return true;
    }

    for (const PrivacyStorageRoot& root : snapshot.storageRoots)
    {
        if ((root.kind == PrivacyStorageRootKind::ManagedStoreRoot) &&
            pathIsInside(cleanPath, root.configuredPath))
        {
            return true;
        }
    }

    for (const PrivacyAsset& asset : snapshot.assets)
    {
        if (matchesRootRelativePath(cleanPath, asset.publicRootUuid,
                                    asset.publicRelativePath, snapshot))
        {
            return true;
        }
    }

    for (const PrivacyContainer& container : snapshot.containers)
    {
        if (matchesRootRelativePath(cleanPath, container.rootUuid,
                                    container.objectRelativePath, snapshot))
        {
            return true;
        }
    }

    for (const PrivacyStore& store : snapshot.stores)
    {
        if (matchesRootRelativePath(cleanPath, store.rootUuid,
                                    store.cipherRelativePath, snapshot) ||
            matchesRootRelativePath(cleanPath, store.rootUuid,
                                    store.configRelativePath, snapshot))
        {
            return true;
        }
    }

    for (const PrivacyTransactionJournal& journal :
         snapshot.transactionJournals)
    {
        if (matchesRootRelativePath(cleanPath, journal.rootUuid,
                                    journal.journalRelativePath, snapshot))
        {
            return true;
        }
    }

    for (const PrivacyDerivative& derivative : snapshot.derivatives)
    {
        for (const PrivacyStore& store : snapshot.stores)
        {
            if ((store.uuid == derivative.storeUuid) &&
                matchesRootRelativePath(cleanPath, store.rootUuid,
                                        derivative.protectedRelativePath,
                                        snapshot))
            {
                return true;
            }
        }
    }

    return false;
}

bool untrackedPublicPathIsSafe(
    const QString& path,
    const PrivacyRepositorySnapshot& snapshot)
{
    const QString cleanPath = QDir::cleanPath(path);
    const QFileInfo info(cleanPath);
    const QString canonical = info.canonicalFilePath();

    if (cleanPath.isEmpty() || !QDir::isAbsolutePath(cleanPath) ||
        cleanPath.contains(QChar::Null) || !info.exists() || !info.isFile() ||
        info.isSymLink() || canonical.isEmpty() ||
        (QDir::cleanPath(canonical) != cleanPath) ||
        isReservedPrivacyPath(cleanPath, snapshot))
    {
        return false;
    }

    const QString runtimeRoot = QDir(
        QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation))
                                    .filePath(QLatin1String("digikam-private"));

    return !pathIsInside(cleanPath, runtimeRoot);
}

bool buildPrivacyRequest(const DItemAccessRequest& request,
                         PrivacyRequestedSource source,
                         PrivacyActionRequest* const privacyRequest,
                         QList<int>* const requestIndexes)
{
    if (!privacyRequest || !requestIndexes)
    {
        return false;
    }

    PrivacyActionRequest built;
    built.actionKind       = actionKindFor(request.purpose);
    built.consumerIdentity = request.consumerIdentity;
    built.requestedSource  = source;
    built.mutationPolicy   = mutationPolicyFor(request.mutation);

    for (int index = 0 ; index < request.logicalUrls.size() ; ++index)
    {
        const QUrl& url = request.logicalUrls.at(index);
        const ItemInfo info = ItemInfo::fromUrl(url);

        if (info.id() <= 0)
        {
            continue;
        }

        PrivacyActionItem item;
        item.imageId    = info.id();
        item.publicPath = QDir::cleanPath(url.toLocalFile());
        built.items << item;
        requestIndexes->append(index);
    }

    if (!built.isValid())
    {
        return false;
    }

    *privacyRequest = built;
    return true;
}

bool brokerSupportsProtectedRequest(const DItemAccessRequest& request)
{
    return ((request.requestedSource ==
             DItemAccessSource::PublicRepresentation) ||
            (request.requestedSource ==
             DItemAccessSource::InternalOriginal));
}

class PreparedProxySnapshot final : public PrivacySourceLifetime
{
public:

    ~PreparedProxySnapshot()
    {
        if (!physicalPath.isEmpty() &&
            !physicalPath.startsWith(QLatin1String("/proc/self/fd/")))
        {
            QFile::remove(physicalPath);
        }
    }

public:

    QSharedPointer<QTemporaryFile> backing;
    QString                        physicalPath;
    bool                           sameProcessOnly = false;
    DItemAccessFileFacts           fileFacts;
};

QSharedPointer<PreparedProxySnapshot> prepareProxySnapshot(
    const QSharedPointer<PrivacyRuntimeCoordinator>& runtime,
    qlonglong imageId, const QString& logicalPath)
{
    if (!runtime || (imageId <= 0))
    {
        return {};
    }

    QString runtimeBase = QStandardPaths::writableLocation(
        QStandardPaths::RuntimeLocation);

    if (runtimeBase.isEmpty())
    {
        runtimeBase = QStandardPaths::writableLocation(
            QStandardPaths::TempLocation);
    }

    const QString runtimePath = QDir(runtimeBase).filePath(
        QLatin1String("digikam-private/prepared-proxies"));

    if (runtimePath.isEmpty() || !QDir().mkpath(runtimePath) ||
        !QFile::setPermissions(runtimePath, QFileDevice::ReadOwner |
                                           QFileDevice::WriteOwner |
                                           QFileDevice::ExeOwner))
    {
        return {};
    }

    QSharedPointer<PreparedProxySnapshot> snapshot(new PreparedProxySnapshot);
    snapshot->backing.reset(new QTemporaryFile(
        QDir(runtimePath).filePath(QLatin1String("proxy-XXXXXX"))));

    if (!snapshot->backing->open() ||
        !snapshot->backing->setPermissions(QFileDevice::ReadOwner |
                                            QFileDevice::WriteOwner) ||
        !snapshot->backing->resize(0) ||
        !runtime->snapshotVerifiedPublicProxy(
            imageId, logicalPath, snapshot->backing.data()) ||
        !snapshot->backing->flush() ||
        !snapshot->backing->setPermissions(QFileDevice::ReadOwner))
    {
        return {};
    }

    snapshot->fileFacts = fileFactsForPath(logicalPath);

    if (!snapshot->fileFacts.available)
    {
        return {};
    }

#if defined(Q_OS_LINUX)

    const QString temporaryPath = snapshot->backing->fileName();

    if ((snapshot->backing->handle() < 0) ||
        !QFile::remove(temporaryPath))
    {
        return {};
    }

    snapshot->physicalPath = QLatin1String("/proc/self/fd/") +
                             QString::number(snapshot->backing->handle());
    snapshot->sameProcessOnly = true;

#else

    snapshot->physicalPath = snapshot->backing->fileName();

#endif


    return snapshot;
}

class PreparedAccessLifetime final
{
public:

    PreparedAccessLifetime(
        const QSharedPointer<PrivacyLeaseRegistry>& leases,
        const PrivacyPreparedAccessToken& accessToken)
        : m_leases(leases),
          m_accessToken(accessToken)
    {
    }

    ~PreparedAccessLifetime()
    {
        if (m_leases)
        {
            m_leases->revokeAll();
        }

        if (m_accessToken.isValid())
        {
            PrivacyPreparedAccessRegistry::release(m_accessToken);
        }
    }

    QSharedPointer<PrivacyLeaseRegistry> leases() const
    {
        return m_leases;
    }

private:

    QSharedPointer<PrivacyLeaseRegistry> m_leases;
    PrivacyPreparedAccessToken           m_accessToken;
};

class DBPreparedItemSourceHandle final : public DItemAccessSourceHandle
{
public:

    DBPreparedItemSourceHandle(
        const DItemAccessEntry& entry,
        const QList<DItemAssociatedAccessEntry>& associatedEntries,
        const QSharedPointer<PreparedAccessLifetime>& lifetime,
        const PrivacyLeaseToken& lease,
        const QList<QSharedPointer<PrivacySourceUseGuard> >& sourceUses,
        const QList<PrivacySourceResult>& sources,
        const QSharedPointer<PrivacyRuntimeCoordinator>& runtime,
        qlonglong proxyImageId)
        : DItemAccessSourceHandle(entry, associatedEntries),
          m_lifetime(lifetime),
          m_lease(lease),
          m_sourceUses(sourceUses),
          m_sources(sources),
          m_runtime(runtime),
          m_proxyImageId(proxyImageId)
    {
    }

    bool validateAccess() const override
    {
        const QSharedPointer<PrivacyLeaseRegistry> leases =
            m_lifetime ? m_lifetime->leases()
                       : QSharedPointer<PrivacyLeaseRegistry>();

        if (!DItemAccessSourceHandle::validateAccess() || !leases ||
            m_sourceUses.isEmpty() ||
            (m_sourceUses.size() != m_sources.size()) ||
            (leases->validate(m_lease) != PrivacyLeaseValidation::Valid))
        {
            return false;
        }

        for (const QSharedPointer<PrivacySourceUseGuard>& sourceUse : m_sourceUses)
        {
            if (!sourceUse || !sourceUse->isAcquired())
            {
                return false;
            }
        }

        if (m_proxyImageId <= 0)
        {
            return !m_runtime;
        }

        return (m_runtime &&
                (m_runtime->validatePublicProxyForDisplay(
                     m_proxyImageId, entry().physicalUrl.toLocalFile()) ==
                 PrivacyPublicProxyDisplayResult::Verified));
    }

private:

    QSharedPointer<PreparedAccessLifetime> m_lifetime;
    PrivacyLeaseToken                    m_lease;
    QList<QSharedPointer<PrivacySourceUseGuard> > m_sourceUses;
    QList<PrivacySourceResult>                   m_sources;
    QSharedPointer<PrivacyRuntimeCoordinator>    m_runtime;
    qlonglong                                    m_proxyImageId = -1;
};

class PreparedAssociatedSpec
{
public:

    QUrl logicalUrl;
    int  role = 0;
    int  ordinal = -1;
    DItemAccessFileFacts fileFacts;
};

class DBPreparedItemAccessHandle final : public DItemAccessHandle
{
public:

    DBPreparedItemAccessHandle(
        const QList<DItemAccessEntry>& entries,
        const QList<QUrl>& excludedLogicalUrls,
        const PreparedPrivacySelection& selection,
        const QSharedPointer<PreparedAccessLifetime>& lifetime,
        const QList<QSharedPointer<PrivacySourceUseGuard> >& sourceUses,
        const QSharedPointer<PrivacyRuntimeCoordinator>& runtime,
        const QHash<QString, qlonglong>& proxyImageIds,
        const QSet<QString>& passThroughPaths,
        const QHash<QString, qlonglong>& deferredProxyImageIds,
        const QHash<QString, QList<PreparedAssociatedSpec> >& associatedByPath)
        : DItemAccessHandle(entries, excludedLogicalUrls, false),
          m_selection(selection),
          m_lifetime(lifetime),
          m_sourceUses(sourceUses),
          m_runtime(runtime),
          m_proxyImageIds(proxyImageIds),
          m_passThroughPaths(passThroughPaths),
          m_deferredProxyImageIds(deferredProxyImageIds),
          m_associatedByPath(associatedByPath)
    {
    }

    ~DBPreparedItemAccessHandle() override
    {
        m_sourceUses.clear();
    }

    bool validateAccess(const QUrl& physicalUrl) const override
    {
        if (!DItemAccessHandle::validateAccess(physicalUrl) ||
            !m_selection.isValid())
        {
            return false;
        }

        const QString physicalPath =
            QDir::cleanPath(physicalUrl.toLocalFile());

        if (m_passThroughPaths.contains(physicalPath))
        {
            return true;
        }

        for (const PreparedPrivacyItem& item : m_selection.items)
        {
            if (QDir::cleanPath(item.physicalPath) != physicalPath)
            {
                continue;
            }

            if (item.disposition ==
                PrivacyPreparedDisposition::UnprotectedPassThrough)
            {
                return true;
            }

            const QSharedPointer<PrivacyLeaseRegistry> leases =
                m_lifetime ? m_lifetime->leases()
                           : QSharedPointer<PrivacyLeaseRegistry>();
            const bool leaseValid =
                (leases &&
                 (item.disposition == PrivacyPreparedDisposition::Allowed) &&
                 (leases->validate(item.lease) ==
                  PrivacyLeaseValidation::Valid));
            const auto proxy = m_proxyImageIds.constFind(physicalPath);

            if (proxy == m_proxyImageIds.constEnd())
            {
                return leaseValid;
            }

            return (leaseValid && m_runtime &&
                    (m_runtime->validatePublicProxyForDisplay(
                         proxy.value(), physicalPath) ==
                     PrivacyPublicProxyDisplayResult::Verified));
        }

        return false;
    }

    QSharedPointer<DItemAccessSourceHandle> acquireSource(
        const QUrl& logicalUrl,
        const QSharedPointer<DItemAccessCancellationToken>& cancellation) const override
    {
        if (cancellation && cancellation->isCanceled())
        {
            return {};
        }

        DItemAccessEntry requested;

        for (const DItemAccessEntry& entry : entries())
        {
            if (entry.logicalUrl == logicalUrl)
            {
                requested = entry;
                break;
            }
        }

        if (!requested.isValid())
        {
            return {};
        }

        if (!requested.deferred)
        {
            const QString physicalPath = QDir::cleanPath(
                requested.physicalUrl.toLocalFile());
            const auto proxy = m_proxyImageIds.constFind(physicalPath);

            if (proxy == m_proxyImageIds.constEnd())
            {
                return DItemAccessHandle::acquireSource(
                    logicalUrl, cancellation);
            }

            const PreparedPrivacyItem* prepared = nullptr;

            for (const PreparedPrivacyItem& item : m_selection.items)
            {
                if ((QDir::cleanPath(item.physicalPath) == physicalPath) &&
                    (item.disposition ==
                     PrivacyPreparedDisposition::Allowed))
                {
                    prepared = &item;
                    break;
                }
            }

            const QSharedPointer<PrivacyLeaseRegistry> leases =
                m_lifetime ? m_lifetime->leases()
                           : QSharedPointer<PrivacyLeaseRegistry>();

            if (!prepared || !leases || !m_runtime ||
                (leases->validate(prepared->lease) !=
                 PrivacyLeaseValidation::Valid) ||
                (m_runtime->validatePublicProxyForDisplay(
                     proxy.value(), physicalPath) !=
                 PrivacyPublicProxyDisplayResult::Verified))
            {
                return {};
            }

            const PrivacySourceResult proxySource =
                PrivacySourceResult::resolved(
                    physicalPath,
                    m_runtime->publicSourceCacheNamespace(proxy.value()),
                    PrivacySourceResult::Persistent);
            const QSharedPointer<PrivacySourceUseGuard> proxyUse(
                new PrivacySourceUseGuard(physicalPath, proxySource));

            if (!proxyUse->isAcquired())
            {
                return {};
            }

            QSharedPointer<DItemAccessSourceHandle> acquired(
                new DBPreparedItemSourceHandle(
                    requested, {}, m_lifetime, prepared->lease,
                    { proxyUse }, { proxySource }, m_runtime, proxy.value()));
            return acquired->validateAccess()
                 ? acquired : QSharedPointer<DItemAccessSourceHandle>();
        }

        const QString logicalPath = QDir::cleanPath(logicalUrl.toLocalFile());
        const PreparedPrivacyItem* prepared = nullptr;

        for (const PreparedPrivacyItem& item : m_selection.items)
        {
            if ((QDir::cleanPath(item.physicalPath) == logicalPath) &&
                (item.disposition == PrivacyPreparedDisposition::Allowed))
            {
                prepared = &item;
                break;
            }
        }

        const QSharedPointer<PrivacyLeaseRegistry> leases =
            m_lifetime ? m_lifetime->leases()
                       : QSharedPointer<PrivacyLeaseRegistry>();

        if (!prepared || !leases ||
            (leases->validate(prepared->lease) !=
             PrivacyLeaseValidation::Valid))
        {
            return {};
        }

        const auto deferredProxy =
            m_deferredProxyImageIds.constFind(logicalPath);

        if (deferredProxy != m_deferredProxyImageIds.constEnd())
        {
            const QSharedPointer<PreparedProxySnapshot> snapshot =
                prepareProxySnapshot(m_runtime, deferredProxy.value(),
                                     logicalPath);

            if (!snapshot || snapshot->physicalPath.isEmpty() ||
                (cancellation && cancellation->isCanceled()))
            {
                return {};
            }

            PrivacySourceResult proxySource = PrivacySourceResult::resolved(
                snapshot->physicalPath,
                m_runtime->publicSourceCacheNamespace(deferredProxy.value()),
                PrivacySourceResult::MemoryOnly);
            proxySource.lifetimeOwner = snapshot;
            const QSharedPointer<PrivacySourceUseGuard> proxyUse(
                new PrivacySourceUseGuard(logicalPath, proxySource));

            if (!proxyUse->isAcquired())
            {
                return {};
            }

            requested.physicalUrl = QUrl::fromLocalFile(snapshot->physicalPath);
            requested.deferred = false;
            requested.sameProcessOnly = true;
            requested.fileFacts = snapshot->fileFacts;
            QSharedPointer<DItemAccessSourceHandle> acquired(
                new DBPreparedItemSourceHandle(
                    requested, {}, m_lifetime, prepared->lease,
                    { proxyUse }, { proxySource }, {}, -1));
            return acquired->validateAccess()
                 ? acquired : QSharedPointer<DItemAccessSourceHandle>();
        }

        PrivacySourceRequest sourceRequest;
        sourceRequest.logicalFilePath = logicalPath;
        sourceRequest.itemReference   = prepared->logicalItem.imageId;
        sourceRequest.consumer        = PrivacySourceRequest::PreparedAccess;
        sourceRequest.isCancelled     = [cancellation]()
        {
            return cancellation && cancellation->isCanceled();
        };
        const PrivacySourceResult source =
            PrivacySourceResolver::resolve(sourceRequest);

        if ((source.disposition != PrivacySourceResult::Resolved) ||
            source.physicalFilePath.isEmpty() ||
            !source.encodedBytes.isEmpty() || !source.lifetimeOwner ||
            (QDir::cleanPath(source.physicalFilePath) == logicalPath))
        {
            return {};
        }

        const QSharedPointer<PrivacySourceUseGuard> sourceUse(
            new PrivacySourceUseGuard(logicalPath, source));

        if (!sourceUse->isAcquired())
        {
            return {};
        }

        requested.physicalUrl = QUrl::fromLocalFile(source.physicalFilePath);
        requested.deferred = false;
        requested.sameProcessOnly = true;
        QList<QSharedPointer<PrivacySourceUseGuard> > acquiredUses { sourceUse };
        QList<PrivacySourceResult> acquiredSources { source };
        QList<DItemAssociatedAccessEntry> associatedEntries;

        for (const PreparedAssociatedSpec& spec :
             m_associatedByPath.value(logicalPath))
        {
            if (cancellation && cancellation->isCanceled())
            {
                return {};
            }

            PrivacySourceRequest associatedRequest;
            associatedRequest.logicalFilePath = logicalPath;
            associatedRequest.itemReference = prepared->logicalItem.imageId;
            associatedRequest.consumer = PrivacySourceRequest::PreparedAccess;
            associatedRequest.assetRole = spec.role;
            associatedRequest.assetOrdinal = spec.ordinal;
            associatedRequest.isCancelled = sourceRequest.isCancelled;
            const PrivacySourceResult associatedSource =
                PrivacySourceResolver::resolve(associatedRequest);

            if ((associatedSource.disposition !=
                 PrivacySourceResult::Resolved) ||
                associatedSource.physicalFilePath.isEmpty() ||
                !associatedSource.encodedBytes.isEmpty() ||
                !associatedSource.lifetimeOwner)
            {
                return {};
            }

            const QSharedPointer<PrivacySourceUseGuard> associatedUse(
                new PrivacySourceUseGuard(
                    logicalPath, associatedSource));

            if (!associatedUse->isAcquired())
            {
                return {};
            }

            DItemAssociatedAccessEntry associated;
            associated.logicalUrl = spec.logicalUrl;
            associated.physicalUrl = QUrl::fromLocalFile(
                associatedSource.physicalFilePath);
            associated.role = spec.role;
            associated.ordinal = spec.ordinal;
            associated.sameProcessOnly = true;
            associated.fileFacts = spec.fileFacts;

            if (!associated.isValid())
            {
                return {};
            }

            acquiredUses << associatedUse;
            acquiredSources << associatedSource;
            associatedEntries << associated;
        }

        QSharedPointer<DItemAccessSourceHandle> acquired(
            new DBPreparedItemSourceHandle(requested, associatedEntries,
                                           m_lifetime, prepared->lease,
                                           acquiredUses, acquiredSources,
                                           {}, -1));
        return acquired->validateAccess()
             ? acquired : QSharedPointer<DItemAccessSourceHandle>();
    }

private:

    PreparedPrivacySelection                 m_selection;
    QSharedPointer<PreparedAccessLifetime>    m_lifetime;
    QList<QSharedPointer<PrivacySourceUseGuard> > m_sourceUses;
    QSharedPointer<PrivacyRuntimeCoordinator>     m_runtime;
    QHash<QString, qlonglong>                     m_proxyImageIds;
    QSet<QString>                                 m_passThroughPaths;
    QHash<QString, qlonglong>                     m_deferredProxyImageIds;
    QHash<QString, QList<PreparedAssociatedSpec> > m_associatedByPath;
};

} // namespace

QSharedPointer<DItemAccessHandle> preparePrivacyItemAccess(
    const DItemAccessRequest& request)
{
    const QSharedPointer<PrivacyRuntimeCoordinator> runtime =
        PrivacyStartupRecovery::coordinator();
    const QSharedPointer<const PrivacyLeaseStateProvider> leaseState =
        PrivacyStartupRecovery::leaseStateProvider();
    PrivacyRepositorySnapshot repositorySnapshot;

    if (!runtime || !leaseState ||
        !PrivacyRepository().loadRuntimeSnapshot(&repositorySnapshot))
    {
        return {};
    }

    QHash<int, DItemAccessEntry> directEntries;
    QSet<QString> passThroughPaths;

    for (int index = 0 ; index < request.logicalUrls.size() ; ++index)
    {
        const QUrl& url = request.logicalUrls.at(index);

        if (ItemInfo::fromUrl(url).id() > 0)
        {
            continue;
        }

        const QString path = QDir::cleanPath(url.toLocalFile());

        if (!untrackedPublicPathIsSafe(path, repositorySnapshot))
        {
            return DItemAccessHandle::canceled(request);
        }

        directEntries.insert(
            index, DItemAccessEntry { url, url, false, false, false, {} });
        passThroughPaths.insert(path);
    }

    const PrivacyRequestedSource requestedSource =
        requestedSourceFor(request.requestedSource);
    PrivacyActionRequest privacyRequest;
    QList<int> privacyIndexes;

    if (!buildPrivacyRequest(request, requestedSource, &privacyRequest,
                             &privacyIndexes))
    {
        return directEntries.size() == request.logicalUrls.size()
             ? DItemAccessHandle::passThrough(request)
             : QSharedPointer<DItemAccessHandle>();
    }

    PrivacyActionPolicyResult policy =
        PrivacyActionGate::classify(privacyRequest);

    if (!policy.isValid())
    {
        return {};
    }

    if ((policy.protectedItemCount > 0) &&
        !brokerSupportsProtectedRequest(request))
    {
        // Writable checkouts and public-original exposure require their own
        // journalled owners. This broker must not prompt as though it can
        // prepare either source.

        return DItemAccessHandle::canceled(request);
    }

    bool excludeUnavailable = false;

    const auto useProxyPolicy = [&]()
    {
        PrivacyActionRequest proxyRequest;
        QList<int> proxyIndexes;

        if (!buildPrivacyRequest(request, PrivacyRequestedSource::PublicProxy,
                                 &proxyRequest, &proxyIndexes) ||
            (proxyIndexes != privacyIndexes))
        {
            return false;
        }

        const PrivacyActionPolicyResult proxyPolicy =
            PrivacyActionGate::classify(proxyRequest);

        if (!proxyPolicy.isValid())
        {
            return false;
        }

        if (proxyPolicy.isImmediatelyReady())
        {
            privacyRequest = proxyRequest;
            policy         = proxyPolicy;
            return true;
        }

        if (request.allowPartialSelection)
        {
            for (const PrivacyActionPolicyItem& item : proxyPolicy.items)
            {
                if (isReadyDisposition(item.disposition))
                {
                    privacyRequest    = proxyRequest;
                    policy            = proxyPolicy;
                    excludeUnavailable = true;
                    return true;
                }
            }
        }

        return false;
    };

    if (!policy.isImmediatelyReady())
    {
        PrivacyActionPolicyResult promptPolicy = policy;

        if (request.allowPlaceholderFallback)
        {
            PrivacyActionRequest proxyRequest;
            QList<int> proxyIndexes;

            if (buildPrivacyRequest(request,
                                    PrivacyRequestedSource::PublicProxy,
                                    &proxyRequest, &proxyIndexes) &&
                (proxyIndexes == privacyIndexes))
            {
                const PrivacyActionPolicyResult proxyPolicy =
                    PrivacyActionGate::classify(proxyRequest);
                bool anyProxyReady = false;

                if (proxyPolicy.isValid())
                {
                    for (const PrivacyActionPolicyItem& item : proxyPolicy.items)
                    {
                        anyProxyReady = anyProxyReady ||
                                        isReadyDisposition(item.disposition);
                    }
                }

                promptPolicy.canContinueWithProxy =
                    proxyPolicy.isValid() &&
                    (proxyPolicy.isImmediatelyReady() ||
                     (request.allowPartialSelection && anyProxyReady));
                promptPolicy.canExcludeAffected =
                    promptPolicy.canExcludeAffected ||
                    (request.allowPartialSelection && anyProxyReady);
            }
        }

        const AccessResolution resolution =
            promptAccessResolution(request, promptPolicy);

        if (resolution == AccessResolution::Cancel)
        {
            return DItemAccessHandle::canceled(request);
        }

        if (resolution == AccessResolution::Unlock)
        {
            const bool unlocked = unlockCategories(
                policy.affectedCategoryUuids);

            if (unlocked)
            {
                policy = PrivacyActionGate::classify(privacyRequest);
            }

            if (!unlocked || !policy.isImmediatelyReady())
            {
                if (!request.allowPlaceholderFallback || !useProxyPolicy())
                {
                    return DItemAccessHandle::canceled(request);
                }
            }
        }
        else if (resolution == AccessResolution::Proxy)
        {
            if (!useProxyPolicy())
            {
                return DItemAccessHandle::canceled(request);
            }
        }
        else if (resolution == AccessResolution::Exclude)
        {
            excludeUnavailable = true;
        }
    }

    if (policy.items.size() != privacyIndexes.size())
    {
        return {};
    }

    const QSharedPointer<PrivacyLeaseRegistry> leases(
        new PrivacyLeaseRegistry(leaseState));
    QList<QSharedPointer<PrivacySourceUseGuard> > sourceUses;
    QList<DItemAccessEntry> entries;
    QList<QUrl> excludedLogicalUrls;
    PreparedPrivacySelection selection;
    selection.disposition = PrivacyPreparedDisposition::Allowed;
    QSet<QString> preparedCategories;
    QHash<QString, qlonglong> proxyImageIds;
    QHash<QString, qlonglong> deferredProxyImageIds;
    QHash<QString, QList<PreparedAssociatedSpec> > associatedByPath;

    for (int index = 0 ; index < policy.items.size() ; ++index)
    {
        const PrivacyActionPolicyItem& policyItem = policy.items.at(index);
        const int requestIndex = privacyIndexes.at(index);
        const QUrl logicalUrl = request.logicalUrls.at(requestIndex);
        PreparedPrivacyItem preparedItem;
        preparedItem.logicalItem = policyItem.logicalItem;

        if (!isReadyDisposition(policyItem.disposition))
        {
            if (!excludeUnavailable)
            {
                return {};
            }

            preparedItem.disposition = PrivacyPreparedDisposition::Excluded;
            selection.items << preparedItem;
            excludedLogicalUrls << logicalUrl;
            continue;
        }

        if (policyItem.disposition ==
            PrivacyActionPolicyDisposition::UnprotectedPassThrough)
        {
            preparedItem.physicalPath = logicalUrl.toLocalFile();
            preparedItem.disposition =
                PrivacyPreparedDisposition::UnprotectedPassThrough;
            selection.items << preparedItem;
            directEntries.insert(
                requestIndex,
                DItemAccessEntry {
                    logicalUrl, logicalUrl, false, false, false, {}
                });
            passThroughPaths.insert(
                QDir::cleanPath(logicalUrl.toLocalFile()));
            continue;
        }

        PrivacySourceResult source;
        bool dependsOnStore = false;
        bool deferredSource = false;
        DItemAccessFileFacts preparedFileFacts;

        if (policyItem.disposition ==
            PrivacyActionPolicyDisposition::ReadyWithInternalOriginal)
        {
            dependsOnStore = true;
            deferredSource = true;
            QList<PreparedAssociatedSpec> associatedSpecs;
            bool foundPrimary = false;

            for (const PrivacyAsset& asset : repositorySnapshot.assets)
            {
                if (asset.itemUuid != policyItem.itemUuid)
                {
                    continue;
                }

                const DItemAccessFileFacts assetFacts = fileFactsForAsset(asset);

                if (!assetFacts.available)
                {
                    return {};
                }

                if ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
                    (asset.ordinal == 0))
                {
                    if (foundPrimary)
                    {
                        return {};
                    }

                    foundPrimary = true;
                    preparedFileFacts = assetFacts;
                    continue;
                }

                const PrivacyStorageRoot* assetRoot = nullptr;

                for (const PrivacyStorageRoot& root :
                     repositorySnapshot.storageRoots)
                {
                    if (root.uuid == asset.publicRootUuid)
                    {
                        if (assetRoot)
                        {
                            return {};
                        }

                        assetRoot = &root;
                    }
                }

                if (!assetRoot || !assetRoot->isValid() ||
                    (assetRoot->kind != PrivacyStorageRootKind::AlbumRoot))
                {
                    return {};
                }

                const QString associatedPath = QDir::cleanPath(
                    QDir(assetRoot->configuredPath).absoluteFilePath(
                        asset.publicRelativePath));

                if (!pathIsInside(associatedPath,
                                  assetRoot->configuredPath))
                {
                    return {};
                }

                PreparedAssociatedSpec spec;
                spec.logicalUrl = QUrl::fromLocalFile(associatedPath);
                spec.role       = asset.role;
                spec.ordinal    = asset.ordinal;
                spec.fileFacts  = assetFacts;
                associatedSpecs << spec;
            }

            if (!foundPrimary)
            {
                return {};
            }

            std::sort(associatedSpecs.begin(), associatedSpecs.end(),
                      [](const PreparedAssociatedSpec& left,
                         const PreparedAssociatedSpec& right)
                      {
                          return (left.role < right.role) ||
                                 ((left.role == right.role) &&
                                  (left.ordinal < right.ordinal));
                      });
            associatedByPath.insert(
                QDir::cleanPath(logicalUrl.toLocalFile()), associatedSpecs);
        }
        else if (policyItem.disposition ==
                 PrivacyActionPolicyDisposition::ReadyWithProxy)
        {
            if (request.consumerScope == DItemAccessConsumerScope::SameProcess)
            {
                deferredSource = true;
                deferredProxyImageIds.insert(
                    QDir::cleanPath(logicalUrl.toLocalFile()),
                    policyItem.logicalItem.imageId);
            }
            else
            {
                if (runtime->validatePublicProxyForDisplay(
                        policyItem.logicalItem.imageId,
                        logicalUrl.toLocalFile()) !=
                    PrivacyPublicProxyDisplayResult::Verified)
                {
                    return {};
                }

                source = PrivacySourceResult::resolved(
                    logicalUrl.toLocalFile(),
                    runtime->publicSourceCacheNamespace(
                        policyItem.logicalItem.imageId),
                    PrivacySourceResult::Persistent);
            }
        }
        else
        {
            // Writable checkouts and public-original exposure need their own
            // journalled owners; a URL substitution must not emulate either.

            return {};
        }

        QSharedPointer<PrivacySourceUseGuard> sourceUse;

        if (!deferredSource)
        {
            sourceUse.reset(
                new PrivacySourceUseGuard(logicalUrl.toLocalFile(), source));

            if (!sourceUse->isAcquired())
            {
                return {};
            }
        }

        const PrivacyLeaseToken lease = leases->issue(
            policyItem.itemUuid, dependsOnStore);

        if (!lease.isValid() ||
            (leases->validate(lease) != PrivacyLeaseValidation::Valid))
        {
            return {};
        }

        const QString physicalPath = deferredSource
            ? logicalUrl.toLocalFile() : source.physicalFilePath;
        const QUrl physicalUrl = QUrl::fromLocalFile(physicalPath);

        if (!physicalUrl.isValid() || physicalUrl.toLocalFile().isEmpty() ||
            !QDir::isAbsolutePath(physicalUrl.toLocalFile()))
        {
            return {};
        }

        if (sourceUse)
        {
            sourceUses << sourceUse;
        }

        preparedCategories.insert(policyItem.categoryUuid);

        if (!dependsOnStore && !deferredSource)
        {
            proxyImageIds.insert(QDir::cleanPath(physicalPath),
                                 policyItem.logicalItem.imageId);
        }

        preparedItem.physicalPath = physicalPath;
        preparedItem.disposition  = PrivacyPreparedDisposition::Allowed;
        preparedItem.lease        = lease;
        selection.items << preparedItem;
        directEntries.insert(requestIndex, DItemAccessEntry {
            logicalUrl,
            deferredSource ? QUrl() : physicalUrl,
            policyItem.disposition ==
                PrivacyActionPolicyDisposition::ReadyWithProxy,
            deferredSource,
            (policyItem.disposition ==
                 PrivacyActionPolicyDisposition::ReadyWithInternalOriginal) ||
            ((policyItem.disposition ==
                  PrivacyActionPolicyDisposition::ReadyWithProxy) &&
             (request.consumerScope == DItemAccessConsumerScope::SameProcess)),
            preparedFileFacts
        });
    }

    for (int index = 0 ; index < request.logicalUrls.size() ; ++index)
    {
        if (directEntries.contains(index))
        {
            entries << directEntries.value(index);
        }
    }

    if (entries.isEmpty() || !selection.isValid())
    {
        return DItemAccessHandle::canceled(request);
    }

    PrivacyPreparedAccessToken accessToken;

    if (!preparedCategories.isEmpty())
    {
        QStringList categories = preparedCategories.values();
        categories.sort();
        accessToken = PrivacyPreparedAccessRegistry::acquire(categories);

        if (!accessToken.isValid())
        {
            return {};
        }
    }

    const QSharedPointer<PreparedAccessLifetime> lifetime(
        new PreparedAccessLifetime(leases, accessToken));
    QSharedPointer<DItemAccessHandle> handle(
        new DBPreparedItemAccessHandle(entries, excludedLogicalUrls,
                                       selection, lifetime, sourceUses, runtime,
                                       proxyImageIds, passThroughPaths,
                                       deferredProxyImageIds,
                                       associatedByPath));

    return (handle->isValid() ? handle : QSharedPointer<DItemAccessHandle>());
}

} // namespace Digikam
