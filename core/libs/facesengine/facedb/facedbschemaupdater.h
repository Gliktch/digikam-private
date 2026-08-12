/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : Face database schema updater
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael dot miller at msn dot com>
 * SPDX-FileCopyrightText: 2020      by Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "collectionscannerobserver.h"

namespace Digikam
{

class FaceDbAccess;

class FaceDbSchemaUpdater
{
public:

    static int schemaVersion();

public:

    explicit FaceDbSchemaUpdater(FaceDbAccess* const dbAccess);
    ~FaceDbSchemaUpdater();

    bool update();
    void setObserver(InitializationObserver* const observer);

private:

    bool startUpdates();
    bool makeUpdates();
    bool createDatabase();
    bool createTables();
    bool createIndices();
    bool createTriggers();
    bool populateDbSettings();
    bool updateV1ToV2();
    bool updateV2ToV6();
    bool updateV3ToV6();
    bool updateV4ToV6();
    bool updateV5ToV6();

private:

    /// @note disabled
    FaceDbSchemaUpdater(const FaceDbSchemaUpdater&)            = delete;
    FaceDbSchemaUpdater& operator=(const FaceDbSchemaUpdater&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
