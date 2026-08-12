/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-12-09
 * Description : a tread-safe libraw Qt interface
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2007-2008 by Guillaume Castagnino <casta at xwing dot info>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "drawdecoder_p.h"

namespace Digikam
{

DRawDecoder::DRawDecoder()
    : d(new Private(this))
{
}

DRawDecoder::~DRawDecoder()
{
    cancel();
    delete d;
}

void DRawDecoder::cancel()
{
    m_cancel = true;
}

bool DRawDecoder::loadRawPreview(QImage& image, const QString& path)
{
    // In first, try to extract the embedded JPEG preview. Very fast.

    bool ret = loadEmbeddedPreview(image, path);

    if (ret)
    {
        return true;
    }

    // In second, decode and half size of RAW picture. More slow.

    return (loadHalfPreview(image, path));
}

bool DRawDecoder::loadEmbeddedPreview(QImage& image, const QString& path)
{
    QByteArray imgData;

    if (loadEmbeddedPreview(imgData, path))
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "Preview data size:" << imgData.size();

        if (image.loadFromData(imgData))
        {
            qCDebug(DIGIKAM_RAWENGINE_LOG) << "Using embedded RAW preview extraction";

            return true;
        }
    }

    qCDebug(DIGIKAM_RAWENGINE_LOG) << "Failed to load embedded RAW preview";

    return false;
}

bool DRawDecoder::loadEmbeddedPreview(QByteArray& imgData, const QString& path)
{
    QFileInfo fileInfo(path);
    QString   rawFilesExt = rawFiles();
    QString   ext         = fileInfo.suffix().toUpper();

    if (!fileInfo.exists() || ext.isEmpty() || !rawFilesExt.toUpper().contains(ext))
    {
        return false;
    }

    qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: loadEmbeddedPreview from" << path;

    LibRaw* const raw = new LibRaw;

#ifdef Q_OS_WIN

    int ret           = raw->open_file((const wchar_t*)path.utf16());

#else

    int ret           = raw->open_file(path.toUtf8().constData());

#endif

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run open_file: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

        return false;
    }

    return (Private::loadEmbeddedPreview(imgData, raw));
}

bool DRawDecoder::loadEmbeddedPreview(QByteArray& imgData, const QBuffer& buffer)
{
    LibRaw* const raw   = new LibRaw;
    QByteArray inData   = buffer.data();
    int ret             = raw->open_buffer(reinterpret_cast<void*>(inData.data()), (size_t)(inData.size()));

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run open_buffer: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

        return false;
    }

    return (Private::loadEmbeddedPreview(imgData, raw));
}

bool DRawDecoder::loadHalfPreview(QImage& image, const QString& path, bool rotate)
{
    QFileInfo fileInfo(path);
    QString   rawFilesExt = rawFiles();
    QString   ext         = fileInfo.suffix().toUpper();

    if (!fileInfo.exists() || ext.isEmpty() || !rawFilesExt.toUpper().contains(ext))
    {
        return false;
    }

    qCDebug(DIGIKAM_RAWENGINE_LOG) << "Try to use reduced RAW picture extraction";

    LibRaw* const raw                 = new LibRaw;
    raw->imgdata.params.use_auto_wb   = 1;         // Use automatic white balance.
    raw->imgdata.params.use_camera_wb = 1;         // Use camera white balance, if possible.
    raw->imgdata.params.half_size     = 1;         // Half-size color image (3x faster than -q).

#ifdef Q_OS_WIN

    int ret                           = raw->open_file((const wchar_t*)path.utf16());

#else

    int ret                           = raw->open_file(path.toUtf8().constData());

#endif

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run open_file: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

        return false;
    }


    if (!Private::loadHalfPreview(image, raw, rotate))
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "Failed to get half preview from LibRaw!";
        return false;
    }

    qCDebug(DIGIKAM_RAWENGINE_LOG) << "Using reduced RAW picture extraction for" << path;

    return true;
}

bool DRawDecoder::loadHalfPreview(QByteArray& imgData, const QString& path)
{
    QFileInfo fileInfo(path);
    QString   rawFilesExt = rawFiles();
    QString   ext         = fileInfo.suffix().toUpper();

    if (!fileInfo.exists() || ext.isEmpty() || !rawFilesExt.toUpper().contains(ext))
    {
        return false;
    }

    qCDebug(DIGIKAM_RAWENGINE_LOG) << "Try to use reduced RAW picture extraction";

    LibRaw* const raw = new LibRaw;

#ifdef Q_OS_WIN

    int ret           = raw->open_file((const wchar_t*)path.utf16());

#else

    int ret           = raw->open_file(path.toUtf8().constData());

#endif

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run dcraw_process: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

        return false;
    }

    QImage image;

    if (!Private::loadHalfPreview(image, raw))
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "DRawDecoder: failed to get half preview: " << libraw_strerror(ret);

        return false;
    }

    QBuffer buffer(&imgData);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPEG");

    return true;
}

