/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to print images
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QUrl>
#include <QString>

// Local includes

#include "dwizardpage.h"

using namespace Digikam;

namespace DigikamGenericPrintCreatorPlugin
{

class AdvPrintAlbumsPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit AdvPrintAlbumsPage(QWizard* const dialog, const QString& title);
    ~AdvPrintAlbumsPage()   override;

    bool validatePage()     override;
    bool isComplete() const override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPrintCreatorPlugin
