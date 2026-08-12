/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-07-20
 * Description : image histogram adjust levels.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"
#include "dcolor.h"

using namespace Digikam;

namespace DigikamEditorAdjustLevelsToolPlugin
{

class AdjustLevelsTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit AdjustLevelsTool(QObject* const parent);
    ~AdjustLevelsTool()                     override;

private Q_SLOTS:

    void slotSaveAsSettings()               override;
    void slotLoadSettings()                 override;
    void slotResetSettings()                override;
    void slotResetCurrentChannel();
    void slotAutoLevels();
    void slotChannelChanged()               override;             // cppcheck-suppress virtualCallInConstructor
    void slotScaleChanged()                 override;
    void slotAdjustSliders();
    void slotGammaInputchanged(double val);
    void slotAdjustMinInputSpinBox(double val);
    void slotAdjustMaxInputSpinBox(double val);
    void slotAdjustMinOutputSpinBox(double val);
    void slotAdjustMaxOutputSpinBox(double val);
    void slotSpotColorChanged(const Digikam::DColor&);
    void slotColorSelectedFromOriginal(const Digikam::DColor&, const QPoint&);
    void slotPickerColorButtonActived(int);
    void slotShowInputHistogramGuide(double v);
    void slotShowOutputHistogramGuide(double v);

private:

    void readSettings()                     override;
    void writeSettings()                    override;
    void preparePreview()                   override;
    void prepareFinal()                     override;
    void abortPreview();
    void setPreviewImage()                  override;
    void setFinalImage()                    override;

    void adjustSliders(int minIn, double gamIn, int maxIn, int minOut, int maxOut);
    void adjustSlidersAndSpinboxes(int minIn, double gamIn, int maxIn, int minOut, int maxOut);
    bool eventFilter(QObject*, QEvent*)     override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorAdjustLevelsToolPlugin
