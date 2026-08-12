/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items by email.
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

// Local includes

#include "dwizarddlg.h"
#include "dinfointerface.h"
#include "mailsettings.h"

using namespace Digikam;

namespace DigikamGenericSendByMailPlugin
{

class MailWizard : public DWizardDlg
{
    Q_OBJECT

public:

    explicit MailWizard(QWidget* const parent, DInfoInterface* const iface = nullptr);
    ~MailWizard()                    override;

    bool validateCurrentPage()       override;
    int  nextId()              const override;

    DInfoInterface* iface()    const;
    MailSettings*   settings() const;

    void setItemsList(const QList<QUrl>& urls);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericSendByMailPlugin
