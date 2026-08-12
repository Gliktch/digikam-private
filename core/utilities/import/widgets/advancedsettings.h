/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-08-12
 * Description : advanced settings for camera interface.
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QDateTime>

// Local settings

#include "downloadsettings.h"

class KConfigGroup;

namespace Digikam
{

class AdvancedSettings : public QWidget
{
    Q_OBJECT

public:

    explicit AdvancedSettings(QWidget* const parent = nullptr);
    ~AdvancedSettings() override;

    void readSettings(const KConfigGroup& group);
    void saveSettings(KConfigGroup& group);

    DownloadSettings settings() const;

Q_SIGNALS:

    void signalDownloadNameChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
