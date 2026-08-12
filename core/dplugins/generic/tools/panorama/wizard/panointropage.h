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

// Qt includes

#include <QAbstractButton>

// Local includes

#include "dwizardpage.h"
#include "panomanager.h"

using namespace Digikam;

namespace DigikamGenericPanoramaPlugin
{

class PanoManager;

class PanoIntroPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit PanoIntroPage(PanoManager* const mngr, QWizard* const dlg);
    ~PanoIntroPage()        override;

    bool binariesFound();

private Q_SLOTS:

    void slotToggleGPano(int state);
    void slotChangeFileFormat(QAbstractButton* button);
    void slotBinariesChanged(bool found);
/*
    // TODO HDR
    void slotShowFileFormat(int state);
*/
private:

    void initializePage()   override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPanoramaPlugin
