/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-01-08
 * Description : database server starter
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2016      by Swati Lodha <swatilodha27 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"
#include "databaseservererror.h"
#include "dbengineparameters.h"

namespace Digikam
{

class DIGIKAM_EXPORT DatabaseServerStarter : public QObject
{
    Q_OBJECT

public:

    DatabaseServerStarter();
    ~DatabaseServerStarter() override;

    /**
     * Global instance of internal server starter. All accessor methods are thread-safe.
     */
    static DatabaseServerStarter* instance();

    DatabaseServerError startServerManagerProcess(const DbEngineParameters& parameters) const;
    void                stopServerManagerProcess();

private:

    /// @note disabled
    explicit DatabaseServerStarter(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
