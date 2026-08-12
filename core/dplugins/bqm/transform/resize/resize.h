/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-17
 * Description : resize image batch tool.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "batchtool.h"

using namespace Digikam;

namespace DigikamBqmResizePlugin
{

class Resize : public BatchTool
{
    Q_OBJECT

public:

    explicit Resize(QObject* const parent = nullptr);
    ~Resize()                                               override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;
    void slotPercentChanged();

private:

    bool toolOperations()                                   override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamBqmResizePlugin
