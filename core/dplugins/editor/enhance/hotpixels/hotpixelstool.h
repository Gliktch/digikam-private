/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : a digiKam image tool for fixing dots produced by
 *               hot/stuck/dead pixels from a CCD.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QPolygon>

// Local includes

#include "editortool.h"
#include "hotpixelprops.h"

class QUrl;

using namespace Digikam;

namespace DigikamEditorHotPixelsToolPlugin
{

class HotPixelsTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    explicit HotPixelsTool(QObject* const parent);
    ~HotPixelsTool()            override;

    static void registerFilter();

private Q_SLOTS:

    void slotHotPixels(const QPolygon&);
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

} // namespace DigikamEditorHotPixelsToolPlugin
