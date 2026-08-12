/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-07-18
 * Description : DNG Raw Converter batch tool.
 *
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dngsettings.h"
#include "dngwriter.h"
#include "batchtool.h"

using namespace Digikam;

namespace DigikamBqmConvertToDngPlugin
{

class ConvertToDNG : public BatchTool
{
    Q_OBJECT

public:

    explicit ConvertToDNG(QObject* const parent = nullptr);
    ~ConvertToDNG()                                         override = default;

    void cancel()                                           override;
    QString outputSuffix()                            const override;
    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private Q_SLOTS:

    void slotSettingsChanged()                              override;
    void slotAssignSettings2Widget()                        override;
    void slotSetupExifTool();

private:

    bool toolOperations()                                   override;

private:

    DNGWriter m_dngProcessor;
    bool      m_changeSettings  = true;
};

} // namespace DigikamBqmConvertToDngPlugin
