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

// Local includes

#include "similaritydbbackend.h"
#include "dbenginebackend_p.h"
#include "similaritydbschemaupdater.h"

namespace Digikam
{

SimilarityDbBackend::SimilarityDbBackend(DbEngineLocking* const locking, const QString& backendName)
    : BdEngineBackend(backendName, locking, *new BdEngineBackendPrivate(this))
{
}

bool SimilarityDbBackend::initSchema(SimilarityDbSchemaUpdater* const updater)
{
    Q_D(BdEngineBackend);

    if (d->status == OpenSchemaChecked)
    {
        return true;
    }

    if (d->status == Unavailable)
    {
        return false;
    }

    if (updater->update())
    {
        d->status = OpenSchemaChecked;
        return true;
    }

    return false;
}

} // namespace Digikam

#include "moc_similaritydbbackend.cpp"
