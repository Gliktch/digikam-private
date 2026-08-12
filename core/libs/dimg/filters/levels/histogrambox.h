/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-30
 * Description : a widget to display an image histogram and its control widgets
 *
 * SPDX-FileCopyrightText: 2008-2009 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "digikam_debug.h"
#include "digikam_export.h"
#include "digikam_globals.h"

class QColor;

namespace Digikam
{

class HistogramWidget;

class DIGIKAM_EXPORT HistogramBox : public QWidget
{
    Q_OBJECT

public:

    explicit HistogramBox(QWidget* const parent = nullptr,
                          HistogramBoxType type = Digikam::LRGB,
                          bool selectMode = false);
    ~HistogramBox() override;

    void setHistogramType(HistogramBoxType type);
    void setHistogramMargin(int);

    void setGradientColors(const QColor& from, const QColor& to);
    void setGradientVisible(bool visible);

    ChannelType channel()           const;
    void setChannelEnabled(bool enabled);

    void setStatisticsVisible(bool b);

    HistogramScale scale()          const;

    HistogramWidget* histogram()    const;

Q_SIGNALS:

    void signalChannelChanged(ChannelType channel);
    void signalScaleChanged(HistogramScale scale);

public Q_SLOTS:

    void setChannel(ChannelType channel);
    void setScale(HistogramScale scale);

protected Q_SLOTS:

    void slotChannelChanged();
    void slotScaleChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
