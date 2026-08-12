/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : setup tab for slideshow options.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

// Local includes

#include "dplugindialog.h"

using namespace Digikam;

namespace DigikamGenericSlideShowPlugin
{

class SlideShowSettings;

class SetupSlideShowDialog : public DPluginDialog
{
    Q_OBJECT

public:

    explicit SetupSlideShowDialog(SlideShowSettings* const settings,
                                  QWidget* const parent = nullptr);
    ~SetupSlideShowDialog() override;

public Q_SLOTS:

    void slotSetUnchecked(int);
    void slotApplySettings();

private:

    void readSettings();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericSlideShowPlugin
