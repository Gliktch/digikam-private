/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-18
 * Description : Channel mixer settings view.
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
#include "mixerfilter.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT MixerSettings : public QWidget
{
    Q_OBJECT

public:

    explicit MixerSettings(QWidget* const parent);
    ~MixerSettings() override;

    MixerContainer defaultSettings() const;
    void resetToDefault();

    MixerContainer settings()        const;
    void setSettings(const MixerContainer& settings);

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void loadSettings();
    void saveAsSettings();

    int  currentChannel() const;

Q_SIGNALS:

    void signalSettingsChanged();
    void signalMonochromeActived(bool);
    void signalOutChannelChanged();

private:

    void updateSettingsWidgets();
    void updateTotalPercents();

private Q_SLOTS:

    void slotResetCurrentChannel();
    void slotGainsChanged();
    void slotMonochromeActived(bool);
    void slotLuminosityChanged(bool);
    void slotOutChannelChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
