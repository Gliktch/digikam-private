/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-10-08
 * Description : confirmation of changing the face recognition model
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QDialog>
#include <QDialogButtonBox>

// Local includes

#include "facescansettings.h"

namespace Digikam
{

class FaceTrainingUpgradeDlg : public QDialog
{
    Q_OBJECT

public:

    explicit FaceTrainingUpgradeDlg(QWidget* const parent);

    ~FaceTrainingUpgradeDlg()       override;

private:

private Q_SLOTS:

    void slotStart();
    void slotCancel();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
