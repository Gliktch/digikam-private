/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : digiKam plugin about dialog
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "dplugin.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPluginAboutDlg : public QDialog
{
    Q_OBJECT

public:

    explicit DPluginAboutDlg(DPlugin* const tool, QWidget* const parent = nullptr);
    ~DPluginAboutDlg() override = default;

private Q_SLOTS:

    void slotOnlineHandbook();

private:

    Q_DISABLE_COPY(DPluginAboutDlg)

private:

    DPlugin* m_tool = nullptr;
};

} // namespace Digikam
