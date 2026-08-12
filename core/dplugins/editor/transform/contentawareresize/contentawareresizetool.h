/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-01
 * Description : Content aware resizing tool.
 *
 * SPDX-FileCopyrightText: 2009      by Julien Pontabry <julien dot pontabry at ulp dot u-strasbg dot fr>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"
#include "dimg.h"

using namespace Digikam;

namespace DigikamEditorContentAwareResizeToolPlugin
{

class ContentAwareResizeTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit ContentAwareResizeTool(QObject* const parent);
    ~ContentAwareResizeTool()           override;

private:

    void writeSettings()                override;
    void readSettings()                 override;
    void preparePreview()               override;
    void prepareFinal()                 override;
    void setPreviewImage()              override;
    void setFinalImage()                override;
    void renderingFinished()            override;
    void blockWidgetSignals(bool b);
    void disableSettings();
    void contentAwareResizeCore(DImg* const image,
                                int target_width,
                                int target_height,
                                const QImage& mask);
    void enableMaskSettings(bool b);
    void enableContentAwareSettings(bool b);
    bool eventFilter(QObject*, QEvent*) override;

private Q_SLOTS:

    void slotResetSettings() override;
    void slotValuesChanged();
    void slotMixedRescaleValueChanged();
    void slotMaskColorChanged(int);
    void slotWeightMaskBoxStateChanged(int);
    void slotMaskPenSizeChanged(int);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorContentAwareResizeToolPlugin
