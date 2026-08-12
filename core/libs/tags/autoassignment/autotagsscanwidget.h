/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-01-06
 * Description : Widget to choose options for autotags scanning
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTabWidget>

// Local includes

#include "digikam_export.h"
#include "statesavingobject.h"
#include "autotagsscansettings.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT AutotagsScanWidget : public QTabWidget,
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

    explicit AutotagsScanWidget(SettingsDisplayMode displayMode,
                                QWidget* const parent = nullptr);
    ~AutotagsScanWidget()                 override;

    AutotagsScanSettings settings() const;
    void setSettings(const AutotagsScanSettings& newSettings);

Q_SIGNALS:

    void signalSettingsChanged();

protected:

    void doLoadState()                    override;
    void doSaveState()                    override;

private Q_SLOTS:

    void slotSettingsChanged();

private:

    void setupUi();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
