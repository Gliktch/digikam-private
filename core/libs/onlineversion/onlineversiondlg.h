/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-12-21
 * Description : Online version dialog.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "digikam_version.h"

namespace Digikam
{

class DIGIKAM_EXPORT OnlineVersionDlg : public QDialog
{
    Q_OBJECT

public:

    explicit OnlineVersionDlg(QWidget* const parent = nullptr,
                              const QString& version = QLatin1String(digikam_version_short),
                              const QDateTime& buildDt = digiKamBuildDate(),
                              bool checkPreRelease = false,
                              bool updateWithDebug = false);
    ~OnlineVersionDlg() override;

Q_SIGNALS:

    void signalSetupUpdate();

private Q_SLOTS:

    void slotNewVersionAvailable(const QString& version);
    void slotNewVersionCheckError(const QString& error);

    void slotDownloadInstaller();
    void slotDownloadError(const QString& error);
    void slotDownloadProgress(qint64, qint64);

    void slotComputeChecksum();

    void slotRunInstaller();
    void slotOpenInFileManager();

    void slotHelp();
    void slotSetupUpdate();

    void slotUpdateStats();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
