/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : a digiKam image tool to reduce
 *               vignetting on an image.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEditorAntivignettingToolPlugin
{

class AntiVignettingTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit AntiVignettingTool(QObject* const parent);
    ~AntiVignettingTool()       override;

private Q_SLOTS:

    void slotResetSettings()    override;

private:

    void writeSettings()        override;
    void readSettings()         override;
    void preparePreview()       override;
    void prepareFinal()         override;
    void setPreviewImage()      override;
    void setFinalImage()        override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorAntivignettingToolPlugin
