/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QWidget>

// Local includes

#include "dwizarddlg.h"
#include "expoblendingactions.h"

using namespace Digikam;

namespace DigikamGenericExpoBlendingPlugin
{
class ExpoBlendingManager;

class ExpoBlendingWizard : public DWizardDlg
{
    Q_OBJECT

public:

    explicit ExpoBlendingWizard(ExpoBlendingManager* const mngr, QWidget* const parent = nullptr);
    ~ExpoBlendingWizard()                override;

    QList<QUrl> itemUrls()         const;

    ExpoBlendingManager* manager() const;

    bool validateCurrentPage()           override;

private Q_SLOTS:

    void slotCurrentIdChanged(int);
    void slotExpoBlendingIntroPageIsValid(bool);
    void slotItemsPageIsValid(bool);
    void slotPreProcessed(const ExpoBlendingItemUrlsMap&);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericExpoBlendingPlugin
