/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-26
 * Description : White Balance settings view.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include <QWidget>

// Local includes

#include "digikam_export.h"
#include "wbfilter.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT WBSettings : public QWidget
{
    Q_OBJECT

public:

    explicit WBSettings(QWidget* const parent);
    ~WBSettings() override;

    WBContainer defaultSettings() const;
    void resetToDefault();

    WBContainer settings()        const;
    void setSettings(const WBContainer& settings);

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void loadSettings();
    void saveAsSettings();

    bool pickTemperatureIsChecked();
    void setCheckedPickTemperature(bool b);

    void showAdvancedButtons(bool b);

Q_SIGNALS:

    void signalSettingsChanged();
    void signalPickerColorButtonActived();
    void signalAutoAdjustExposure();

private Q_SLOTS:

    void slotTemperatureChanged(double temperature);
    void slotTemperaturePresetChanged(int tempPreset);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
