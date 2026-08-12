/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-11
 * Description : Color Balance settings view.
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
#include "cbfilter.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT CBSettings : public QWidget
{
    Q_OBJECT

public:

    explicit CBSettings(QWidget* const parent);
    ~CBSettings() override;

    CBContainer defaultSettings() const;
    void resetToDefault();

    CBContainer settings() const;
    void setSettings(const CBContainer& settings);

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

Q_SIGNALS:

    void signalSettingsChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
