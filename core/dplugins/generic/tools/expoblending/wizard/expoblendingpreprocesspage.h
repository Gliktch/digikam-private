/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dwizardpage.h"
#include "expoblendingactions.h"

using namespace Digikam;

namespace DigikamGenericExpoBlendingPlugin
{

class ExpoBlendingManager;

class ExpoBlendingPreProcessPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit ExpoBlendingPreProcessPage(ExpoBlendingManager* const mngr, QWizard* const dlg);
    ~ExpoBlendingPreProcessPage() override;

    void process();
    void cancel();

Q_SIGNALS:

    void signalPreProcessed(const ExpoBlendingItemUrlsMap&);

private Q_SLOTS:

    void slotProgressTimerDone();
    void slotExpoBlendingAction(const DigikamGenericExpoBlendingPlugin::ExpoBlendingActionData&);

private:

    void resetTitle();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericExpoBlendingPlugin
