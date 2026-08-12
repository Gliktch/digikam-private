/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-11
 * Description : a digiKam image editor tool to correct
 *               image white balance
 *
 * SPDX-FileCopyrightText: 2008-2009 by Guillaume Castagnino <casta at xwing dot info>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"
#include "dcolor.h"

using namespace Digikam;

namespace DigikamEditorWhiteBalanceToolPlugin
{

class WhiteBalanceTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit WhiteBalanceTool(QObject* const parent);
    ~WhiteBalanceTool()         override;

private Q_SLOTS:

    void slotSaveAsSettings()   override;
    void slotLoadSettings()     override;
    void slotResetSettings()    override;
    void slotPickerColorButtonActived();
    void slotSpotPositionChangedFromOriginal(const Digikam::DColor&, const QPoint&);
    void slotColorSelectedFromOriginal(const Digikam::DColor&);
    void slotAutoAdjustExposure();

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

} // namespace DigikamEditorWhiteBalanceToolPlugin
