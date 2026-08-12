/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-04
 * Description : an unit-test to detect image quality level - shared code
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imgqsorttest_shared.h"

// Qt includes

#include <QStringList>

// Local includes

#include "dimg.h"
#include "previewloadthread.h"
#include "imagequalityparser.h"
#include "imagequalitysettings.h"
#include "dpluginloader.h"

namespace Digikam
{

ImageQualitySettings ImgQSortTest_ArrangeSettings (DetectionType type)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Process images for detection type "<<type;

    qCInfo(DIGIKAM_TESTS_LOG)  << "Detection type (0:Blur, 1:Noise, 2:Compression, 3:Exposure, 4: General)";

    ImageQualitySettings settings;

    settings.detectBlur         = false;
    settings.detectNoise        = false;
    settings.detectCompression  = false;
    settings.detectExposure     = false;
    settings.detectAesthetic    = false;

    switch (type)
    {
        case DETECTNOISE:
            settings.detectNoise        = true;
            break;

        case DETECTCOMPRESSION:
            settings.detectCompression  = true;
            break;

        case DETECTEXPOSURE:
            settings.detectExposure     = true;
            break;

        case DETECTBLUR:
            settings.detectBlur         = true;
            break;

        case DETECTAESTHETIC:
            settings.detectAesthetic    = true;
            break;

        default:
            settings.detectBlur         = true;
            settings.detectCompression  = true;
            settings.detectNoise        = true;
            settings.detectExposure     = true;
            settings.detectAesthetic    = false;
            break;
    }

    return settings;
}

ImageQualitySettings ImgQSortTest_ArrangeCustomSettings(const CustomDetection& customSetting)
{
    qCInfo(DIGIKAM_TESTS_LOG)  << "Detection type activate Blur "<< customSetting.detectBlur
                               << "Noise "      << customSetting.detectNoise
                               << "Compression "<< customSetting.detectCompression
                               << "Exposure "   << customSetting.detectExposure;

    ImageQualitySettings settings;

    settings.detectBlur         = customSetting.detectBlur;
    settings.detectCompression  = customSetting.detectCompression;
    settings.detectNoise        = customSetting.detectNoise;
    settings.detectExposure     = customSetting.detectExposure;
    settings.detectAesthetic    = customSetting.detectAesthetic;

    return settings;
}

QHash<QString, int> ImgQSortTest_ParseTestImagesCore(const ImageQualitySettings& settings, const QFileInfoList& list)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Quality Detection Settings:" << settings;

    QHash<QString, int> results;

    for (const QFileInfo& inf : std::as_const(list))
    {
        QString path = inf.filePath();
        qCDebug(DIGIKAM_TESTS_LOG) << path;

        DImg dimg    = PreviewLoadThread::loadFastSynchronously(path, 1024);

        if (dimg.isNull())
        {
            qCDebug(DIGIKAM_TESTS_LOG) << path << "File cannot be loaded...";
        }

        PickLabel pick;
        ImageQualityParser parser(dimg, settings, &pick);
        parser.startAnalyse();
        results.insert( path.split(QLatin1String("/")).last(), pick);
    }

    qCInfo(DIGIKAM_TESTS_LOG) << "Quality Results (0:None, 1:Rejected, 2:Pending, 3:Accepted):";

    const auto keys = results.keys();

    for (const auto& image_name : keys)
    {
        qCInfo(DIGIKAM_TESTS_LOG) << "==>" << image_name << ":" << results.value(image_name);
    }

    return results;
}

QHash<QString, int> ImgQSortTest_ParseTestImagesDefautDetection(DetectionType type, const QFileInfoList& list)
{
    ImageQualitySettings settings = ImgQSortTest_ArrangeSettings(type);

    return ImgQSortTest_ParseTestImagesCore(settings, list);
}

QHash<QString, int> ImgQSortTest_ParseTestImagesCustomDetection(const CustomDetection& customSetting, const QFileInfoList& list)
{
    ImageQualitySettings settings = ImgQSortTest_ArrangeCustomSettings(customSetting);

    return ImgQSortTest_ParseTestImagesCore(settings, list);
}

} // namespace Digikam
