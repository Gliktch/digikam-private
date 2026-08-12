/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-23
 * Description : a command line tool to test DImg image loader
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2013 by Sayantan Datta <sayantan dot knz at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include <QFileInfo>
#include <QString>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "drawdecoding.h"
#include "nrestimate.h"
#include "nrfilter.h"
#include "dimgthreadedfilter.h"
#include "metaengine.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "testnrestimate - test NR parameters";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <image>";
        return -1;
    }

    MetaEngine::initializeExiv2();

    QFileInfo input(QString::fromUtf8(argv[1]));
    QString   outFilePath(input.baseName() + QLatin1String(".denoise.png"));

    DRawDecoderSettings settings;
    settings.halfSizeColorImage    = false;
    settings.sixteenBitsImage      = true;
    settings.RGBInterpolate4Colors = false;
    settings.RAWQuality            = DRawDecoderSettings::BILINEAR;

    DImg img(input.filePath(), nullptr, DRawDecoding(settings));
    NREstimate nre(&img);
    nre.setLogFilesPath(input.filePath());
    nre.startFilterDirectly();
    NRContainer prm = nre.settings();

    qCDebug(DIGIKAM_TESTS_LOG) << prm;

    NRFilter nrf(&img, nullptr, prm);
    nrf.startFilterDirectly();
    img.putImageData(nrf.getTargetImage().bits());
    img.save(outFilePath, QLatin1String("PNG"));

    return 0;
}
