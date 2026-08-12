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

using namespace Digikam;

namespace DigikamGenericExpoBlendingPlugin
{
class ExpoBlendingManager;

class ExpoBlendingLastPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit ExpoBlendingLastPage(ExpoBlendingManager* const mngr, QWizard* const dlg);
    ~ExpoBlendingLastPage() override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericExpoBlendingPlugin
