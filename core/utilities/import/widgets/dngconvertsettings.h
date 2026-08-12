/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2016-05-03
 * Description : DNG convert settings for camera interface.
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2016 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local settings

#include "downloadsettings.h"

class KConfigGroup;

namespace Digikam
{

class DNGConvertSettings : public QWidget
{
    Q_OBJECT

public:

    explicit DNGConvertSettings(QWidget* const parent = nullptr);
    ~DNGConvertSettings() override;

    void readSettings(const KConfigGroup& group);
    void saveSettings(KConfigGroup& group);

    void settings(DownloadSettings* const settings);

Q_SIGNALS:

    void signalDownloadNameChanged();

private Q_SLOTS:

    void slotSetupExifTool();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
