/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-04-01
 * Description : JPEG-XL image export settings widget.
 *
 * SPDX-FileCopyrightText: 2022-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dimgloadersettings.h"

namespace Digikam
{

class DImgJXLExportSettings : public DImgLoaderSettings
{
    Q_OBJECT

public:

    explicit DImgJXLExportSettings(QWidget* const parent = nullptr);
    ~DImgJXLExportSettings()                    override;

    /**
     * This widget manage 2 parameters for the image encoding:
     * "quality"  as integer [1 - 99].
     * "lossless" as boolean.
     */
    void setSettings(const DImgLoaderPrms& set) override;
    DImgLoaderPrms settings() const             override;

private Q_SLOTS:

    void slotToggleJXLLossLess(bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
