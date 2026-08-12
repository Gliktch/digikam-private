/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-04-04
 * Description : WEBP image export settings widget.
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

class DImgWEBPExportSettings : public DImgLoaderSettings
{
    Q_OBJECT

public:

    explicit DImgWEBPExportSettings(QWidget* const parent = nullptr);
    ~DImgWEBPExportSettings()                   override;

    /**
     * This widget manage 2 parameters for the image encoding:
     * "quality"  as integer [1 - 99].
     * "lossless" as boolean.
     */
    void setSettings(const DImgLoaderPrms& set) override;
    DImgLoaderPrms settings() const             override;

private Q_SLOTS:

    void slotToggleWEBPLossLess(bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
