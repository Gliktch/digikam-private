/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Core database Schema updater
 *
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QVariant>

// Local includes

#include "coredb.h"
#include "coredbaccess.h"
#include "coredbbackend.h"
#include "dbengineparameters.h"

namespace Digikam
{

class CoreDbAccess;
class InitializationObserver;

class DIGIKAM_DATABASE_EXPORT CoreDbSchemaUpdater
{
public:

    static int  schemaVersion();
    static int  filterSettingsVersion();
    static int  uniqueHashVersion();
    static bool isUniqueHashUpToDate();

public:

    explicit CoreDbSchemaUpdater(CoreDB* const albumDB,
                                 CoreDbBackend* const backend,
                                 const DbEngineParameters& parameters);
    virtual ~CoreDbSchemaUpdater();

    bool  update();
    bool  updateUniqueHash();
    void  setObserver(InitializationObserver* const observer);
    const QString getLastErrorMessage();
    void  setCoreDbAccess(CoreDbAccess* const dbAccess);

private:

    bool startUpdates();
    bool makeUpdates();
    bool beginWrapSchemaUpdateStep();
    bool endWrapSchemaUpdateStep(bool stepOperationSuccess, const QString& errorMsg);
    void defaultFilterSettings(QStringList& defaultItemFilter,
                               QStringList& defaultVideoFilter,
                               QStringList& defaultAudioFilter);
    void defaultIgnoreDirectoryFilterSettings(QStringList& defaultIgnoreDirectoryFilter);
    bool createFilterSettings();
    bool updateFilterSettings();
    bool createDatabase();
    bool createTables();
    bool createIndices();
    bool createTriggers();
    bool copyV3toV4(const QString& digikam3DBPath, const QString& currentDBPath);
    bool performUpdateToVersion(const QString& actionName, int newVersion, int newRequiredVersion);
    bool updateToVersion(int targetVersion);
    bool updateV4toV7();
    void setLegacySettingEntries();
    void readVersionSettings();
    void setVersionSettings();

private:

    // cppcheck-suppress unusedPrivateFunction
    bool createTablesV3();
    // cppcheck-suppress unusedPrivateFunction
    void preAlpha010Update1();
    // cppcheck-suppress unusedPrivateFunction
    void preAlpha010Update2();
    // cppcheck-suppress unusedPrivateFunction
    void preAlpha010Update3();
    // cppcheck-suppress unusedPrivateFunction
    void beta010Update1();
    // cppcheck-suppress unusedPrivateFunction
    void beta010Update2();

private:

    /// @note disabled
    CoreDbSchemaUpdater(const CoreDbSchemaUpdater&)            = delete;
    CoreDbSchemaUpdater& operator=(const CoreDbSchemaUpdater&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
