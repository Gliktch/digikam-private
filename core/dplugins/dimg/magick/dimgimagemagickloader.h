/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-04-19
 * Description : Magick loader for DImg framework.
 *
 * SPDX-FileCopyrightText: 2019-2025 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dimg.h"
#include "dimgloader.h"
#include "digikam_config.h"

using namespace Digikam;

namespace DigikamImageMagickDImgPlugin
{

class DImgImageMagickLoader : public DImgLoader
{
public:

    explicit DImgImageMagickLoader(DImg* const image);
    ~DImgImageMagickLoader()                                               override = default;

    bool load(const QString& filePath, DImgLoaderObserver* const observer) override;
    bool save(const QString& filePath, DImgLoaderObserver* const observer) override;

    bool hasAlpha()                                                  const override;
    bool sixteenBit()                                                const override;
    bool isReadOnly()                                                const override;

private:

    bool m_sixteenBit   = false;
    bool m_hasAlpha     = false;
};

} // namespace DigikamImageMagickDImgPlugin
