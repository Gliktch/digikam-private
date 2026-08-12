/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-07-11
 * Description : digiKam image editor Color Balance tool.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include <QObject>

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEditorColorBalanceToolPlugin
{

class CBTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit CBTool(QObject* const parent);
    ~CBTool()                   override;

private Q_SLOTS:

    void slotResetSettings()    override;

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

} // namespace DigikamEditorColorBalanceToolPlugin
