/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-17
 * Description : Color profile conversion tool.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "batchtool.h"
#include "iccprofilesettings.h"

using namespace Digikam;

namespace DigikamBqmIccConvertPlugin
{

class IccConvert : public BatchTool
{
    Q_OBJECT

public:

    explicit IccConvert(QObject* const parent = nullptr);
    ~IccConvert()                                           override = default;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    IccProfilesSettings* m_settingsView = nullptr;
};

} // namespace DigikamBqmIccConvertPlugin
