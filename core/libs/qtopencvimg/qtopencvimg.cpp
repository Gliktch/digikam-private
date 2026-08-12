/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-12-23
 * Description : Helper methods to convert OpenCV image to Qt containers and vis-versa.
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 * SPDX-FileCopyrightText: 2012-2015 by Debao Zhang    <hello at debao dot me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "qtopencvimg.h"

// Qt includes

#include <QSysInfo>
#include <QDebug>

// digikam includes

#include "digikam_debug.h"

namespace Digikam
{

cv::Mat QtOpenCVImg::argb2bgra(const cv::Mat& mat)
{
    Q_ASSERT(mat.channels() == 4);

    cv::Mat newMat(mat.rows, mat.cols, mat.type());
    int from_to[] = { 0, 3, 1, 2, 2, 1, 3, 0 };

    try
    {
        cv::mixChannels(&mat, 1, &newMat, 1, from_to, 4);
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_QTOPENCVIMG_LOG) << "QtOpenCVImg::argb2bgra: cv::Exception:" << e.what();

        return cv::Mat();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_QTOPENCVIMG_LOG) << "QtOpenCVImg::argb2bgra: Default exception from OpenCV";

        return cv::Mat();
    }

    return newMat;
}

cv::Mat QtOpenCVImg::adjustChannelsOrder(const cv::Mat& srcMat, MatColorOrder srcOrder, MatColorOrder targetOrder)
{
    Q_ASSERT(srcMat.channels() == 4);

    if (srcOrder == targetOrder)
    {
        return srcMat.clone();
    }

    cv::Mat desMat;

    try
    {
        if     (
                ((srcOrder == MCO_ARGB) && (targetOrder == MCO_BGRA)) ||
                ((srcOrder == MCO_BGRA) && (targetOrder == MCO_ARGB))
               )
        {
            // ARGB <==> BGRA

            desMat = argb2bgra(srcMat);
        }
        else if (srcOrder == MCO_ARGB && targetOrder == MCO_RGBA)
        {
            // ARGB ==> RGBA

            desMat        = cv::Mat(srcMat.rows, srcMat.cols, srcMat.type());
            int from_to[] = { 0, 3, 1, 0, 2, 1, 3, 2 };
            cv::mixChannels(&srcMat, 1, &desMat, 1, from_to, 4);
        }
        else if (srcOrder == MCO_RGBA && targetOrder == MCO_ARGB)
        {
            // RGBA ==> ARGB

            desMat        = cv::Mat(srcMat.rows, srcMat.cols, srcMat.type());
            int from_to[] = { 0, 1, 1, 2, 2, 3, 3, 0 };
            cv::mixChannels(&srcMat, 1, &desMat, 1, from_to, 4);
        }
        else
        {
            // BGRA <==> RBGA

            cv::cvtColor(srcMat, desMat, cv::ColorConversionCodes::COLOR_BGRA2RGBA);
        }
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_QTOPENCVIMG_LOG) << "QtOpenCVImg::adjustChannelsOrder: cv::Exception:" << e.what();

        return cv::Mat();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_QTOPENCVIMG_LOG) << "QtOpenCVImg::adjustChannelsOrder: Default exception from OpenCV";

        return cv::Mat();
    }

    return desMat;
}

