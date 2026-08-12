/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-05-16
 * Description : dialog to set time stamp of picture files.
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2005 by Jesper Pedersen <blackie at kde dot org>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018-2021 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>

// Local includes

#include "dplugindialog.h"
#include "timeadjustsettings.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericTimeAdjustPlugin
{

class TimeAdjustDialog : public DPluginDialog
{
    Q_OBJECT

public:

    explicit TimeAdjustDialog(QWidget* const parent, DInfoInterface* const iface);
    ~TimeAdjustDialog()             override;

private Q_SLOTS:

    void setBusy(bool);
    void slotCancelThread();
    void slotThreadFinished();
    void slotDialogFinished();
    void slotProcessStarted(const QUrl&);
    void slotPreviewReady(const QUrl&, const QDateTime&, const QDateTime&);
    void slotProcessEnded(const QUrl&, const QDateTime&, const QDateTime&, int);

    void slotUpdateTimestamps();
    void slotOkExitTimestamps();

    void slotPreviewTimer();
    void slotUpdateTimer();

private:

    void readSettings();
    void saveSettings();

protected:

    void closeEvent(QCloseEvent*)   override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericTimeAdjustPlugin
