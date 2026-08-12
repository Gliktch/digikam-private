/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-06
 * Description : PGF image export settings widget.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dimgloadersettings.h"

namespace Digikam
{

class DImgPGFExportSettings : public DImgLoaderSettings
{
    Q_OBJECT

public:

    explicit DImgPGFExportSettings(QWidget* const parent = nullptr);
    ~DImgPGFExportSettings()                    override;

    /**
     * This widget manage 2 parameters for the image encoding:
     * "quality"  as integer [1 - 9].
     * "lossless" as boolean.
     */
    void setSettings(const DImgLoaderPrms& set) override;
    DImgLoaderPrms settings() const             override;

private Q_SLOTS:

    void slotTogglePGFLossLess(bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
