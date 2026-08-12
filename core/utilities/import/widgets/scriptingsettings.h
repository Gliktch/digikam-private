/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-07-20
 * Description : scripting settings for camera interface.
 *
 * SPDX-FileCopyrightText: 2012 by Petri Damstén <damu@iki.fi>
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
#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_GUI_EXPORT ScriptingSettings : public QWidget
{
    Q_OBJECT

public:

    explicit ScriptingSettings(QWidget* const parent = nullptr);
    ~ScriptingSettings() override;

    void readSettings(const KConfigGroup& group);
    void saveSettings(KConfigGroup& group);

    void settings(DownloadSettings* const settings) const;

private Q_SLOTS:

    void slotToolTipButtonToggled(bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
