/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-03-09
 * Description : a BQM plugin to automatically rotate images.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2025      by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "batchtool.h"

using namespace Digikam;

namespace DigikamBqmAutoRotatePlugin
{

class AutoRotate : public BatchTool
{
    Q_OBJECT

public:

    explicit AutoRotate(QObject* const parent = nullptr);
    ~AutoRotate()                                           override = default;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override {};
    void slotSettingsChanged()                              override {};
};

} // namespace DigikamBqmAutoAutoRotatePlugin
