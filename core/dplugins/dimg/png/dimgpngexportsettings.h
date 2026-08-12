/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : PNG image export settings widget.
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

class DImgPNGExportSettings : public DImgLoaderSettings
{
    Q_OBJECT

public:

    explicit DImgPNGExportSettings(QWidget* const parent = nullptr);
    ~DImgPNGExportSettings()                    override;

    /**
     * This widget manage 2 parameters for the image encoding:
     * "quality"  as integer [1 - 9].
     */
    void setSettings(const DImgLoaderPrms& set) override;
    DImgLoaderPrms settings() const             override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
