/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a template to create wizard page.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWizardPage>
#include <QString>
#include <QPixmap>

// Local includes

#include "digikam_export.h"

class QWizard;

namespace Digikam
{

class DIGIKAM_EXPORT DWizardPage : public QWizardPage
{
    Q_OBJECT

public:

    explicit DWizardPage(QWizard* const dlg, const QString& title);
    ~DWizardPage()                override;

public:

    void setPageWidget(QWidget* const w);
    void removePageWidget(QWidget* const w);
    void setLeftBottomPix(const QPixmap& pix);
    void setLeftBottomPix(const QIcon& icon);
    void setShowLeftView(bool v);

    void setComplete(bool b);
    bool isComplete()       const override;

    int  id()               const;

    QWizard* assistant()    const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
