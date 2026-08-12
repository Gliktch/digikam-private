/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-31
 * Description : Auto-Color correction tool.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPixmap>

// Local includes

#include "editortool.h"
#include "dimg.h"

using namespace Digikam;

namespace DigikamEditorAutoCorrectionToolPlugin
{

class AutoCorrectionTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit AutoCorrectionTool(QObject* const parent);
    ~AutoCorrectionTool()       override;

private Q_SLOTS:

    void slotInit() override;
    void slotResetSettings()    override;

private:

    void writeSettings()        override;
    void readSettings()         override;
    void preparePreview()       override;
    void prepareFinal()         override;
    void setPreviewImage()      override;
    void setFinalImage()        override;

    void autoCorrection(DImg* const img, DImg* const ref, int type);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorAutoCorrectionToolPlugin
