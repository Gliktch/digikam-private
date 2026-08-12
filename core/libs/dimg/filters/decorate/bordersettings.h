/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-17
 * Description : Border settings view.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include <QWidget>
#include <QColor>

// Local includes

#include "digikam_export.h"
#include "borderfilter.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT BorderSettings : public QWidget
{
    Q_OBJECT

public:

    explicit BorderSettings(QWidget* const parent);
    ~BorderSettings() override;

    BorderContainer defaultSettings() const;
    void resetToDefault();

    BorderContainer settings()        const;
    void setSettings(const BorderContainer& settings);

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:

    void slotBorderTypeChanged(int borderType);
    void slotPreserveAspectRatioToggled(bool);
    void slotColorForegroundChanged(const QColor& color);
    void slotColorBackgroundChanged(const QColor& color);

private:

    void toggleBorderSlider(bool b);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
