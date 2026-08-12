/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dwizardpage.h"
#include "expoblendingactions.h"

using namespace Digikam;

namespace DigikamGenericExpoBlendingPlugin
{

class ExpoBlendingManager;

class ItemsPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit ItemsPage(ExpoBlendingManager* const mngr, QWizard* const dlg);
    ~ItemsPage() override;

    QList<QUrl> itemUrls() const;

Q_SIGNALS:

    void signalItemsPageIsValid(bool);

private:

    void setIdentity(const QUrl& url, const QString& identity);

private Q_SLOTS:

    void slotSetupList();
    void slotImageListChanged();
    void slotAddItems(const QList<QUrl>&);
    void slotExpoBlendingAction(const DigikamGenericExpoBlendingPlugin::ExpoBlendingActionData&);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericExpoBlendingPlugin
