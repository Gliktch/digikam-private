/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-05-16
 * Description : time adjust settings widget.
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

// Local includes

#include "digikam_export.h"
#include "timeadjustcontainer.h"

namespace Digikam
{

class TimeAdjustContainer;

class DIGIKAM_EXPORT TimeAdjustSettings : public QScrollArea
{
    Q_OBJECT

public:

    explicit TimeAdjustSettings(QWidget* const parent, bool timeAdjustTool = false);
    ~TimeAdjustSettings() override;

public:

    void setSettings(const TimeAdjustContainer& settings);
    TimeAdjustContainer settings() const;
    void detAdjustmentByClockPhotoUrl(const QUrl& url);
    void setCurrentItemUrl(const QUrl& url);

Q_SIGNALS:

    void signalSettingsChanged();
    void signalSrcTimestampChanged();

private Q_SLOTS:

    void slotUseExifToolChanged();
    void slotSrcTimestampChanged();
    void slotResetDateToCurrent();
    void slotAdjustmentTypeChanged();
    void slotDetAdjustmentByClockPhotoDialog();
    void slotDetAdjustmentByClockPhotoUrl(const QUrl& url);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
