/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : JPEG image export settings widget.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dimgloadersettings.h"

namespace Digikam
{

class DImgJPEGExportSettings : public DImgLoaderSettings
{
    Q_OBJECT

public:

    explicit DImgJPEGExportSettings(QWidget* const parent = nullptr);
    ~DImgJPEGExportSettings()                   override;

    /**
     * This widget manage 2 parameters for the image encoding:
     * "quality"  as integer [1 - 100].
     * "subsampling" as integer [0 - 3].
     */
    void setSettings(const DImgLoaderPrms& set) override;
    DImgLoaderPrms settings() const             override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
