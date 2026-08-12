/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : a tool to enhance image with local contrasts (as human eye does).
 *
 * SPDX-FileCopyrightText: 2009      by Julien Pontabry <julien dot pontabry at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEditorLocalContrastToolPlugin
{

class LocalContrastTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit LocalContrastTool(QObject* const parent);
    ~LocalContrastTool()        override;

private:

    void readSettings()         override;
    void writeSettings()        override;
    void preparePreview()       override;
    void prepareFinal()         override;
    void setPreviewImage()      override;
    void setFinalImage()        override;

private Q_SLOTS:

    void slotSaveAsSettings()   override;
    void slotLoadSettings()     override;
    void slotResetSettings()    override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorLocalContrastToolPlugin
