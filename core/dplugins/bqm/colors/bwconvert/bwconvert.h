/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-23
 * Description : Black and White conversion batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "batchtool.h"
#include "bwsepiasettings.h"

using namespace Digikam;

namespace DigikamBqmBWConvertPlugin
{

class BWConvert : public BatchTool
{
    Q_OBJECT

public:

    explicit BWConvert(QObject* const parent = nullptr);
    ~BWConvert()                                            override = default;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

public Q_SLOTS:

    void slotResetSettingsToDefault()                       override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    DImg             m_preview;
    BWSepiaSettings* m_settingsView = nullptr;
};

} // namespace DigikamBqmBWConvertPlugin
