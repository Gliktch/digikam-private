/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : Similarity database backend
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2017 by Swati  Lodha   <swatilodha27 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "dbenginebackend.h"

namespace Digikam
{

class SimilarityDbSchemaUpdater;

class DIGIKAM_DATABASE_EXPORT SimilarityDbBackend : public BdEngineBackend
{
    Q_OBJECT

public:

    explicit SimilarityDbBackend(DbEngineLocking* const locking,
                                 const QString& backendName = QLatin1String("similarityDatabase-"));
    ~SimilarityDbBackend() override = default;

    /**
     * Initialize the database schema to the current version,
     * carry out upgrades if necessary.
     * Shall only be called from the thread that called open().
     */
    bool initSchema(SimilarityDbSchemaUpdater* const updater);

private:

    /// @note disabled
    explicit SimilarityDbBackend(QObject*) = delete;

    Q_DECLARE_PRIVATE(BdEngineBackend)
};

} // namespace Digikam
