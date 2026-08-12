/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-10-11
 * Description : save image thread for scanned data
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "saveimgthread.h"

// Qt includes

#include <QDateTime>
#include <QScopedPointer>

// KDE includes

#include <ksanewidget.h>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "dmetadata.h"
#include "actionthreadbase.h"

using namespace Digikam;
using namespace KSaneIface;

namespace DigikamGenericDScannerPlugin
{

class Q_DECL_HIDDEN SaveImgThread::Private
{
public:

    Private() = default;

public:

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    QImage     imageData;

#elif KSANE_VERSION < QT_VERSION_CHECK(21,8,0)

    int        width        = 0;
    int        height       = 0;
    int        bytesPerLine = 0;
    int        frmt         = 0;

    QByteArray ksaneData;

#else

    QImage     imageData;

#endif

    QString    make;
    QString    model;
    QString    format;

    QUrl       newUrl;
};

SaveImgThread::SaveImgThread(QObject* const parent)
    : QThread(parent),
      d      (new Private)
{
}

SaveImgThread::~SaveImgThread()
{
    // wait for the thread to finish

    wait();

    delete d;
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

void SaveImgThread::setImageData(const QImage& imageData)
{
    d->imageData = imageData;
}

#elif KSANE_VERSION < QT_VERSION_CHECK(21,8,0)

void SaveImgThread::setImageData(const QByteArray& ksaneData, int width, int height,
                                 int bytesPerLine, int ksaneFormat)
{
    d->width        = width;
    d->height       = height;
    d->bytesPerLine = bytesPerLine;
    d->frmt         = ksaneFormat;
    d->ksaneData    = ksaneData;
}

#else

void SaveImgThread::setImageData(const QImage& imageData)
{
    d->imageData = imageData;
}

#endif

void SaveImgThread::setTargetFile(const QUrl& url, const QString& format)
{
    d->newUrl = url;
    d->format = format;
}

void SaveImgThread::setScannerModel(const QString& make, const QString& model)
{
    d->make  = make;
    d->model = model;
}

void SaveImgThread::run()
{
    ActionThreadBase::setCurrentThreadName(QLatin1String("SaveImgThread"));       // To customize thread name

    Q_EMIT signalProgress(d->newUrl, 10);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    bool sixteenBit   = ((d->imageData.format() == QImage::Format_RGBX64) ||
                         (d->imageData.format() == QImage::Format_Grayscale16));
    DImg img((uint)d->imageData.width(), (uint)d->imageData.height(), sixteenBit, false);
    int progress;

    if (!sixteenBit)
    {
        uchar* dst = img.bits();

        for (int h = 0 ; h < d->imageData.height() ; ++h)
        {
            for (int w = 0 ; w < d->imageData.width() ; ++w)
            {
                if      (d->imageData.format() == QImage::Format_RGB32)         // Color 8 bits
                {
                    const QRgb* rgbData = reinterpret_cast<QRgb*>(d->imageData.scanLine(h));
                    dst[0] = qBlue(rgbData[w]);     // Blue
                    dst[1] = qGreen(rgbData[w]);    // Green
                    dst[2] = qRed(rgbData[w]);      // Red
                    dst[3] = 0x00;                  // Alpha

                    dst   += 4;
                }
                else if (d->imageData.format() == QImage::Format_Grayscale8)    // Gray
                {
                    const uchar* grayScale = d->imageData.scanLine(h);
                    dst[0] = grayScale[w];          // Blue
                    dst[1] = grayScale[w];          // Green
                    dst[2] = grayScale[w];          // Red
                    dst[3] = 0x00;                  // Alpha

                    dst   += 4;
                }
                else if (d->imageData.format() == QImage::Format_Mono)          // Lineart
                {
                    const uchar* mono = d->imageData.scanLine(h);
                    const int index   = w / 8;
                    const int mod     = w % 8;

                    if (mono[index] & (1 << mod))
                    {
                        dst[0] = 0x00;              // Blue
                        dst[1] = 0x00;              // Green
                        dst[2] = 0x00;              // Red
                        dst[3] = 0x00;              // Alpha
                    }
                    else
                    {
                        dst[0] = 0xFF;              // Blue
                        dst[1] = 0xFF;              // Green
                        dst[2] = 0xFF;              // Red
                        dst[3] = 0x00;              // Alpha
                    }

                    dst       += 4;
                }
            }

            progress = 10 + (int)(((double)h * 50.0) / d->imageData.height());

            if (progress % 5 == 0)
            {
                Q_EMIT signalProgress(d->newUrl, progress);
            }
        }
    }
    else
    {
        unsigned short* dst = reinterpret_cast<unsigned short*>(img.bits());

        for (int h = 0 ; h < d->imageData.height() ; ++h)
        {
            for (int w = 0 ; w < d->imageData.width() ; ++w)
            {
                if      (d->imageData.format() == QImage::Format_RGBX64)        // Color 16 bits
                {
                    const QRgba64* rgbData = reinterpret_cast<QRgba64*>(d->imageData.scanLine(h));
                    dst[0] = rgbData[w].blue();     // Blue
                    dst[1] = rgbData[w].green();    // Green
                    dst[2] = rgbData[w].red();      // Red
                    dst[3] = 0x0000;                // Alpha

                    dst    += 4;
                }
                else if (d->imageData.format() == QImage::Format_Grayscale16)   // Gray16
                {
                    const unsigned short* grayScale = reinterpret_cast<unsigned short*>(d->imageData.scanLine(h));
                    dst[0] = grayScale[w];          // Blue
                    dst[1] = grayScale[w];          // Green
                    dst[2] = grayScale[w];          // Red
                    dst[3] = 0x0000;                // Alpha

                    dst   += 4;
                }
            }

            progress = 10 + (int)(((double)h * 50.0) / d->imageData.height());

            if ((progress % 5) == 0)
            {
                Q_EMIT signalProgress(d->newUrl, progress);
            }
        }
    }

#elif KSANE_VERSION < QT_VERSION_CHECK(21,8,0)

    bool sixteenBit   = ((d->frmt == KSaneWidget::FormatRGB_16_C) ||
                         (d->frmt == KSaneWidget::FormatGrayScale16));
    DImg img((uint)d->width, (uint)d->height, sixteenBit, false);
    int progress;

    if (!sixteenBit)
    {
        uchar* src = reinterpret_cast<uchar*>(d->ksaneData.data());
        uchar* dst = img.bits();

        for (int h = 0 ; h < d->height ; ++h)
        {
            for (int w = 0 ; w < d->width ; ++w)
            {
                if      (d->frmt == KSaneWidget::FormatRGB_8_C)     // Color 8 bits
                {
                    dst[0] = src[2];    // Blue
                    dst[1] = src[1];    // Green
                    dst[2] = src[0];    // Red
                    dst[3] = 0x00;      // Alpha

                    dst   += 4;
                    src   += 3;
                }
                else if (d->frmt == KSaneWidget::FormatGrayScale8)  // Gray
                {
                    dst[0] = src[0];    // Blue
                    dst[1] = src[0];    // Green
                    dst[2] = src[0];    // Red
                    dst[3] = 0x00;      // Alpha

                    dst   += 4;
                    src   += 1;
                }
                else if (d->frmt == KSaneWidget::FormatBlackWhite)  // Lineart
                {
                    for (int i = 0 ; i < 8 ; ++i)
                    {
                        if (*src & (1 << (7 - i)))
                        {
                            dst[0] = 0x00;    // Blue
                            dst[1] = 0x00;    // Green
                            dst[2] = 0x00;    // Red
                            dst[3] = 0x00;    // Alpha
                        }
                        else
                        {
                            dst[0] = 0xFF;    // Blue
                            dst[1] = 0xFF;    // Green
                            dst[2] = 0xFF;    // Red
                            dst[3] = 0x00;    // Alpha
                        }

                        dst       += 4;
                    }

                    src += 1;
                    w   += 7;
                }
            }

            progress = 10 + (int)(((double)h * 50.0) / d->height);

            if (progress % 5 == 0)
            {
                Q_EMIT signalProgress(d->newUrl, progress);
            }
        }
    }
    else
    {
        unsigned short* src = reinterpret_cast<unsigned short*>(d->ksaneData.data());
        unsigned short* dst = reinterpret_cast<unsigned short*>(img.bits());

        for (int h = 0 ; h < d->height ; ++h)
        {
            for (int w = 0 ; w < d->width ; ++w)
            {
                if      (d->frmt == KSaneWidget::FormatRGB_16_C)    // Color 16 bits
                {
                    dst[0] = src[2];    // Blue
                    dst[1] = src[1];    // Green
                    dst[2] = src[0];    // Red
                    dst[3] = 0x0000;    // Alpha

                    dst   += 4;
                    src   += 3;
                }
                else if (d->frmt == KSaneWidget::FormatGrayScale16) // Gray16
                {
                    dst[0] = src[0];    // Blue
                    dst[1] = src[0];    // Green
                    dst[2] = src[0];    // Red
                    dst[3] = 0x0000;    // Alpha

                    dst   += 4;
                    src   += 1;
                }
            }

            progress = 10 + (int)(((double)h * 50.0) / d->height);

            if ((progress % 5) == 0)
            {
                Q_EMIT signalProgress(d->newUrl, progress);
            }
        }
    }

#else

    bool sixteenBit   = ((d->imageData.format() == QImage::Format_RGBX64) ||
                         (d->imageData.format() == QImage::Format_Grayscale16));
    DImg img((uint)d->imageData.width(), (uint)d->imageData.height(), sixteenBit, false);
    int progress;

    if (!sixteenBit)
    {
        uchar* dst = img.bits();

        for (int h = 0 ; h < d->imageData.height() ; ++h)
        {
            for (int w = 0 ; w < d->imageData.width() ; ++w)
            {
                if      (d->imageData.format() == QImage::Format_RGB32)         // Color 8 bits
                {
                    const QRgb* rgbData = reinterpret_cast<QRgb*>(d->imageData.scanLine(h));
                    dst[0] = qBlue(rgbData[w]);     // Blue
                    dst[1] = qGreen(rgbData[w]);    // Green
                    dst[2] = qRed(rgbData[w]);      // Red
                    dst[3] = 0x00;                  // Alpha

                    dst   += 4;
                }
                else if (d->imageData.format() == QImage::Format_Grayscale8)    // Gray
                {
                    const uchar* grayScale = d->imageData.scanLine(h);
                    dst[0] = grayScale[w];          // Blue
                    dst[1] = grayScale[w];          // Green
                    dst[2] = grayScale[w];          // Red
                    dst[3] = 0x00;                  // Alpha

                    dst   += 4;
                }
                else if (d->imageData.format() == QImage::Format_Mono)          // Lineart
                {
                    const uchar* mono = d->imageData.scanLine(h);
                    const int index   = w / 8;
                    const int mod     = w % 8;

                    if (mono[index] & (1 << mod))
                    {
                        dst[0] = 0x00;              // Blue
                        dst[1] = 0x00;              // Green
                        dst[2] = 0x00;              // Red
                        dst[3] = 0x00;              // Alpha
                    }
                    else
                    {
                        dst[0] = 0xFF;              // Blue
                        dst[1] = 0xFF;              // Green
                        dst[2] = 0xFF;              // Red
                        dst[3] = 0x00;              // Alpha
                    }

                    dst       += 4;
                }
            }

            progress = 10 + (int)(((double)h * 50.0) / d->imageData.height());

            if (progress % 5 == 0)
            {
                Q_EMIT signalProgress(d->newUrl, progress);
            }
        }
    }
    else
    {
        unsigned short* dst = reinterpret_cast<unsigned short*>(img.bits());

        for (int h = 0 ; h < d->imageData.height() ; ++h)
        {
            for (int w = 0 ; w < d->imageData.width() ; ++w)
            {
                if      (d->imageData.format() == QImage::Format_RGBX64)        // Color 16 bits
                {
                    const QRgba64* rgbData = reinterpret_cast<QRgba64*>(d->imageData.scanLine(h));
                    dst[0] = rgbData[w].blue();     // Blue
                    dst[1] = rgbData[w].green();    // Green
                    dst[2] = rgbData[w].red();      // Red
                    dst[3] = 0x0000;                // Alpha

                    dst    += 4;
                }
                else if (d->imageData.format() == QImage::Format_Grayscale16)   // Gray16
                {
                    const unsigned short* grayScale = reinterpret_cast<unsigned short*>(d->imageData.scanLine(h));
                    dst[0] = grayScale[w];          // Blue
                    dst[1] = grayScale[w];          // Green
                    dst[2] = grayScale[w];          // Red
                    dst[3] = 0x0000;                // Alpha

                    dst   += 4;
                }
            }

            progress = 10 + (int)(((double)h * 50.0) / d->imageData.height());

            if ((progress % 5) == 0)
            {
                Q_EMIT signalProgress(d->newUrl, progress);
            }
        }
    }

#endif

    Q_EMIT signalProgress(d->newUrl, 60);

    bool success = img.save(d->newUrl.toLocalFile(), d->format);

    Q_EMIT signalProgress(d->newUrl, 80);

    if (!success)
    {
        Q_EMIT signalComplete(d->newUrl, success);

        return;
    }

    QScopedPointer<DMetadata> meta(new DMetadata(d->newUrl.toLocalFile()));
    meta->setExifTagString("Exif.Image.DocumentName", QLatin1String("Scanned Image")); // not i18n
    meta->setExifTagString("Exif.Image.Make",         d->make);
    meta->setXmpTagString("Xmp.tiff.Make",            d->make);
    meta->setExifTagString("Exif.Image.Model",        d->model);
    meta->setXmpTagString("Xmp.tiff.Model",           d->model);
    meta->setItemOrientation(DMetadata::ORIENTATION_NORMAL);
    meta->setItemColorWorkSpace(DMetadata::WORKSPACE_SRGB);

    Q_EMIT signalProgress(d->newUrl, 90);

    meta->applyChanges(true);

    Q_EMIT signalProgress(d->newUrl, 100);

    Q_EMIT signalComplete(d->newUrl, success);
}

} // namespace DigikamGenericDScannerPlugin

#include "moc_saveimgthread.cpp"
