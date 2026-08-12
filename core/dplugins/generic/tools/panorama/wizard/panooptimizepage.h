/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 *
 * SPDX-FileCopyrightText: 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

class PanoOptimizePage : public DWizardPage
{
    Q_OBJECT

public:

    explicit PanoOptimizePage(PanoManager* const mngr, QWizard* const dlg);
    ~PanoOptimizePage() override;

private:

    void process();

    void initializePage()   override;
    bool validatePage()     override;
    void cleanupPage()      override;

Q_SIGNALS:

    void signalOptimized();

private Q_SLOTS:

    void slotProgressTimerDone();
    void slotPanoAction(const DigikamGenericPanoramaPlugin::PanoActionData&);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPanoramaPlugin
