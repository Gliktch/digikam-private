/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-19
 * Description : a tool for color space conversion
 *
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"
#include "iccprofile.h"

using namespace Digikam;

namespace DigikamEditorProfileConversionToolPlugin
{

class ProfileConversionTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit ProfileConversionTool(QObject* const parent);
    ~ProfileConversionTool()    override;

    static QStringList favoriteProfiles();
    static void fastConversion(const IccProfile& profile);

private Q_SLOTS:

    void slotResetSettings()    override;
    void slotCurrentProfInfo();
    void slotProfileChanged();

private:

    void readSettings()         override;
    void writeSettings()        override;
    void preparePreview()       override;
    void prepareFinal()         override;
    void setPreviewImage()      override;
    void setFinalImage()        override;

    void updateTransform();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorProfileConversionToolPlugin
