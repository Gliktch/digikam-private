/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : Image Quality settings widget
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTabWidget>

// Local includes

#include "digikam_export.h"
#include "imagequalitysettings.h"
#include "statesavingobject.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_GUI_EXPORT ImageQualityWidget : public QTabWidget,
                                              public StateSavingObject
{
    Q_OBJECT

public:

    enum SettingsDisplayMode
    {
        Normal,
        Maintenance,
        BQM
    };
    Q_ENUM(SettingsDisplayMode)

public:

    explicit ImageQualityWidget(SettingsDisplayMode displayMode,
                                QWidget* const parent = nullptr);
    ~ImageQualityWidget()                                 override;

    void applySettings();
    void applySettings(KConfigGroup&);

    void readSettings();
    void readSettings(const KConfigGroup&);

    ImageQualitySettings settings() const;
    void setSettings(const ImageQualitySettings& imq);

    ImageQualitySettings defaultSettings()          const;
    void resetToDefault();

Q_SIGNALS:

    void signalSettingsChanged();

protected:

    void doLoadState()                                    override;
    void doSaveState()                                    override;

private:

    void setupUi();

private Q_SLOTS:

    void slotDisableOptionViews();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