QImage::Format QtOpenCVImg::findClosestFormat(QImage::Format formatHint)
{
    QImage::Format format = QImage::Format_Invalid;

    switch (formatHint)
    {
        case QImage::Format_Indexed8:
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:

#if QT_VERSION >= 0x040400

        case QImage::Format_RGB888:

#endif

#if QT_VERSION >= 0x050200

        case QImage::Format_RGBX8888:
        case QImage::Format_RGBA8888:
        case QImage::Format_RGBA8888_Premultiplied:

#endif

#if QT_VERSION >= 0x050500

        case QImage::Format_Alpha8:
        case QImage::Format_Grayscale8:

#endif

        {
            format = formatHint;

            break;
        }

        case QImage::Format_Mono:
        case QImage::Format_MonoLSB:
        {
            format = QImage::Format_Indexed8;

            break;
        }

        case QImage::Format_RGB16:
        {
            format = QImage::Format_RGB32;

            break;
        }

#if QT_VERSION > 0x040400

        case QImage::Format_RGB444:
        case QImage::Format_RGB555:
        case QImage::Format_RGB666:
        {
            format = QImage::Format_RGB888;

            break;
        }

        case QImage::Format_ARGB4444_Premultiplied:
        case QImage::Format_ARGB6666_Premultiplied:
        case QImage::Format_ARGB8555_Premultiplied:
        case QImage::Format_ARGB8565_Premultiplied:
        {
            format = QImage::Format_ARGB32_Premultiplied;

            break;
        }

#endif

        default:
        {
            format = QImage::Format_ARGB32;

            break;
        }
    }

    return format;
}

QtOpenCVImg::MatColorOrder QtOpenCVImg::getColorOrderOfRGB32Format()
{

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN

    return MCO_BGRA;

#else

    return MCO_ARGB;

#endif

}

cv::Mat QtOpenCVImg::image2Mat(const DImg& img, int requiredMatType, MatColorOrder requiredOrder)
{
    int targetDepth    = CV_MAT_DEPTH(requiredMatType);
    int targetChannels = CV_MAT_CN(requiredMatType);

    Q_ASSERT((targetChannels == CV_CN_MAX) || (targetChannels == 1)   || (targetChannels == 3)  || (targetChannels == 4));

    Q_ASSERT((targetDepth == CV_8U)        || (targetDepth == CV_16U) || (targetDepth == CV_32F));

    if (img.isNull())
    {
        return cv::Mat();
    }

    // DImg is always 4 channel BGR (MCO_BGRA) so the starting cv::Mat is either CV_16UC4 or CV_8UC4

    int type = (img.sixteenBit() ? CV_16UC4 : CV_8UC4);

    // copy the image data to cv::Mat

    cv::Mat mat0;

    try
    {
        // deep copy is necessary here otherwise the memory of DImg will be changed

        mat0 = cv::Mat(img.height(), img.width(), type, img.bits()).clone();

        // apply scalar to 16-bit images

        if (CV_16UC4 == type)
        {
            // 16 bits image

            mat0 = mat0 / 255;
        }

        // convert the color order if needed

        if (MCO_BGRA != requiredOrder)
        {
            mat0 = adjustChannelsOrder(mat0, MCO_BGRA, requiredOrder);
        }

        // convert to target bit depth if needed

        if (targetDepth != CV_MAT_DEPTH(type))
        {
            mat0.convertTo(mat0, CV_MAKE_TYPE(targetDepth, mat0.channels()));
        }

        // convert the number of color channels if needed

        if (targetChannels != CV_MAT_CN(type))
        {
            switch (targetChannels)
            {
                case 1:
                {
                    if      (MCO_RGB == requiredOrder)
                    {
                        cv::cvtColor(mat0, mat0, cv::ColorConversionCodes::COLOR_RGBA2GRAY);
                    }
                    else if (MCO_BGR == requiredOrder)
                    {
                        cv::cvtColor(mat0, mat0, cv::ColorConversionCodes::COLOR_BGRA2GRAY);
                    }

                    break;
                }
                case 3:
                {
                    if      (MCO_RGB == requiredOrder)
                    {
                        cv::cvtColor(mat0, mat0, cv::ColorConversionCodes::COLOR_RGBA2RGB);
                    }
                    else if (MCO_BGR == requiredOrder)
                    {
                        cv::cvtColor(mat0, mat0, cv::ColorConversionCodes::COLOR_BGRA2BGR);
                    }

                    break;
                }
            }
        }
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_QTOPENCVIMG_LOG) << "QtOpenCVImg::image2Mat: cv::Exception:" << e.what();

        return cv::Mat();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_QTOPENCVIMG_LOG) << "QtOpenCVImg::image2Mat: Default exception from OpenCV";

        return cv::Mat();
    }

    return mat0;
}

