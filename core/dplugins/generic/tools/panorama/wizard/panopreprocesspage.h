/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 * Acknowledge : based on the expoblending tool
 *
 * SPDX-FileCopyrightText: 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dwizardpage.h"
#include "panoactions.h"

using namespace Digikam;

namespace DigikamGenericPanoramaPlugin
{

class PanoManager;

class PanoPreProcessPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit PanoPreProcessPage(PanoManager* const mngr, QWizard* const dlg);
    ~PanoPreProcessPage() override;

private:

    void process();
    void initializePage() override;
    bool validatePage()   override;
    void cleanupPage()    override;

Q_SIGNALS:

    void signalPreProcessed();

private Q_SLOTS:

    void slotProgressTimerDone();
    void slotPanoAction(const DigikamGenericPanoramaPlugin::PanoActionData&);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPanoramaPlugin
