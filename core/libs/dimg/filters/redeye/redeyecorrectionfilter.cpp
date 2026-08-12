/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 17-8-2016
 * Description : A Red-Eye automatic detection and correction filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2016      by Omar Amin <Omar dot moh dot amin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "redeyecorrectionfilter.h"

// C++ includes

#include <iterator>

// Qt includes

#include <QFile>
#include <QDataStream>
#include <QListIterator>
#include <QStandardPaths>
#include <QMutexLocker>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated
#include "dnnmodelmanager.h"
#include "dnnmodelyunet.h"
#include "qtopencvimg.h"

namespace Digikam
{

class Q_DECL_HIDDEN RedEyeCorrectionFilter::Private
{
public:

    Private() = default;

    DNNModelYuNet*                  model           = nullptr;
    int                             faceThreshold   = 7;

    RedEyeCorrectionContainer       settings;
};

RedEyeCorrectionFilter::RedEyeCorrectionFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d                 (new Private)
{
    initFilter();
}

RedEyeCorrectionFilter::RedEyeCorrectionFilter(DImg* const orgImage,
                                               QObject* const parent,
                                               const RedEyeCorrectionContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("RedEyeCorrection")),
      d                 (new Private)
{
    d->settings = settings;
    initFilter();
}

RedEyeCorrectionFilter::RedEyeCorrectionFilter(const RedEyeCorrectionContainer& settings,
                                               DImgThreadedFilter* const parentFilter,
                                               const DImg& orgImage,
                                               const DImg& destImage,
                                               int progressBegin,
                                               int progressEnd)
    : DImgThreadedFilter(parentFilter, orgImage, destImage,
                         progressBegin, progressEnd,
                         parentFilter->filterName() + QLatin1String(": RedEyeCorrection")),
      d                 (new Private)
{
    d->settings = settings;
    this->filterImage();
}

RedEyeCorrectionFilter::~RedEyeCorrectionFilter()
{
    cancelFilter();
    delete d;
}

QString RedEyeCorrectionFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("RedEye Correction Filter"));
}

void RedEyeCorrectionFilter::filterImage()
{
    if (!runningFlag())
    {
        return;
    }

    if (!d->model)
    {
        d->model = static_cast<DNNModelYuNet*>(DNNModelManager::instance()->getModel(QLatin1String("YuNet"), DNNModelUsage::DNNUsageFaceDetection));

        if (!d->model)
        {
            qCDebug(DIGIKAM_DIMG_LOG) << "RedEyeCorrectionFilter::filterImage: Error get YuNet model";
            return;
        }

        d->model->getNet();
    }

    if (!d->model->modelLoaded)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "RedEyeCorrectionFilter::filterImage: Error loading YuNet model";
        return;
    }

    // convert the image to CV_8UC3 RGB

    cv::Mat faceLandmarks;
    cv::Mat cvImage = QtOpenCVImg::image2Mat(m_orgImage, 
                                             CV_8UC3, 
                                             QtOpenCVImg::MatColorOrder::MCO_BGR);

    // extract the face landmarks

    QMutexLocker lock(&d->model->mutex);
    d->model->getNet()->setScoreThreshold(d->model->getThreshold(d->faceThreshold));
    d->model->getNet()->setNMSThreshold(d->model->getThreshold(d->faceThreshold) * 0.8);
    d->model->getNet()->setInputSize(cvImage.size());
    d->model->getNet()->detect(cvImage, faceLandmarks);

    // find eye bounding boxes

    std::vector<cv::Rect> eyes;

    for (int i = 0 ; i < faceLandmarks.rows ; ++i)
    {
        int eyeWidth = (int)faceLandmarks.at<float>(i, 2) * 0.14f;
        int eyeHeight = (int)faceLandmarks.at<float>(i, 3) * 0.08f;

        // get the bounding boxes for eyes

        cv::Rect eyeRectR = cv::Rect(faceLandmarks.at<float>(i, 4) - eyeWidth,
                                     faceLandmarks.at<float>(i, 5) - eyeHeight,
                                     eyeWidth * 2,
                                     eyeHeight * 2);

        cv::Rect eyeRectL = cv::Rect(faceLandmarks.at<float>(i, 6) - eyeWidth,
                                     faceLandmarks.at<float>(i, 7) - eyeHeight,
                                     eyeWidth * 2,
                                     eyeHeight * 2);

        // use eyes that don't exceed the face bounding box and image bounds

        if ((eyeRectR.x > 0) &&
            (eyeRectR.x > (int)faceLandmarks.at<float>(i, 0)))
        {
            eyes.push_back(eyeRectR);
        }

        if ((eyeRectL.x + eyeRectL.width < cvImage.cols) &&
            (eyeRectL.x + eyeRectL.width < (int)faceLandmarks.at<float>(i, 0) + (int)faceLandmarks.at<float>(i, 2)))
        {
            eyes.push_back(eyeRectL);
        }
    }

    cv::Mat intermediateImage;
    int type          = m_orgImage.sixteenBit() ? CV_16UC4 : CV_8UC4;

    intermediateImage = cv::Mat(m_orgImage.height(), m_orgImage.width(),
                                type, m_orgImage.bits());

    for (unsigned int j = 0 ; runningFlag() && (j < eyes.size()) ; ++j)
    {
        correctRedEye(intermediateImage.data,
                        intermediateImage.type(),
                        eyes[j],
                        cv::Rect(0, 0, intermediateImage.size().width ,
                                        intermediateImage.size().height));
    }

    if (runningFlag())
    {
        m_destImage.putImageData(m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit(),
                                 !m_orgImage.hasAlpha(), intermediateImage.data, true);
    }
}