cv::Mat QtOpenCVImg::image2Mat(const QImage& img, int requiredMatType, MatColorOrder requriedOrder)
{
    int targetDepth    = CV_MAT_DEPTH(requiredMatType);
    int targetChannels = CV_MAT_CN(requiredMatType);

    Q_ASSERT((targetChannels == CV_CN_MAX) || (targetChannels == 1)   || (targetChannels == 3)  || (targetChannels == 4));

    Q_ASSERT((targetDepth == CV_8U)        || (targetDepth == CV_16U) || (targetDepth == CV_32F));

    if (img.isNull())
    {
        return cv::Mat();
    }

    cv::Mat mat_adjustDepth;

    try
    {
        // Find the closest image format that can be used in image2Mat_shared()

        QImage::Format format  = findClosestFormat(img.format());
        QImage image           = ((format == img.format()) ? img : img.convertToFormat(format));

        MatColorOrder srcOrder = MCO_INVALID;
        cv::Mat mat0           = image2Mat_shared(image, &srcOrder);

        // Adjust mat channels if needed.

        cv::Mat mat_adjustCn;
        const float maxAlpha   = ((targetDepth == CV_8U) ? 255
                                                         : ((targetDepth == CV_16U) ? 65535
                                                                                    : 1.0));

        if (targetChannels == CV_CN_MAX)
        {
            targetChannels = mat0.channels();
        }

        switch (targetChannels)
        {
            case 1:
            {
                if      (mat0.channels() == 3)
                {
                    cv::cvtColor(mat0, mat_adjustCn, cv::ColorConversionCodes::COLOR_RGB2GRAY);
                }
                else if (mat0.channels() == 4)
                {
                    if      (srcOrder == MCO_BGRA)
                    {
                        cv::cvtColor(mat0, mat_adjustCn, cv::ColorConversionCodes::COLOR_BGRA2GRAY);
                    }
                    else if (srcOrder == MCO_RGBA)
                    {
                        cv::cvtColor(mat0, mat_adjustCn, cv::ColorConversionCodes::COLOR_RGBA2GRAY);
                    }
                    else    // MCO_ARGB
                    {
                        cv::cvtColor(argb2bgra(mat0), mat_adjustCn, cv::ColorConversionCodes::COLOR_BGRA2GRAY);
                    }
                }

                break;
            }

            case 3:
            {
                if      (mat0.channels() == 1)
                {
                    cv::cvtColor(mat0, mat_adjustCn, ((requriedOrder == MCO_BGR) ? cv::ColorConversionCodes::COLOR_GRAY2BGR : cv::ColorConversionCodes::COLOR_GRAY2RGB));
                }
                else if (mat0.channels() == 3)
                {
                    if (requriedOrder != srcOrder)
                    {
                        cv::cvtColor(mat0, mat_adjustCn, cv::ColorConversionCodes::COLOR_RGB2BGR);
                    }
                }

                else if (mat0.channels() == 4)
                {
                    if      (srcOrder == MCO_ARGB)
                    {
                        mat_adjustCn   = cv::Mat(mat0.rows, mat0.cols, CV_MAKE_TYPE(mat0.type(), 3));
                        int ARGB2RGB[] = { 1, 0, 2, 1, 3, 2 };
                        int ARGB2BGR[] = { 1, 2, 2, 1, 3, 0 };
                        cv::mixChannels(&mat0, 1, &mat_adjustCn, 1, ((requriedOrder == MCO_BGR) ? ARGB2BGR : ARGB2RGB), 3);
                    }
                    else if (srcOrder == MCO_BGRA)
                    {
                        cv::cvtColor(mat0, mat_adjustCn, ((requriedOrder == MCO_BGR) ? cv::ColorConversionCodes::COLOR_BGRA2BGR : cv::ColorConversionCodes::COLOR_BGRA2RGB));
                    }
                    else    // RGBA
                    {
                        cv::cvtColor(mat0, mat_adjustCn, ((requriedOrder == MCO_BGR) ? cv::ColorConversionCodes::COLOR_RGBA2BGR : cv::ColorConversionCodes::COLOR_RGBA2RGB));
                    }
                }

                break;
            }

            case 4:
            {
                if (mat0.channels() == 1)
                {
                    if      (requriedOrder == MCO_ARGB)
                    {
                        cv::Mat alphaMat(mat0.rows, mat0.cols, CV_MAKE_TYPE(mat0.type(), 1), cv::Scalar(maxAlpha));
                        mat_adjustCn  = cv::Mat(mat0.rows, mat0.cols, CV_MAKE_TYPE(mat0.type(), 4));
                        cv::Mat in[]  = { alphaMat, mat0 };
                        int from_to[] = { 0, 0, 1, 1, 1, 2, 1, 3 };
                        cv::mixChannels(in, 2, &mat_adjustCn, 1, from_to, 4);
                    }
                    else if (requriedOrder == MCO_RGBA)
                    {
                        cv::cvtColor(mat0, mat_adjustCn, cv::ColorConversionCodes::COLOR_GRAY2RGBA);
                    }
                    else    // MCO_BGRA
                    {
                        cv::cvtColor(mat0, mat_adjustCn, cv::ColorConversionCodes::COLOR_GRAY2BGRA);
                    }
                }
                else if (mat0.channels() == 3)
                {
                    if      (requriedOrder == MCO_ARGB)
                    {
                        cv::Mat alphaMat(mat0.rows, mat0.cols, CV_MAKE_TYPE(mat0.type(), 1), cv::Scalar(maxAlpha));
                        mat_adjustCn  = cv::Mat(mat0.rows, mat0.cols, CV_MAKE_TYPE(mat0.type(), 4));
                        cv::Mat in[]  = { alphaMat, mat0 };
                        int from_to[] = { 0, 0, 1, 1, 2, 2, 3, 3 };
                        cv::mixChannels(in, 2, &mat_adjustCn, 1, from_to, 4);
                    }
                    else if (requriedOrder == MCO_RGBA)
                    {
                        cv::cvtColor(mat0, mat_adjustCn, cv::ColorConversionCodes::COLOR_RGB2RGBA);
                    }
                    else    // MCO_BGRA
                    {
                        cv::cvtColor(mat0, mat_adjustCn, cv::ColorConversionCodes::COLOR_RGB2BGRA);
                    }
                }
                else if (mat0.channels() == 4)
                {
                    if (srcOrder != requriedOrder)
                    {
                        mat_adjustCn = adjustChannelsOrder(mat0, srcOrder, requriedOrder);
                    }
                }

                break;
            }

            default:
            {
                break;
            }
        }

        // Adjust depth if needed.

        if (targetDepth == CV_8U)
        {
            return (mat_adjustCn.empty() ? mat0.clone() : mat_adjustCn);
        }

        if (mat_adjustCn.empty())
        {
            mat_adjustCn = mat0;
        }


        mat_adjustCn.convertTo(
                               mat_adjustDepth,
                               CV_MAKE_TYPE(targetDepth, mat_adjustCn.channels()),
                               ((targetDepth == CV_16U) ? 255.0 : (1 / 255.0))
                              );
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_QTOPENCVIMG_LOG) << "QtOpenCVImg::image2Mat: cv::Exception:" << e.what();

        return cv::Mat();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_QTOPENCVIMG_LOG) << "QtOpenCVImg::image2Mat: Default exception from OpenCV";

        return cv::Mat();
    }

    return mat_adjustDepth;
}

