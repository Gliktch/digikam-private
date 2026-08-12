/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-09-23
 * Description : assign labels metadata batch tool.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "batchtool.h"

using namespace Digikam;

namespace DigikamBqmAssignLabelsPlugin
{

class AssignLabels : public BatchTool
{
    Q_OBJECT

public:

    explicit AssignLabels(QObject* const parent = nullptr);
    ~AssignLabels()                                         override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    bool toolOperations()                                   override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamBqmAssignLabelsPlugin
