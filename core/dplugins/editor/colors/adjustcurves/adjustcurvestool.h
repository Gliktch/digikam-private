/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image histogram adjust curves.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"
#include "dcolor.h"

using namespace Digikam;

namespace DigikamEditorAdjustCurvesToolPlugin
{

class AdjustCurvesTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit AdjustCurvesTool(QObject* const parent);
    ~AdjustCurvesTool()         override;

private Q_SLOTS:

    void slotSaveAsSettings()   override;
    void slotLoadSettings()     override;
    void slotResetSettings()    override;
    void slotPickerColorButtonActived(int);
    void slotSpotColorChanged();
    void slotColorSelectedFromOriginal(const Digikam::DColor&, const QPoint&);
    void slotResetCurrentChannel();
    void slotChannelChanged()   override;                       // cppcheck-suppress virtualCallInConstructor
    void slotScaleChanged()     override;                       // cppcheck-suppress virtualCallInConstructor

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

} // namespace DigikamEditorAdjustCurvesToolPlugin
