/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : TIFF image export settings widget.
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

class DImgTIFFExportSettings : public DImgLoaderSettings
{
    Q_OBJECT

public:

    explicit DImgTIFFExportSettings(QWidget* const parent = nullptr);
    ~DImgTIFFExportSettings()                   override;

    /**
     * This widget manage 2 parameters for the image encoding:
     * "compress" as boolean.
     */
    void setSettings(const DImgLoaderPrms& set) override;
    DImgLoaderPrms settings() const             override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
