/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-03
 * Description : a batch tool to add film grain to images.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "batchtool.h"
#include "filmgrainsettings.h"

using namespace Digikam;

namespace DigikamBqmFilmGrainPlugin
{

class FilmGrain : public BatchTool
{
    Q_OBJECT

public:

    explicit FilmGrain(QObject* const parent = nullptr);
    ~FilmGrain()                                            override = default;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    FilmGrainSettings* m_settingsView = nullptr;
};

} // namespace DigikamBqmFilmGrainPlugin
