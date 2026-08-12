/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QWidget>
#include <QWizard>

// Local includes

#include "digikam_export.h"
#include "dbengineparameters.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT FirstRunDlg : public QWizard
{
    Q_OBJECT

public:

    explicit FirstRunDlg(QWidget* const parent = nullptr);
    ~FirstRunDlg() override;

    QString firstAlbumPath() const;
    DbEngineParameters getDbEngineParameters() const;

private:

    bool validateCurrentPage() override;

private Q_SLOTS:

    void slotFinishPressed();
    void slotHelp();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
