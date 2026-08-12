/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Parser
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imagequalityparser_p.h"

namespace Digikam
{

ImageQualityParser::ImageQualityParser(const DImg& image,
                                       const ImageQualitySettings& settings,
                                       PickLabel* const label)
    : d(new Private)
{
    d->imq   = settings;
    d->image = image;
    d->label = label;
}

ImageQualityParser::~ImageQualityParser()
{
    delete d;
}

void ImageQualityParser::startAnalyse()
{
    float finalQuality = -1.0F;

    // TODO See bug #424441: if image is null, report a defective quality

    cv::Mat cvImage    = AbstractDetector::prepareForDetection(d->image);

    cv::Mat grayImage;

    cv::cvtColor(cvImage, grayImage, cv::COLOR_BGR2GRAY);

    //-----------------------------------------------------------------------------

    std::unique_ptr<BlurDetector>        blurDetector;
    std::unique_ptr<NoiseDetector>       noiseDetector;
    std::unique_ptr<CompressionDetector> compressionDetector;
    std::unique_ptr<ExposureDetector>    exposureDetector;
    std::unique_ptr<AestheticDetector>   aestheticDetector;

    ImageQualityThreadPool pool(this, d->calculator);

    float aestheticScore = -1.0F;

    if (d->running)
    {
        if (d->imq.detectAesthetic)
        {
            if (AestheticDetector::s_isEmptyModel())
            {
                AestheticDetector::s_loadModel();
            }

            aestheticDetector = std::unique_ptr<AestheticDetector>(new AestheticDetector());
            aestheticScore    = aestheticDetector->detect(cvImage);
        }
        else
        {
            if (d->imq.detectBlur)
            {
                blurDetector = std::unique_ptr<BlurDetector>(new BlurDetector(d->image));

                pool.addDetector(cvImage, d->imq.blurWeight, blurDetector.get());
            }

            if (d->imq.detectNoise)
            {
                noiseDetector = std::unique_ptr<NoiseDetector>(new NoiseDetector());

                pool.addDetector(grayImage, d->imq.noiseWeight, noiseDetector.get());
            }

            if (d->imq.detectCompression)
            {
                compressionDetector = std::unique_ptr<CompressionDetector>(new CompressionDetector());

                pool.addDetector(cvImage, d->imq.compressionWeight, compressionDetector.get());
            }

            if (d->imq.detectExposure)
            {
                exposureDetector = std::unique_ptr<ExposureDetector>(new ExposureDetector());

                pool.addDetector(grayImage, d->imq.exposureWeight, exposureDetector.get());
            }

            pool.start();
            pool.end();
        }
    }

#ifdef TRACE
/*
    QFile filems("imgqsortresult.txt");

    if (filems.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream oms(&filems);
        oms << "File:" << d->image.originalFilePath() << Qt::endl;

        if (d->imq.detectBlur)
        {
            oms << "Blur Present:" << blur << Qt::endl;
            oms << "Blur Present(using LoG filter):"<< blur2 << Qt::endl;
        }

        if (d->imq.detectNoise)
        {
            oms << "Noise Present:" << noise << Qt::endl;
        }

        if (d->imq.detectCompression)
        {
            oms << "Compression Present:" << compressionLevel << Qt::endl;
        }

        if (d->imq.detectExposure)
        {
            oms << "Under-exposure Percents:" << underLevel << Qt::endl;
            oms << "Over-exposure Percents:"  << overLevel  << Qt::endl;
        }

        filems.close();
    }
*/
#endif // TRACE

    // Calculating finalquality

    *d->label = NoPickLabel;

    if (d->running)
    {
        if (d->imq.detectAesthetic)
        {
            if      (aestheticScore == -1.0F)
            {
                return;
            }
            else if (aestheticScore == 0.0F)
            {
                if (d->imq.lowQRejected)
                {
                    *d->label = RejectedLabel;
                }

                return;
            }
            else if (aestheticScore == 1.0F)
            {
                if (d->imq.mediumQPending)
                {
                    *d->label = PendingLabel;
                }

                return;
            }
            else if (d->imq.highQAccepted)
            {
                *d->label = AcceptedLabel;
            }
        }
        else
        {
            finalQuality =  d->calculator->calculateQuality();

            qCDebug(DIGIKAM_DIMG_LOG) << "Final Quality estimated: " << finalQuality;

            // Assigning PickLabels

            if      (finalQuality == -1.0F)
            {
                return;
            }
            else if ((int)finalQuality <= d->imq.rejectedThreshold)
            {
                if (d->imq.lowQRejected)
                {
                    *d->label = RejectedLabel;
                }

                return;
            }
            else if (
                     ((int)finalQuality >  d->imq.rejectedThreshold) &&     // cppcheck-suppress knownConditionTrueFalse
                     ((int)finalQuality <= d->imq.acceptedThreshold)
                    )
            {
                if (d->imq.mediumQPending)
                {
                    *d->label = PendingLabel;
                }

                return;
            }
            else if (d->imq.highQAccepted)
            {
                *d->label = AcceptedLabel;
            }
        }
    }
}

void ImageQualityParser::cancelAnalyse()
{
    d->running = false;
}

void ImageQualityParser::unloadDLModel()
{
    AestheticDetector::s_unloadModel();
}

} // namespace Digikam

#include "moc_imagequalityparser.cpp"
