/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to create calendar.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "dwizardpage.h"

using namespace Digikam;

namespace DigikamGenericCalendarPlugin
{

class CalIntroPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit CalIntroPage(QWizard* const dialog, const QString& title);
    ~CalIntroPage() override = default;
};

} // namespace DigikamGenericCalendarPlugin
