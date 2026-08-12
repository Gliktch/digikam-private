/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-15-09
 * Description : a command line tool to convert RAW file to PNG
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QPointer>
#include <QScopedPointer>

// Local includes

#include "digikam_debug.h"
#include "drawdecoder.h"
#include "drawdecodersettings.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "raw2png - RAW Camera Image to PNG Converter";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <rawfile>";
        return -1;
    }

    QString            filePath = QString::fromLatin1(argv[1]);
    QFileInfo          input(filePath);
    QString            previewFilePath(input.baseName() + QString::QString::fromLatin1(".preview.png"));
    QFileInfo          previewOutput(previewFilePath);
    QString            halfFilePath(input.baseName() + QString::fromLatin1(".half.png"));
    QFileInfo          halfOutput(halfFilePath);
    QString            fullFilePath(input.baseName() + QString::fromLatin1(".full.png"));
    QFileInfo          fullOutput(fullFilePath);
    QImage             image;
    QScopedPointer<DRawInfo> identify(new DRawInfo);

    // -----------------------------------------------------------

    qCDebug(DIGIKAM_TESTS_LOG) << "raw2png: Identify RAW image from " << input.fileName();

    QPointer<DRawDecoder> rawProcessor(new DRawDecoder);

    if (!rawProcessor->rawFileIdentify(*identify, filePath))
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "raw2png: Identifying RAW image failed. Aborted...";
        return -1;
    }

    int width  = identify->imageSize.width();
    int height = identify->imageSize.height();

    qCDebug(DIGIKAM_TESTS_LOG) << "raw2png: Raw image info:";
    qCDebug(DIGIKAM_TESTS_LOG) << "--- Date:      " << identify->dateTime.toString(Qt::ISODate);
    qCDebug(DIGIKAM_TESTS_LOG) << "--- Make:      " << identify->make;
    qCDebug(DIGIKAM_TESTS_LOG) << "--- Model:     " << identify->model;
    qCDebug(DIGIKAM_TESTS_LOG) << "--- Size:      " << width << "x" << height;
    qCDebug(DIGIKAM_TESTS_LOG) << "--- Filter:    " << identify->filterPattern;
    qCDebug(DIGIKAM_TESTS_LOG) << "--- Colors:    " << identify->rawColors;

    // -----------------------------------------------------------

    qCDebug(DIGIKAM_TESTS_LOG) << "raw2png: Loading RAW image preview";

    if (!rawProcessor->loadRawPreview(image, filePath))
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "raw2png: Loading RAW image preview failed. Aborted...";
        return -1;
    }

    qCDebug(DIGIKAM_TESTS_LOG) << "raw2png: Saving preview image to "
             << previewOutput.fileName() << " size ("
             << image.width() << "x" << image.height()
             << ")";
    image.save(previewFilePath, "PNG");

    // -----------------------------------------------------------

    qCDebug(DIGIKAM_TESTS_LOG) << "raw2png: Loading half RAW image";

    image = QImage();

    if (!rawProcessor->loadHalfPreview(image, filePath))
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "raw2png: Loading half RAW image failed. Aborted...";
        return -1;
    }

    qCDebug(DIGIKAM_TESTS_LOG) << "raw2png: Saving half image to "
             << halfOutput.fileName() << " size ("
             << image.width() << "x" << image.height()
             << ")";

    image.save(halfFilePath, "PNG");

    // -----------------------------------------------------------

    qCDebug(DIGIKAM_TESTS_LOG) << "raw2png: Loading full RAW image";

    image = QImage();
    DRawDecoderSettings settings;
    settings.halfSizeColorImage    = false;
    settings.sixteenBitsImage      = false;
    settings.RGBInterpolate4Colors = false;
    settings.RAWQuality            = DRawDecoderSettings::BILINEAR;

    if (!rawProcessor->loadFullImage(image, filePath, settings))
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "raw2png: Loading full RAW image failed. Aborted...";
        return -1;
    }

    qCDebug(DIGIKAM_TESTS_LOG) << "raw2png: Saving full RAW image to "
             << fullOutput.fileName() << " size ("
             << image.width() << "x" << image.height()
             << ")";

    image.save(fullFilePath, "PNG");

    return 0;
}