QImage QtOpenCVImg::mat2Image(const cv::Mat& mat, MatColorOrder order, QImage::Format formatHint)
{
    Q_ASSERT((mat.channels() == 1)  || (mat.channels() == 3)   || (mat.channels() == 4));

    Q_ASSERT((mat.depth() == CV_8U) || (mat.depth() == CV_16U) || (mat.depth() == CV_32F));

    try
    {
        if (mat.empty())
        {
            return QImage();
        }

        // Adjust mat channels if needed, and find proper QImage format.

        QImage::Format format = QImage::Format_Invalid;
        cv::Mat mat_adjustCn;

        if (mat.channels() == 1)
        {
            format = formatHint;

            if (
                   (formatHint != QImage::Format_Indexed8)

#if QT_VERSION >= 0x050500

                && (formatHint != QImage::Format_Alpha8)
                && (formatHint != QImage::Format_Grayscale8)

#endif

               )
            {
                format = QImage::Format_Indexed8;
            }
        }

        else if (mat.channels() == 3)
        {

#if QT_VERSION >= 0x040400

            format = QImage::Format_RGB888;

            if (order == MCO_BGR)
            {
                cv::cvtColor(mat, mat_adjustCn, cv::ColorConversionCodes::COLOR_BGR2RGB);
            }

#else

            format = QImage::Format_RGB32;
            cv::Mat mat_tmp;
            cv::cvtColor(mat, mat_tmp, ((order == MCO_BGR) ? cv::ColorConversionCodes::COLOR_BGR2BGRA : cv::ColorConversionCodes::COLOR_RGB2BGRA));

#   if Q_BYTE_ORDER == Q_LITTLE_ENDIAN

            mat_adjustCn = mat_tmp;

#   else

            mat_adjustCn = argb2bgra(mat_tmp);

#   endif

#endif

        }

        else if (mat.channels() == 4)
        {
            // Find best format if the formatHint can not be applied.

            format = findClosestFormat(formatHint);

            if (
                   (format != QImage::Format_RGB32)
                && (format != QImage::Format_ARGB32)
                && (format != QImage::Format_ARGB32_Premultiplied)

#if QT_VERSION >= 0x050200

                && (format != QImage::Format_RGBX8888)
                && (format != QImage::Format_RGBA8888)
                && (format != QImage::Format_RGBA8888_Premultiplied)

#endif

               )
            {

#if QT_VERSION >= 0x050200

                format = ((order == MCO_RGBA) ? QImage::Format_RGBA8888 : QImage::Format_ARGB32);

#else

                format = QImage::Format_ARGB32;

#endif

            }

            // Channel order required by the target QImage

            MatColorOrder requiredOrder = getColorOrderOfRGB32Format();

#if QT_VERSION >= 0x050200

            if (
                (formatHint == QImage::Format_RGBX8888) ||
                (formatHint == QImage::Format_RGBA8888) ||
                (formatHint == QImage::Format_RGBA8888_Premultiplied)
               )
            {
                requiredOrder = MCO_RGBA;
            }

#endif

            if (order != requiredOrder)
            {
                mat_adjustCn = adjustChannelsOrder(mat, order, requiredOrder);
            }
        }

        if (mat_adjustCn.empty())
        {
            mat_adjustCn = mat;
        }

        // Adjust mat depth if needed.

        cv::Mat mat_adjustDepth = mat_adjustCn;

        if (mat.depth() != CV_8U)
        {
            mat_adjustCn.convertTo(
                                   mat_adjustDepth,
                                   CV_8UC(mat_adjustCn.channels()),
                                   ((mat.depth() == CV_16U) ? (1 / 255.0) : 255.0)
                                  );
        }

        // Should we convert the image to the format specified by formatHint?

        QImage image = mat2Image_shared(mat_adjustDepth, format);

        if ((format == formatHint) || (formatHint == QImage::Format_Invalid))
        {
            return image.copy();
        }
        else
        {
            return image.convertToFormat(formatHint);
        }
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_QTOPENCVIMG_LOG) << "QtOpenCVImg::mat2Image: cv::Exception:" << e.what();

        return QImage();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_QTOPENCVIMG_LOG) << "QtOpenCVImg::mat2Image: Default exception from OpenCV";

        return QImage();
    }
}

