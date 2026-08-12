/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-06-07
 * Description : Face database backend
 *
 * SPDX-FileCopyrightText: 2007-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class FaceDbSchemaUpdater;
class FaceDbBackendPrivate;

class DIGIKAM_GUI_EXPORT FaceDbBackend : public BdEngineBackend
{
    Q_OBJECT

public:

    explicit FaceDbBackend(DbEngineLocking* const locking,
                           const QString& backendName = QLatin1String("faceDatabase-"));
    ~FaceDbBackend() override = default;

    /**
     * @brief Initialize the database schema to the current version,
     * carry out upgrades if necessary.
     * Shall only be called from the thread that called open().
     */
    bool initSchema(FaceDbSchemaUpdater* const updater);

private:

    /// @note disabled
    explicit FaceDbBackend(QObject*) = delete;

    Q_DECLARE_PRIVATE(BdEngineBackend)
};

} // namespace Digikam
