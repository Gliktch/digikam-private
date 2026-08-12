/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a template to create wizard dialog.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWizard>
#include <QString>

// Local includes

#include "dplugin.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DWizardDlg : public QWizard
{
    Q_OBJECT

public:

    explicit DWizardDlg(QWidget* const parent, const QString& objName);
    ~DWizardDlg()                   override;

    void setPlugin(DPlugin* const tool);

protected:

    void restoreDialogSize();
    void saveDialogSize();

private Q_SLOTS:

    void slotAboutPlugin();
    void slotOnlineHandbook();

protected:

    void showEvent(QShowEvent*)     override;

private:

    DPlugin* m_tool = nullptr;

    Q_DISABLE_COPY(DWizardDlg)
};

} // namespace Digikam