cv::Mat QtOpenCVImg::image2Mat_shared(const QImage& img, MatColorOrder* const order)
{
    if (img.isNull())
    {
        return cv::Mat();
    }

    try
    {
        switch (img.format())
        {
            case QImage::Format_Indexed8:
            {
                break;
            }

#if QT_VERSION >= 0x040400

            case QImage::Format_RGB888:
            {
                if (order)
                {
                    *order = MCO_RGB;
                }

                break;
            }

#endif

            case QImage::Format_RGB32:
            case QImage::Format_ARGB32:
            case QImage::Format_ARGB32_Premultiplied:
            {
                if (order)
                {
                    *order = getColorOrderOfRGB32Format();
                }

                break;
            }

#if QT_VERSION >= 0x050200

            case QImage::Format_RGBX8888:
            case QImage::Format_RGBA8888:
            case QImage::Format_RGBA8888_Premultiplied:
            {
                if (order)
                {
                    *order = MCO_RGBA;
                }

                break;
            }

#endif

#if QT_VERSION >= 0x050500

            case QImage::Format_Alpha8:
            case QImage::Format_Grayscale8:
            {
                break;
            }

#endif

            default:
            {
                return cv::Mat();
            }
        }

        return cv::Mat(
                       img.height(),
                       img.width(),
                       CV_8UC(img.depth() / 8),
                       const_cast<uchar*>(img.bits()),
                       img.bytesPerLine()
                      );
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_QTOPENCVIMG_LOG) << "QtOpenCVImg::image2Mat_shared: cv::Exception:" << e.what();

        return cv::Mat();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_QTOPENCVIMG_LOG) << "QtOpenCVImg::image2Mat_shared: Default exception from OpenCV";

        return cv::Mat();
    }
}

