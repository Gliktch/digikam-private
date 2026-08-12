/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-14
 * Description : a curves widget with additional control elements
 *
 * SPDX-FileCopyrightText: 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "digikam_export.h"
#include "digikam_globals.h"

class KConfigGroup;

namespace Digikam
{

class ImageCurves;
class DColor;
class DImg;

class DIGIKAM_EXPORT CurvesBox : public QWidget
{
    Q_OBJECT

public:

    enum ColorPicker
    {
        NoPicker      = -1,
        BlackTonal    = 0,
        GrayTonal,
        WhiteTonal
    };

    enum CurvesDrawingType
    {
        SmoothDrawing = 0,
        FreeDrawing
    };

public:

    CurvesBox(int w, int h,                            // Widget size.
              QWidget* const parent = nullptr,         // Parent widget instance.
              bool readOnly=false);                    // If true : widget with full edition mode capabilities.
                                                       // If false : display curve data only without edition.

    CurvesBox(int w, int h,                            // Widget size.
              const DImg& img,                         // Image data.
              QWidget* const parent = nullptr,         // Parent widget instance.
              bool readOnly=false);                    // If true : widget with full edition mode capabilities.
                                                       // If false : display curve data only without edition.
    ~CurvesBox() override;

    void enablePickers(bool enable);
    void enableHGradient(bool enable);
    void enableVGradient(bool enable);
    void enableGradients(bool enable);
    void enableResetButton(bool enable);
    void enableCurveTypes(bool enable);
    void enableControlWidgets(bool enable);

    void setCurveGuide(const DColor& color);

    void resetPickers();
    void resetChannel(int channel);
    void resetChannels();
    void reset();

    void readCurveSettings(KConfigGroup& group, const QString& prefix);
    void writeCurveSettings(KConfigGroup& group, const QString& prefix);

    int  picker()          const;
    ChannelType channel()  const;
    int curvesLeftOffset() const;
    ImageCurves* curves()  const;

Q_SIGNALS:

    void signalPickerChanged(int);
    void signalCurvesChanged();
    void signalChannelReset(int);
    void signalCurveTypeChanged(int);

public Q_SLOTS:

    void setChannel(ChannelType channel);
    void setScale(HistogramScale scale);

private Q_SLOTS:

    void slotCurveTypeChanged(int type);
    void slotResetChannel();
    void slotResetChannels();

private:

    void setup();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
