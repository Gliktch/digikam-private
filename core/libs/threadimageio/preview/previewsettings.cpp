/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-11-14
 * Description : Settings container for preview settings
 *
 * SPDX-FileCopyrightText: 2014 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "previewsettings.h"

namespace Digikam
{

PreviewSettings::PreviewSettings(Quality quality, RawLoading rawLoading)
    : quality   (quality),
      rawLoading(rawLoading)
{
}

PreviewSettings PreviewSettings::fastPreview()
{
    return PreviewSettings(FastPreview, RawPreviewAutomatic);
}

PreviewSettings PreviewSettings::highQualityPreview()
{
    return PreviewSettings(FastButLargePreview, RawPreviewAutomatic);
}

bool PreviewSettings::operator==(const PreviewSettings& other) const
{
    return (
            (quality           == other.quality)     &&
            (rawLoading        == other.rawLoading)  &&
            (convertToEightBit == other.convertToEightBit)
           );
}

} // namespace Digikam