void RedEyeCorrectionFilter::correctRedEye(uchar* data, int type,
                                           const cv::Rect& eyerect, const cv::Rect& imgRect)
{
    uchar*  onebytedata = data;
    ushort* twobytedata = reinterpret_cast<ushort*>(data);
    int     pixeldepth  = 0;

    if ((type == CV_8UC4) || (type == CV_16UC4))
    {
        pixeldepth = 4;
    }
    else
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Unsupported Type in redeye correction filter";
    }

    bool sixteendepth = (type == CV_16UC4) ? true : false;
    double redratio   = d->settings.m_redToAvgRatio;

    for (int i = eyerect.y ; i < eyerect.y + eyerect.height ; ++i)
    {
        for (int j = eyerect.x ; j < eyerect.x + eyerect.width ; ++j)
        {
            uint pixelindex = (i*imgRect.width + j) * pixeldepth;
            onebytedata     = &(reinterpret_cast<uchar*> (data)[pixelindex]);
            twobytedata     = &(reinterpret_cast<ushort*>(data)[pixelindex]);

            if (sixteendepth)
            {
                float redIntensity = ((float)twobytedata[2] / (( (unsigned int)twobytedata[1]
                                                               + (unsigned int)twobytedata[0]) / 2));
                if (redIntensity > redratio)
                {
                    // reduce red to the average of blue and green

                    twobytedata[2] = ((int)twobytedata[1] + (int)twobytedata[0]) / 2;
                }
            }
            else
            {
                float redIntensity = ((float)onebytedata[2] / (( (unsigned int)onebytedata[1]
                                                               + (unsigned int)onebytedata[0]) / 2));
                if (redIntensity > redratio)
                {
                    // reduce red to the average of blue and green

                    onebytedata[2] = ((int)onebytedata[1] + (int)onebytedata[0]) / 2;
                }
            }
        }
    }
}

void RedEyeCorrectionFilter::QRectFtocvRect(const QList<QRect>& faces, std::vector<cv::Rect>& result)
{
    QListIterator<QRect> listit(faces);

    while (listit.hasNext())
    {
        QRect  temp = listit.next();
        result.push_back(cv::Rect(temp.topLeft().rx(), temp.topLeft().ry(),
                                  temp.width()       , temp.height()) );
    }
}

FilterAction RedEyeCorrectionFilter::filterAction()
{
    DefaultFilterAction<RedEyeCorrectionFilter> action;
    d->settings.writeToFilterAction(action);

    return action;
}

void RedEyeCorrectionFilter::readParameters(const FilterAction& action)
{
    d->settings = RedEyeCorrectionContainer::fromFilterAction(action);
}

} // namespace Digikam

#include "moc_redeyecorrectionfilter.cpp"
