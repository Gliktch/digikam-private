/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-02-11
 * Description : apply metadata batch tool with ExifTool.
 *
 * SPDX-FileCopyrightText: 2024 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "batchtool.h"

using namespace Digikam;

namespace DigikamBqmApplyMetadataPlugin
{

class ApplyMetadata : public BatchTool
{
    Q_OBJECT

public:

    explicit ApplyMetadata(QObject* const parent = nullptr);
    ~ApplyMetadata()                                        override;

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

} // namespace DigikamBqmApplyMetadataPlugin