bool DRawDecoder::loadHalfPreview(QByteArray& imgData, const QBuffer& inBuffer)
{
    LibRaw* const raw   = new LibRaw;
    QByteArray inData   = inBuffer.data();
    int ret             = raw->open_buffer(reinterpret_cast<void*>(inData.data()), (size_t)(inData.size()));

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run dcraw_make_mem_image: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

        return false;
    }

    QImage image;

    if (!Private::loadHalfPreview(image, raw))
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "DRawDecoder: failed to get half preview: " << libraw_strerror(ret);
        return false;
    }

    QBuffer buffer(&imgData);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPG");
    buffer.close();

    return true;
}

bool DRawDecoder::loadFullImage(QImage& image,
                                const QString& path,
                                const DRawDecoderSettings& settings)
{
    QFileInfo fileInfo(path);
    QString   rawFilesExt = rawFiles();
    QString   ext         = fileInfo.suffix().toUpper();

    if (!fileInfo.exists() || ext.isEmpty() || !rawFilesExt.toUpper().contains(ext))
    {
        return false;
    }

    qCDebug(DIGIKAM_RAWENGINE_LOG) << "Try to load full RAW picture...";

    QPointer<DRawDecoder> decoder(new DRawDecoder);
    QByteArray imgData;
    int width, height, rgbmax;
    DRawDecoderSettings prm = settings;
    prm.sixteenBitsImage    = false;
    bool ret                = decoder->decodeRAWImage(path, prm, imgData, width, height, rgbmax);

    if (!ret)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "Failed to load full RAW picture";

        return false;
    }

    uchar* sptr             = reinterpret_cast<unsigned char*>(imgData.data());
    uchar tmp8[2];

    // Set RGB color components.

    for (int i = 0 ; i < (width * height) ; ++i)
    {
        // Swap Red and Blue

        tmp8[0] = sptr[2];
        tmp8[1] = sptr[0];
        sptr[0] = tmp8[0];
        sptr[2] = tmp8[1];
        sptr   += 3;
    }

    image      = QImage(width, height, QImage::Format_ARGB32);
    uint* dptr = reinterpret_cast<uint*>(image.bits());
    sptr       = reinterpret_cast<unsigned char*>(imgData.data());

    for (int i = 0 ; i < (width * height) ; ++i)
    {
        *dptr++ = qRgba(sptr[2], sptr[1], sptr[0], 0xFF);
        sptr   += 3;
    }

    qCDebug(DIGIKAM_RAWENGINE_LOG) << "Load full RAW picture done";

    return true;
}

bool DRawDecoder::rawFileIdentify(DRawInfo& identify, const QString& path)
{
    QFileInfo fileInfo(path);
    QString rawFilesExt  = rawFiles();
    QString ext          = fileInfo.suffix().toUpper();
    identify.isDecodable = false;

    if (!fileInfo.exists() || ext.isEmpty() || !rawFilesExt.toUpper().contains(ext))
    {
        return false;
    }

    LibRaw* const raw = new LibRaw;

#ifdef Q_OS_WIN

    int ret           = raw->open_file((const wchar_t*)path.utf16());

#else

    int ret           = raw->open_file(path.toUtf8().constData());

#endif

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run open_file: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

        return false;
    }

    ret               = raw->adjust_sizes_info_only();

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run adjust_sizes_info_only: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

        return false;
    }

    Private::fillIndentifyInfo(raw, identify);
    raw->recycle();
    delete raw;

    return true;
}

// ----------------------------------------------------------------------------------

