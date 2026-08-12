/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-11
 * Description : Hue/Saturation/Lightness batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "batchtool.h"
#include "hslsettings.h"

using namespace Digikam;

namespace DigikamBqmHSLCorrectionPlugin
{

class HSLCorrection : public BatchTool
{
    Q_OBJECT

public:

    explicit HSLCorrection(QObject* const parent = nullptr);
    ~HSLCorrection()                                        override = default;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    HSLSettings* m_settingsView = nullptr;
};

} // namespace DigikamBqmHSLCorrectionPlugin
