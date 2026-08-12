/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-03-12
 * Description : a command line tool to test autocrop of DImg
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2013 by Sayantan Datta <sayantan dot knz at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include <QFileInfo>
#include <QString>
#include <QRect>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "drawdecoding.h"
#include "autocrop.h"
#include "dimgthreadedfilter.h"
#include "metaengine.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "testautocrop - test auto-crop transform";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <image>";
        return -1;
    }

    MetaEngine::initializeExiv2();

    QFileInfo input(QString::fromUtf8(argv[1]));
    QString   outFilePath(input.baseName() + QLatin1String(".cropped.png"));

    DRawDecoderSettings settings;
    settings.halfSizeColorImage    = false;
    settings.sixteenBitsImage      = true;
    settings.RGBInterpolate4Colors = false;
    settings.RAWQuality            = DRawDecoderSettings::BILINEAR;

    DImg img(input.filePath(), nullptr, DRawDecoding(settings));

    AutoCrop ac(&img);
    ac.startFilterDirectly();
    QRect rect = ac.autoInnerCrop();

    qCDebug(DIGIKAM_TESTS_LOG) << "Cropped image area: " << rect;

    img.crop(rect);
    img.save(outFilePath, QLatin1String("PNG"));

    return 0;
}
