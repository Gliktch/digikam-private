/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-16
 * Description : a dialog to display ICC profile information.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QByteArray>
#include <QString>
#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "icctransform.h"

class QWidget;

namespace Digikam
{

class DIGIKAM_EXPORT ICCProfileInfoDlg : public QDialog
{
    Q_OBJECT

public:

    explicit ICCProfileInfoDlg(QWidget* const parent, const QString& profilePath, const IccProfile& profileData = IccProfile());
    ~ICCProfileInfoDlg() override = default;

private Q_SLOTS:

    void slotHelp();
};

} // namespace Digikam
