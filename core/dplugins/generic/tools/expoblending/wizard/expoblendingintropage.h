/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dwizardpage.h"
#include "expoblendingmanager.h"

using namespace Digikam;

namespace DigikamGenericExpoBlendingPlugin
{

class ExpoBlendingIntroPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit ExpoBlendingIntroPage(ExpoBlendingManager* const mngr, QWizard* const dlg);
    ~ExpoBlendingIntroPage() override;

    bool binariesFound();

Q_SIGNALS:

    void signalExpoBlendingIntroPageIsValid(bool);

private Q_SLOTS:

    void slotExpoBlendingIntroPageIsValid();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericExpoBlendingPlugin
