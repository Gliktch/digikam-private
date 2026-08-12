/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-09-24
 * Description : assign captions metadata batch tool.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "batchtool.h"
#include "captionvalues.h"

using namespace Digikam;

namespace DigikamBqmAssignCaptionsPlugin
{

class AssignCaptions : public BatchTool
{
    Q_OBJECT

public:

    explicit AssignCaptions(QObject* const parent = nullptr);
    ~AssignCaptions()                                                     override;

    BatchToolSettings defaultSettings()                                   override;

    BatchTool* clone(QObject* const parent = nullptr)               const override;

    void registerSettingsWidget()                                         override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                                      override;
    void slotSettingsChanged()                                            override;

private:

    QString captionsMapToString(const CaptionsMap& cmap)            const;
    CaptionsMap captionsMapFromString(const QString& str)           const;

    QString altLangMapToString(const MetaEngine::AltLangMap& amap)  const;
    MetaEngine::AltLangMap altLangMapFromString(const QString& str) const;

    bool toolOperations()                                                 override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamBqmAssignCaptionsPlugin
