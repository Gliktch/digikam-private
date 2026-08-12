/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-11-08
 * Description : a batch tool to apply color effects to images.
 *
 * SPDX-FileCopyrightText: 2012 by Alexander Dymo <adymo at develop dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "batchtool.h"
#include "colorfxsettings.h"

using namespace Digikam;

namespace DigikamBqmColorFXPlugin
{

class ColorFX : public BatchTool
{
    Q_OBJECT

public:

    explicit ColorFX(QObject* const parent = nullptr);
    ~ColorFX()                                              override = default;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    ColorFXSettings* m_settingsView = nullptr;
};

} // namespace DigikamBqmColorFXPlugin
