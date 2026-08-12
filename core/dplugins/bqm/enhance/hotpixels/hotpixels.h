/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-08-07
 * Description : Hot Pixels Fixer batch tool.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "batchtool.h"
#include "hotpixelsettings.h"

using namespace Digikam;

namespace DigikamBqmHotPixelsPlugin
{

class HotPixels : public BatchTool
{
    Q_OBJECT

public:

    explicit HotPixels(QObject* const parent = nullptr);
    ~HotPixels()                                            override = default;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    HotPixelSettings* m_settingsView = nullptr;
};

} // namespace DigikamBqmHotPixelsPlugin
