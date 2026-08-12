/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-08-13
 * Description : batch tool to add texture.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "batchtool.h"
#include "texturesettings.h"

using namespace Digikam;

namespace DigikamBqmTexturePlugin
{

class Texture : public BatchTool
{
    Q_OBJECT

public:

    explicit Texture(QObject* const parent = nullptr);
    ~Texture()                                              override = default;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private Q_SLOTS:

    void slotSettingsChanged()                              override;
    void slotAssignSettings2Widget()                        override;

private:

    bool toolOperations()                                   override;

private:

    TextureSettings* m_settingsView = nullptr;
};

} // namespace DigikamBqmTexturePlugin
