/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : Media Server configuration dialog to share a single list of files
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>

// Local includes

#include "dinfointerface.h"
#include "dplugindialog.h"

using namespace Digikam;

namespace DigikamGenericMediaServerPlugin
{

class DMediaServerDlg : public DPluginDialog
{
    Q_OBJECT

public:

    explicit DMediaServerDlg(QObject* const parent,
                             DInfoInterface* const iface = nullptr);
    ~DMediaServerDlg()              override;

private:

    void readSettings();
    void saveSettings();
    void updateServerStatus();
    bool setMediaServerContents();
    void startMediaServer();

private Q_SLOTS:

    void accept()                   override;
    void slotToggleMediaServer();
    void slotSelectionChanged();

private:

    /// @note disabled
    explicit DMediaServerDlg(QWidget*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericMediaServerPlugin
