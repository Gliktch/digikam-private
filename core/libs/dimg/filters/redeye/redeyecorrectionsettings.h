/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-09
 * Description : Red Eyes auto correction settings view.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2016      by Omar Amin <Omar dot moh dot amin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include <QWidget>

// Local includes

#include "digikam_export.h"
#include "redeyecorrectionfilter.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT RedEyeCorrectionSettings : public QWidget
{
    Q_OBJECT

public:

    explicit RedEyeCorrectionSettings(QWidget* const parent = nullptr);
    ~RedEyeCorrectionSettings()                       override;

    RedEyeCorrectionContainer defaultSettings() const;
    void resetToDefault();

    RedEyeCorrectionContainer settings()        const;
    void setSettings(const RedEyeCorrectionContainer& settings);

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

Q_SIGNALS:

    void signalSettingsChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
