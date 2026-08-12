/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-02-17
 * Description : a tool to change image perspective.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"

class QRect;

using namespace Digikam;

namespace DigikamEditorPerspectiveToolPlugin
{

class PerspectiveTool : public EditorTool
{
    Q_OBJECT

public:

    explicit PerspectiveTool(QObject* const parent);
    ~PerspectiveTool()                          override;

private Q_SLOTS:

    void slotInverseTransformationChanged(bool b);
    void slotResetSettings()                    override;
    void slotUpdateInfo(const QRect& newSize,
                        float topLeftAngle,
                        float topRightAngle,
                        float bottomLeftAngle,
                        float bottomRightAngle,
                        bool valid);
    void slotColorGuideChanged();

private:

    void readSettings()                         override;
    void writeSettings()                        override;
    void finalRendering()                       override;
    void setBackgroundColor(const QColor& bg)   override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorPerspectiveToolPlugin
