/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-12-06
 * Description : digiKam image editor Ratio Crop tool
 *
 * SPDX-FileCopyrightText: 2007      by Jaromir Malenko <malenko at email dot cz>
 * SPDX-FileCopyrightText: 2008      by Roberto Castagnola <roberto dot castagnola at gmail dot com>
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QRect>

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEditorRatioCropToolPlugin
{

class RatioCropTool : public EditorTool
{
    Q_OBJECT

public:

    explicit RatioCropTool(QObject* const parent);
    ~RatioCropTool()                            override;

private:

    void  readSettings()                        override;
    void  writeSettings()                       override;

    void  finalRendering()                      override;
    void  updateCropInfo();
    void  applyRatioChanges(int a);
    void  blockWidgetSignals(bool b);
    void  setRatioCBText(int orientation);
    void  setInputRange(const QRect& rect);
    void  setBackgroundColor(const QColor& bg)  override;
    QRect getNormalizedRegion() const;

private Q_SLOTS:

    void slotMaxAspectRatio();
    void slotResetSelection();
    void slotResetSettings()                    override;

    void slotCenterWidth();
    void slotCenterHeight();
    void slotXChanged(int x);
    void slotYChanged(int y);
    void slotWidthChanged(int w);
    void slotHeightChanged(int h);
    void slotCustomRatioChanged();
    void slotCustomNRatioChanged(int a);
    void slotCustomDRatioChanged(int a);
    void slotPreciseCropChanged(bool a);
    void slotOrientChanged(int o);
    void slotAutoOrientChanged(bool a);
    void slotRatioChanged(int a);
    void slotSelectionChanged(const QRect&);
    void slotSelectionOrientationChanged(int);
    void slotGuideTypeChanged(int t);
    void slotGoldenGuideTypeChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorRatioCropToolPlugin
