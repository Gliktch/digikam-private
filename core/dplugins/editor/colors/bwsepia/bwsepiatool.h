/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-12-06
 * Description : Black and White conversion tool.
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEditorBWSepiaToolPlugin
{

class BWSepiaTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit BWSepiaTool(QObject* const parent);
    ~BWSepiaTool()              override;

private Q_SLOTS:

    void slotInit()             override;
    void slotSaveAsSettings()   override;
    void slotLoadSettings()     override;
    void slotResetSettings()    override;
    void slotPickerColorButtonActived(int);
    void slotSpotColorChanged(const Digikam::DColor&);
    void slotColorSelectedFromOriginal(const Digikam::DColor&, const QPoint&);

private:

    void readSettings()         override;
    void writeSettings()        override;
    void preparePreview()       override;
    void prepareFinal()         override;
    void setPreviewImage()      override;
    void setFinalImage()        override;
    void slotScaleChanged()     override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorBWSepiaToolPlugin
