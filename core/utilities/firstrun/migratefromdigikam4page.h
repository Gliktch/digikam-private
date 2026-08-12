/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2016-09-29
 * Description : migration page from digikam4
 *
 * SPDX-FileCopyrightText: 2016 by Antonio Larrosa <alarrosa at suse dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dwizardpage.h"

namespace Digikam
{

class MigrateFromDigikam4Page : public DWizardPage
{
    Q_OBJECT

public:

    explicit MigrateFromDigikam4Page(QWizard* const dlg);
    ~MigrateFromDigikam4Page()      override;

    /**
     * Returns true if the user selected to do a migration
     */
    bool isMigrationChecked() const;
    void doMigration();
    int  nextId()             const override;

    /**
     * Return true if migration data are available on the system
     */
    static bool checkForMigration();

public Q_SLOTS:

    void migrationToggled(bool b);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