QImage QtOpenCVImg::mat2Image_shared(const cv::Mat& mat, QImage::Format formatHint)
{
    Q_ASSERT((mat.type() == CV_8UC1) || (mat.type() == CV_8UC3) || (mat.type() == CV_8UC4));

    if (mat.empty())
    {
        return QImage();
    }

    try
    {
        // Adjust formatHint if needed.

        if (mat.type() == CV_8UC1)
        {
            if (
                   // cppcheck-suppress duplicateConditionalAssign
                   (formatHint != QImage::Format_Indexed8)

#if QT_VERSION >= 0x050500

                && (formatHint != QImage::Format_Alpha8)
                && (formatHint != QImage::Format_Grayscale8)

#endif

               )
            {
                // cppcheck-suppress duplicateConditionalAssign
                formatHint = QImage::Format_Indexed8;
            }

#if QT_VERSION >= 0x040400

        }

        else if (mat.type() == CV_8UC3)
        {
            formatHint = QImage::Format_RGB888;

#endif

        }

        else if (mat.type() == CV_8UC4)
        {
            if (   (formatHint != QImage::Format_RGB32)
                && (formatHint != QImage::Format_ARGB32)
                && (formatHint != QImage::Format_ARGB32_Premultiplied)

#if QT_VERSION >= 0x050200

                && (formatHint != QImage::Format_RGBX8888)
                && (formatHint != QImage::Format_RGBA8888)
                && (formatHint != QImage::Format_RGBA8888_Premultiplied)

#endif

               )
            {
                formatHint = QImage::Format_ARGB32;
            }
        }

        QImage img(mat.data, mat.cols, mat.rows, mat.step, formatHint);

        // Should we add directly support for user-customed-colorTable?

        if (formatHint == QImage::Format_Indexed8)
        {
            QVector<QRgb> colorTable;

            for (int i = 0 ; i < 256 ; ++i)
            {
                colorTable.append(qRgb(i, i, i));
            }

            img.setColorTable(colorTable);
        }

        return img;
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_QTOPENCVIMG_LOG) << "QtOpenCVImg::mat2Image_shared: cv::Exception:" << e.what();

        return QImage();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_QTOPENCVIMG_LOG) << "QtOpenCVImg::mat2Image_shared: Default exception from OpenCV";

        return QImage();
    }
}

} // namespace Digikam
