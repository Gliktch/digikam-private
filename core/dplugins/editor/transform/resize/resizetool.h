/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-07
 * Description : a tool to resize an image
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEditorResizeToolPlugin
{

class ResizeTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit ResizeTool(QObject* const parent);
    ~ResizeTool()               override;

private:

    void writeSettings()        override;
    void readSettings()         override;
    void preparePreview()       override;
    void prepareFinal()         override;
    void setPreviewImage()      override;
    void setFinalImage()        override;
    void renderingFinished()    override;
    void blockWidgetSignals(bool b);

    double pixelsToUnits(int pix);
    int    unitsToPixels(double val);

private Q_SLOTS:

    void slotSaveAsSettings()   override;
    void slotLoadSettings()     override;
    void slotResetSettings()    override;             // cppcheck-suppress virtualCallInConstructor
    void slotValuesChanged();
    void slotUnitsChanged();
    void slotPresetsChanged();
    void slotRestorationToggled(bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorResizeToolPlugin
