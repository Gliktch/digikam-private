/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-09-30
 * Description : a tool to add rain drop over an image
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEditorRainDropToolPlugin
{

class RainDropTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit RainDropTool(QObject* const parent);
    ~RainDropTool()             override;

private Q_SLOTS:

    void slotResetSettings()    override;

private:

    void readSettings()         override;
    void writeSettings()        override;
    void preparePreview()       override;
    void prepareFinal()         override;
    void setPreviewImage()      override;
    void setFinalImage()        override;

    void blockWidgetSignals(bool b);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorRainDropToolPlugin
