/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-12-23
 * Description : a tool to shear an image
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEditorShearToolPlugin
{

class ShearTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit ShearTool(QObject* const parent);
    ~ShearTool()                override;

private Q_SLOTS:

    void slotResetSettings()    override;
    void slotColorGuideChanged();

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

} // namespace DigikamEditorShearToolPlugin
