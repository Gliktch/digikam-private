/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-09
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2008-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dplugindialog.h"

using namespace Digikam;

namespace DigikamGenericPresentationPlugin
{

class PresentationContainer;

class PresentationDlg : public DPluginDialog
{
    Q_OBJECT

public:

    explicit PresentationDlg(QWidget* const parent,
                             PresentationContainer* const sharedData);
    ~PresentationDlg()              override;

Q_SIGNALS:

    void buttonStartClicked();

private:

    void readSettings();
    void saveSettings();
    void closeEvent(QCloseEvent* e) override;

private Q_SLOTS:

    void slotStartClicked();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPresentationPlugin
