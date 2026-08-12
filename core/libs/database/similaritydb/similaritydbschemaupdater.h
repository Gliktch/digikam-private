/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-30
 * Description : Similarity DB schema update
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2017 by Swati  Lodha   <swatilodha27 at gmail dot com>
 * SPDX-FileCopyrightText:      2018 by Mario  Frank   <mario dot frank at uni minus potsdam dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

namespace Digikam
{

class SimilarityDbAccess;
class InitializationObserver;

class SimilarityDbSchemaUpdater
{
public:

    static int schemaVersion();

public:

    explicit SimilarityDbSchemaUpdater(SimilarityDbAccess* const dbAccess);
    ~SimilarityDbSchemaUpdater();

    bool update();
    void setObserver(InitializationObserver* const observer);

private:

    bool startUpdates();
    bool makeUpdates();
    bool createDatabase();
    bool createTables();
    bool createIndices();
    bool createTriggers();
    bool updateV1ToV2();

private:

    /// @note disabled
    SimilarityDbSchemaUpdater(const SimilarityDbSchemaUpdater&)            = delete;
    SimilarityDbSchemaUpdater& operator=(const SimilarityDbSchemaUpdater&) = delete;

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
