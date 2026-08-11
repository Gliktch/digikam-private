/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "privacycategorysession.h"

namespace Digikam
{

/** Linux production adapter for the pinned gocryptfs category store. */
class DIGIKAM_DATABASE_EXPORT PrivacyGocryptfsCategoryStoreBackend final
    : public PrivacyCategoryStoreBackend
{
public:

    PrivacyGocryptfsCategoryStoreBackend(PrivacyProcessRunner& runner,
                                         const PrivacyMountStateProbe& mountProbe,
                                         PrivacyGocryptfsToolPaths toolPaths,
                                         QString runtimeRoot);

    bool createOrResume(const PrivacyStorageRoot& root,
                        const PrivacyStore& store,
                        const QString& temporaryCipherRelativePath,
                        const PrivacyPassword& password,
                        const QByteArray& sentinel,
                        PrivacyGocryptfsEnvelope* envelope,
                        PrivacyGocryptfsError* error) override;
    bool validateEnvelope(const PrivacyGocryptfsEnvelope& envelope,
                          const PrivacyPassword& password,
                          PrivacyGocryptfsError* error) override;
    std::unique_ptr<PrivacyCategoryStoreLease> unlock(
        const PrivacyStorageRoot& root,
        const PrivacyStore& store,
        const PrivacyGocryptfsEnvelope& envelope,
        const PrivacyPassword& password,
        const QByteArray& sentinel,
        PrivacyGocryptfsError* error) override;
    bool lock(std::unique_ptr<PrivacyCategoryStoreLease>& lease,
              PrivacyGocryptfsError* error) override;
    bool rewrapPassword(const PrivacyStorageRoot& root,
                        const PrivacyStore& store,
                        const PrivacyGocryptfsEnvelope& envelope,
                        const PrivacyPassword& oldPassword,
                        const PrivacyPassword& newPassword,
                        const QByteArray& sentinel,
                        QByteArray* newOpaqueConfig,
                        PrivacyGocryptfsError* error) override;

private:

    PrivacyProcessRunner& m_runner;
    const PrivacyMountStateProbe& m_mountProbe;
    PrivacyGocryptfsToolPaths m_toolPaths;
    QString m_runtimeRoot;
};

} // namespace Digikam
