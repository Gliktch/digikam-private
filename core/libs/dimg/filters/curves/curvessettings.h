/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-01
 * Description : Curves settings view.
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
#include "curvesfilter.h"
#include "curveswidget.h"
#include "curvesbox.h"
#include "dimg.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT CurvesSettings : public QWidget
{
    Q_OBJECT

public:

    explicit CurvesSettings(QWidget* const parent, DImg* const img);
    ~CurvesSettings() override;

    CurvesContainer defaultSettings() const;
    void resetToDefault();

    CurvesContainer settings()        const;
    void setSettings(const CurvesContainer& settings);

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void loadSettings();
    void saveAsSettings();

    void setScale(HistogramScale type);
    void setCurrentChannel(ChannelType channel);

    int curvesLeftOffset() const;

    void setCurveGuide(const DColor& color);

Q_SIGNALS:

    void signalSettingsChanged();
    void signalSpotColorChanged();
    void signalChannelReset(int);
    void signalPickerChanged(int);

public Q_SLOTS:

    void slotSpotColorChanged(const Digikam::DColor& color);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
