/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-30
 * Description : maintenance dialog
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2025      by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "maintenancesettings.h"

namespace Digikam
{

class MaintenanceDlg : public QDialog
{
    Q_OBJECT

public:

    explicit MaintenanceDlg(QWidget* const parent = nullptr);
    ~MaintenanceDlg() override;

    MaintenanceSettings settings() const;

private Q_SLOTS:

    void slotItemToggled(int index, bool b);
    void slotUseLastSettings(bool checked);
    void slotMetadataSetup();
    void slotOk();
    void slotHelp();

private:

    void writeSettings();
    void readSettings();

protected:

    void showEvent(QShowEvent*) override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
