/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-08-24
 * Description : image editor plugin to blur the background of an image
 *
 * SPDX-FileCopyrightText: 2025-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEditorBackgroundBlurToolPlugin
{

class BackgroundBlurTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit BackgroundBlurTool(QObject* const parent);
    ~BackgroundBlurTool()                 override;

private Q_SLOTS:

    void slotResetSettings()    override;
    void slotPreviewMask(const QImage& mask);
    void slotSelectionChanged(const QRectF&);

private:

    void readSettings()         override;
    void writeSettings()        override;
    void preparePreview()       override;
    void prepareFinal()         override;
    void setPreviewImage()      override;
    void setFinalImage()        override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorBackgroundBlurToolPlugin
