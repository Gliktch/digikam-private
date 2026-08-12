/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-09
 * Description : Local Contrast settings view.
 *               LDR ToneMapper zynaddsubfx.sourceforge.net/other/tonemapping
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
#include "localcontrastcontainer.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT LocalContrastSettings : public QWidget
{
    Q_OBJECT

public:

    explicit LocalContrastSettings(QWidget* const parent);
    ~LocalContrastSettings()                          override;

    LocalContrastContainer defaultSettings()    const;
    void resetToDefault();

    LocalContrastContainer settings()           const;
    void setSettings(const LocalContrastContainer& settings);

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void loadSettings();
    void saveAsSettings();

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:

    void slotStageEnabled(int, bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
