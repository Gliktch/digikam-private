/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-04-02
 * Description : plugin to export image as wallpaper - Settings dialog
 *
 * SPDX-FileCopyrightText: 2019      by Igor Antropov <antropovi at yahoo dot com>
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dplugin.h"
#include "dplugindialog.h"

using namespace Digikam;

namespace DigikamGenericWallpaperPlugin
{

class WallpaperPluginDlg : public DPluginDialog
{
    Q_OBJECT

public:

    explicit WallpaperPluginDlg(DPlugin* const plugin,
                                QWidget* const parent = nullptr);
    ~WallpaperPluginDlg()       override;

    int wallpaperLayout() const;

private:

    void showEvent(QShowEvent*) override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericWallpaperPlugin
