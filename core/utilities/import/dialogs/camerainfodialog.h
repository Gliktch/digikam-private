/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-01-28
 * Description : a dialog to display camera information.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QDialog>

namespace Digikam
{

class CameraInfoDialog : public QDialog
{
    Q_OBJECT

public:

    CameraInfoDialog(QWidget* const parent,
                     const QString& summary,
                     const QString& manual,
                     const QString& about);
    ~CameraInfoDialog() override = default;

private Q_SLOTS:

    void slotHelp();
};

} // namespace Digikam
