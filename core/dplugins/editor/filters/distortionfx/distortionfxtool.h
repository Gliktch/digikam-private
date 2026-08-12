/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-02-11
 * Description : a tool to apply Distortion FX to an image.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * Original Distortion algorithms copyrighted 2004-2005 by
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEditorDistortionFxToolPlugin
{

class DistortionFXTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit DistortionFXTool(QObject* const parent);
    ~DistortionFXTool()         override;

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

} // namespace DigikamEditorDistortionFxToolPlugin
