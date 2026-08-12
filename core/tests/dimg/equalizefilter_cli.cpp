/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2016-04-08
 * Description : a command line tool to test DImg equalize filter
 *
 * SPDX-FileCopyrightText: 2016-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include <QFileInfo>

// Local includes

#include "digikam_debug.h"
#include "metaengine.h"
#include "equalizefilter.h"
#include "dimg.h"
#include "dimgthreadedfilter.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "testequalizefilter - test DImg equalize algorithm";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <image>";
        return -1;
    }

    MetaEngine::initializeExiv2();

    QFileInfo input(QString::fromUtf8(argv[1]));
    QString   outFilePath(input.baseName() + QLatin1String(".out.png"));

    DRawDecoderSettings settings;
    settings.halfSizeColorImage    = false;
    settings.sixteenBitsImage      = false;
    settings.RGBInterpolate4Colors = false;
    settings.RAWQuality            = DRawDecoderSettings::BILINEAR;

    DImg img(input.filePath(), nullptr, DRawDecoding(settings));
    EqualizeFilter filter(&img, &img);
    filter.startFilterDirectly();
    img.putImageData(filter.getTargetImage().bits());
    img.save(outFilePath, QLatin1String("PNG"));

    return 0;
}