bool DRawDecoder::extractRAWData(const QString& filePath,
                                 QByteArray& rawData,
                                 DRawInfo& identify,
                                 unsigned int shotSelect)
{
    QFileInfo fileInfo(filePath);
    QString rawFilesExt  = rawFiles();
    QString ext          = fileInfo.suffix().toUpper();
    identify.isDecodable = false;

    if (!fileInfo.exists() || ext.isEmpty() || !rawFilesExt.toUpper().contains(ext))
    {
        return false;
    }

    if (m_cancel)
    {
        return false;
    }

    d->setProgress(0.1);

    LibRaw* const raw = new LibRaw;

    // Set progress call back function.

    raw->set_progress_handler(s_progressCallbackForLibRaw, d);
    raw->set_exifparser_handler(s_exifParserCallbackForLibRaw, d);

#ifdef Q_OS_WIN

    int ret           = raw->open_file((const wchar_t*)filePath.utf16());

#else

    int ret           = raw->open_file(filePath.toUtf8().constData());

#endif

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run open_file: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

        return false;
    }

    // cppcheck-suppress knownConditionTrueFalse
    if (m_cancel)
    {
        raw->recycle();
        delete raw;

        return false;
    }

    d->setProgress(0.3);

    raw->imgdata.params.output_bps     = 16;
    raw->imgdata.rawparams.shot_select = shotSelect;
    raw->imgdata.params.user_flip      = -1;
    ret                                = raw->unpack();

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run unpack: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

        return false;
    }

    // cppcheck-suppress knownConditionTrueFalse
    if (m_cancel)
    {
        raw->recycle();
        delete raw;

        return false;
    }

    d->setProgress(0.4);

    ret = raw->raw2image();

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run raw2image: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

        return false;
    }

    // cppcheck-suppress knownConditionTrueFalse
    if (m_cancel)
    {
        raw->recycle();
        delete raw;

        return false;
    }

    d->setProgress(0.6);

    Private::fillIndentifyInfo(raw, identify);

    // cppcheck-suppress knownConditionTrueFalse
    if (m_cancel)
    {
        raw->recycle();
        delete raw;

        return false;
    }

    d->setProgress(0.8);

    rawData = QByteArray();

    if (raw->imgdata.idata.filters == 0)
    {
        rawData.resize((int)((int)raw->imgdata.sizes.iwidth * (int)raw->imgdata.sizes.iheight * (int)raw->imgdata.idata.colors * sizeof(unsigned short)));

        unsigned short* output = reinterpret_cast<unsigned short*>(rawData.data());

        for (unsigned int row = 0 ; row < raw->imgdata.sizes.iheight ; ++row)
        {
            for (unsigned int col = 0 ; col < raw->imgdata.sizes.iwidth ; ++col)
            {
                for (int color = 0 ; color < raw->imgdata.idata.colors ; ++color)
                {
                    *output = raw->imgdata.image[raw->imgdata.sizes.iwidth*row + col][color];
                    output++;
                }
            }
        }
    }
    else
    {
        rawData.resize((int)((int)raw->imgdata.sizes.iwidth * (int)raw->imgdata.sizes.iheight * sizeof(unsigned short)));

        unsigned short* output = reinterpret_cast<unsigned short*>(rawData.data());

        for (unsigned int row = 0 ; row < raw->imgdata.sizes.iheight ; ++row)
        {
            for (unsigned int col = 0 ; col < raw->imgdata.sizes.iwidth ; ++col)
            {
                *output = raw->imgdata.image[raw->imgdata.sizes.iwidth*row + col][raw->COLOR(row, col)];
                output++;
            }
        }
    }

    raw->recycle();
    delete raw;
    d->setProgress(1.0);

    return true;
}

bool DRawDecoder::decodeHalfRAWImage(const QString& filePath,
                                     const DRawDecoderSettings& DRawDecoderSettings,
                                     QByteArray& imageData,
                                     int& width,
                                     int& height,
                                     int& rgbmax)
{
    m_decoderSettings                    = DRawDecoderSettings;
    m_decoderSettings.halfSizeColorImage = true;

    return (d->loadFromLibraw(filePath, imageData, width, height, rgbmax));
}

bool DRawDecoder::decodeRAWImage(const QString& filePath,
                                 const DRawDecoderSettings& DRawDecoderSettings,
                                 QByteArray& imageData,
                                 int& width,
                                 int& height,
                                 int& rgbmax)
{
    m_decoderSettings = DRawDecoderSettings;

    return (d->loadFromLibraw(filePath, imageData, width, height, rgbmax));
}

bool DRawDecoder::checkToCancelWaitingData()
{
    return m_cancel;
}

void DRawDecoder::setWaitingDataProgress(double)
{
}

QString DRawDecoder::rawFiles()
{
    return s_rawFileExtensions();
}

QStringList DRawDecoder::rawFilesList()
{
    QString string = rawFiles();

    return string.remove(QLatin1String("*.")).split(QLatin1Char(' '));
}

int DRawDecoder::rawFilesVersion()
{
    return s_rawFileExtensionsVersion();
}

QStringList DRawDecoder::supportedCamera()
{
    QStringList camera;
    const char** const list = LibRaw::cameraList();

    for (int i = 0 ; i < LibRaw::cameraCount() ; ++i)
    {
        camera.append(QString::fromUtf8(list[i]));
    }

    return camera;
}

QString DRawDecoder::librawVersion()
{
    QString simplified = QString::fromLatin1(LIBRAW_VERSION_STR)
                         .section(QLatin1Char('-'), 0, -2);

    if (simplified.isEmpty())
    {
        simplified = QString::fromLatin1(LIBRAW_VERSION_STR);
    }

    return simplified;
}

int DRawDecoder::librawUseGomp()
{

#ifdef LIBRAW_FORCE_OPENMP

    return true;

#else

#   ifdef LIBRAW_USE_OPENMP

    return true;

#   else

    return false;

#   endif

#endif

}

bool DRawDecoder::isRawFile(const QUrl& url)
{
    QString   rawFilesExt = rawFiles();
    QFileInfo fileInfo(url.toLocalFile());

    return (rawFilesExt.toUpper().contains(fileInfo.suffix().toUpper()));
}

} // namespace Digikam

#include "moc_drawdecoder.cpp"
