/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-02-09
 * Description : a tool to apply Blur FX to images
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEditorBlurFxToolPlugin
{

class BlurFXTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit BlurFXTool(QObject* const parent);
    ~BlurFXTool()               override;

private Q_SLOTS:

    void slotEffectTypeChanged(int type);
    void slotResetSettings()    override;

private:

    void readSettings()         override;
    void writeSettings()        override;
    void preparePreview()       override;
    void prepareFinal()         override;
    void setPreviewImage()      override;
    void setFinalImage()        override;
    void renderingFinished()    override;
    void blockWidgetSignals(bool b);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorBlurFxToolPlugin
