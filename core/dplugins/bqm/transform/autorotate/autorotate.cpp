/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-03-09
 * Description : a BQM plugin to automatically rotate images.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2025      by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "autorotate.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "dcombobox.h"
#include "dimg.h"
#include "dimgbuiltinfilter.h"
#include "dmetadata.h"
#include "jpegutils.h"
#include "freerotationfilter.h"
#include "freerotationsettings.h"
#include "autorotator.h"

namespace DigikamBqmAutoRotatePlugin
{

AutoRotate::AutoRotate(QObject* const parent)
    : BatchTool(QLatin1String("AutoRotate"), TransformTool, parent)
{
}

BatchToolSettings AutoRotate::defaultSettings()
{
    return BatchToolSettings();
}

BatchTool* AutoRotate::clone(QObject* const parent) const
{
    return new AutoRotate(parent);
}

bool AutoRotate::toolOperations()
{
    // NOTE: for JPEG image uses lossless method if non-custom rotation angle.

    AutoRotator autorotator;
    MetaEngineRotation::TransformationAction rotation = autorotator.rotationOrientation(inputUrl().toLocalFile(), 10);

    if (JPEGUtils::isJpegImage(inputUrl().toLocalFile()) && image().isNull())
    {
        JPEGUtils::JpegRotator rotator(inputUrl().toLocalFile());
        rotator.setDestinationFile(outputUrl().toLocalFile());

        return rotator.exifTransform(rotation);
    }

    // Non-JPEG image: DImg

    if (!loadToDImg())
    {
        return false;
    }

    switch (rotation)
    {
        case MetaEngineRotation::Rotate90:
        {
            DImgBuiltinFilter filter(DImgBuiltinFilter::Rotate90);
            applyFilter(&filter);
            break;
        }

        case MetaEngineRotation::Rotate180:
        {
            DImgBuiltinFilter filter(DImgBuiltinFilter::Rotate180);
            applyFilter(&filter);
            break;
        }

        case MetaEngineRotation::Rotate270:
        {
            DImgBuiltinFilter filter(DImgBuiltinFilter::Rotate270);
            applyFilter(&filter);
            break;
        }

        default:
        {
            // no transformation

            break; 
        }
    }

    return (savefromDImg());
}

} // namespace DigikamBqmAutoRotatePlugin

#include "moc_autorotate.cpp"
