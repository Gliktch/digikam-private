/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * SPDX-FileCopyrightText: 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "wstooldialog.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericRajcePlugin
{

class RajceWidget;

class RajceWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit RajceWindow(DInfoInterface* const iface, QWidget* const parent = nullptr);
    ~RajceWindow() override;

    void reactivate();

protected:

    void closeEvent(QCloseEvent* e) override;

private Q_SLOTS:

    void slotSetUploadButtonEnabled(bool);
    void slotFinished();

private:

    RajceWidget* m_widget = nullptr;
};

} // namespace DigikamGenericRajcePlugin
