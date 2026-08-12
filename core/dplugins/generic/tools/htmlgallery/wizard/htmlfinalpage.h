/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

namespace DigikamGenericHtmlGalleryPlugin
{

class HTMLFinalPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit HTMLFinalPage(QWizard* const dialog, const QString& title);
    ~HTMLFinalPage()        override;

    void initializePage()   override;
    void cleanupPage()      override;
    bool isComplete() const override;

Q_SIGNALS:

    void signalCancel();

private Q_SLOTS:

    void slotProcess();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericHtmlGalleryPlugin
