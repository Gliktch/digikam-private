/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : finish page of export tool, where user can watch upload process.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

namespace DigikamGenericUnifiedPlugin
{

class WSFinalPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit WSFinalPage(QWizard* const dialog, const QString& title);
    ~WSFinalPage();

    void initializePage();
    bool isComplete() const;
    void cleanupPage();

private Q_SLOTS:

    void slotDone();
    void slotProcess();
    void slotMessage(const QString&, bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericUnifiedPlugin
