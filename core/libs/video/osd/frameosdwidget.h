/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-24
 * Description : frame on screen display settings widget
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Quoc Hưng Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include <QWidget>

// Local includes

#include "digikam_export.h"
#include "frameosdsettings.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT FrameOsdWidget : public QWidget
{
    Q_OBJECT

public:

    explicit FrameOsdWidget(QWidget* const parent);
    ~FrameOsdWidget()                          override;

    FrameOsdSettings settings()           const;
    void setSettings(const FrameOsdSettings& settings);

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:

    void slotSetUnchecked(int);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
