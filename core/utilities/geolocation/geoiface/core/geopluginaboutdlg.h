/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : Geolocation plugin about dialog
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

#include "PluginInterface.h"
#include "digikam_export.h"

using namespace Marble;

namespace Digikam
{

class DIGIKAM_EXPORT GeoPluginAboutDlg : public QDialog
{
    Q_OBJECT

public:

    explicit GeoPluginAboutDlg(PluginInterface* const tool, QWidget* const parent = nullptr);
    ~GeoPluginAboutDlg() override = default;

private Q_SLOTS:

    void slotOnlineHandbook();

private:

    Q_DISABLE_COPY(GeoPluginAboutDlg)

private:

    PluginInterface* m_tool = nullptr;
};

} // namespace Digikam
