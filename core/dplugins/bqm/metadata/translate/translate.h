/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-30
 * Description : translate metadata batch tool.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "batchtool.h"
#include "dmetadata.h"

using namespace Digikam;

namespace DigikamBqmTranslatePlugin
{

class Translate : public BatchTool
{
    Q_OBJECT

public:

    explicit Translate(QObject* const parent = nullptr);
    ~Translate()                                            override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

    void slotLocalizeChanged();

private:

    bool toolOperations()                                   override;

    bool insertTranslation(int entry,
                           const QString& trLang,
                           DMetadata* const meta)                           const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamBqmTranslatePlugin
