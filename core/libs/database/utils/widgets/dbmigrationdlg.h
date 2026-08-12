/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// QT includes

#include <QThread>
#include <QProgressBar>
#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "dbsettingswidget.h"
#include "coredbbackend.h"
#include "coredbcopymanager.h"

namespace Digikam
{

class DatabaseCopyThread : public QThread
{
    Q_OBJECT

public:

    explicit DatabaseCopyThread(QWidget* const parent);
    ~DatabaseCopyThread() override;

    void init(const DbEngineParameters& fromDatabaseSettingsWidget,
              const DbEngineParameters& toDatabaseSettingsWidget);
    void run() override;

public:

    CoreDbCopyManager m_copyManager;

private:

    class Private;
    Private* const d = nullptr;
};

// --------------------------------------------------------------------

class DIGIKAM_GUI_EXPORT DatabaseMigrationDialog : public QDialog
{
    Q_OBJECT

public:

    explicit DatabaseMigrationDialog(QWidget* const parent);
    ~DatabaseMigrationDialog() override;

private Q_SLOTS:

    void slotHelp();
    void slotPerformCopy();
    void slotUnlockInputFields();
    void slotLockInputFields();

    void slotHandleFinish(int finishState, const QString& errorMsg);
    void slotHandleStepStarted(const QString& stepName);
    void slotHandleSmallStepStarted(int currValue, int maxValue);

private:

    void setupMainArea();
    void dataInit();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
