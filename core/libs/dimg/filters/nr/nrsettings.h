/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-22
 * Description : noise reduction settings view.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include <QWidget>

// Local includes

#include "digikam_export.h"
#include "nrfilter.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT NRSettings : public QWidget
{
    Q_OBJECT

public:

    explicit NRSettings(QWidget* const parent);
    ~NRSettings() override;

    NRContainer defaultSettings()   const;
    void resetToDefault();

    NRContainer settings()          const;
    void setSettings(const NRContainer& settings);

    void setEstimateNoise(bool b);
    bool estimateNoise()            const;

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void loadSettings();
    void saveAsSettings();

Q_SIGNALS:

    void signalSettingsChanged();
    void signalEstimateNoise();

private Q_SLOTS:

    void slotDisableParameters(bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
