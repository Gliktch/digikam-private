/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyproxygenerator.h"

// Qt includes

#include <QBuffer>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QLinearGradient>
#include <QPainter>
#include <QSet>

namespace Digikam
{

namespace
{

const QSize ProxyPixelSize(512, 512);
const QSize BlurSampleSize(24, 24);
const QSize MaximumDecodedSampleSize(96, 96);
constexpr qint64 MaximumSourceFileBytes = 1024LL * 1024LL * 1024LL;
constexpr qint64 MaximumSourcePixels    = 200LL * 1000LL * 1000LL;
constexpr int MaximumSourceDimension   = 65535;

QByteArray canonicalFormat(QByteArray format)
{
    format = format.toLower();

    if ((format == "jpg") || (format == "jpe"))
    {
        return QByteArray("jpeg");
    }

    if (format == "tif")
    {
        return QByteArray("tiff");
    }

    return format;
}

bool isAllowedSameFormat(const QByteArray& format)
{
    static const QSet<QByteArray> formats = {
        QByteArray("bmp"),
        QByteArray("jpeg"),
        QByteArray("png"),
        QByteArray("tiff"),
        QByteArray("webp")
    };

    return formats.contains(format);
}

bool writerSupports(const QByteArray& format)
{
    const QList<QByteArray> supported = QImageWriter::supportedImageFormats();

    for (const QByteArray& candidate : supported)
    {
        if (canonicalFormat(candidate) == format)
        {
            return true;
        }
    }

    return false;
}

bool isSafeSourcePath(const QString& path, QFileInfo* const safeInfo)
{
    if (!safeInfo || path.isEmpty() || !QDir::isAbsolutePath(path) ||
        (QDir::cleanPath(path) != path) || path.contains(QChar::Null))
    {
        return false;
    }

    const QFileInfo info(path);

    if (!info.isFile() || info.isSymLink() || (info.size() < 0) ||
        (info.canonicalFilePath() != info.absoluteFilePath()))
    {
        return false;
    }

    *safeInfo = info;

    return true;
}

bool isSingleFileName(const QString& fileName)
{
    return (!fileName.isEmpty() && !fileName.contains(QChar::Null) &&
            !fileName.contains(QLatin1Char('/')) &&
            !fileName.contains(QLatin1Char('\\')) &&
            (QFileInfo(fileName).fileName() == fileName));
}

bool isSafeSourceSize(const QSize& size)
{
    if (!size.isValid() || (size.width() <= 0) || (size.height() <= 0) ||
        (size.width() > MaximumSourceDimension) ||
        (size.height() > MaximumSourceDimension))
    {
        return false;
    }

    return (static_cast<qint64>(size.width()) * size.height()) <=
           MaximumSourcePixels;
}

QImage cleanRgbImage(const QImage& source)
{
    if (source.isNull())
    {
        return QImage();
    }

    QImage clean(source.size(), QImage::Format_RGB32);
    clean.fill(Qt::black);

    QPainter painter(&clean);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(0, 0, source);
    painter.end();

    return clean;
}

QImage genericPresentation()
{
    QImage image(ProxyPixelSize, QImage::Format_RGB32);
    image.fill(QColor(57, 62, 70));

    QPainter painter(&image);
    QLinearGradient gradient(0, 0, ProxyPixelSize.width(), ProxyPixelSize.height());
    gradient.setColorAt(0.0, QColor(76, 82, 91));
    gradient.setColorAt(0.5, QColor(54, 59, 67));
    gradient.setColorAt(1.0, QColor(39, 43, 50));
    painter.fillRect(image.rect(), gradient);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 10));
    painter.drawEllipse(QRect(-128, -96, 440, 440));
    painter.setBrush(QColor(0, 0, 0, 16));
    painter.drawEllipse(QRect(220, 190, 430, 430));
    painter.end();

    return image;
}

QImage blurredPresentation(const QImage& decoded)
{
    const QImage clean = cleanRgbImage(decoded);

    if (clean.isNull())
    {
        return QImage();
    }

    const QImage tiny = clean.scaled(BlurSampleSize,
                                     Qt::KeepAspectRatioByExpanding,
                                     Qt::SmoothTransformation);
    const int left = qMax(0, (tiny.width()  - BlurSampleSize.width())  / 2);
    const int top  = qMax(0, (tiny.height() - BlurSampleSize.height()) / 2);
    const QImage sample = tiny.copy(left, top,
                                    qMin(BlurSampleSize.width(),  tiny.width()),
                                    qMin(BlurSampleSize.height(), tiny.height()));
    QImage blurred = sample.scaled(ProxyPixelSize,
                                   Qt::IgnoreAspectRatio,
                                   Qt::SmoothTransformation);
    blurred = cleanRgbImage(blurred);

    QPainter painter(&blurred);
    painter.fillRect(blurred.rect(), QColor(45, 48, 54, 72));
    painter.end();

    return blurred;
}

bool encodeImage(const QImage& image,
                 const QByteArray& format,
                 QByteArray* encoded)
{
    if (!encoded || image.isNull() || format.isEmpty())
    {
        return false;
    }

    encoded->clear();
    QBuffer buffer(encoded);

    if (!buffer.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QImageWriter writer(&buffer, format);

    if ((format == "jpeg") || (format == "webp"))
    {
        writer.setQuality(82);
    }

    writer.setOptimizedWrite(true);

    return writer.write(image) && !encoded->isEmpty();
}

bool validateEncodedImage(const QByteArray& encoded,
                          const QByteArray& expectedFormat)
{
    QBuffer buffer;
    buffer.setData(encoded);

    if (!buffer.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QImageReader reader(&buffer);
    const QByteArray actualFormat = canonicalFormat(reader.format());

    if ((actualFormat != expectedFormat) || (reader.size() != ProxyPixelSize))
    {
        return false;
    }

    const QImage decoded = reader.read();

    return (!decoded.isNull() && (decoded.size() == ProxyPixelSize) &&
            decoded.textKeys().isEmpty());
}

} // namespace

bool PrivacyStillProxyResult::isValid() const
{
    if ((outcome == PrivacyStillProxyOutcome::Failed) ||
        (error != PrivacyStillProxyError::None) || encodedBytes.isEmpty() ||
        encodedFormat.isEmpty() || (sha256.size() != 32) ||
        (pixelSize != ProxyPixelSize))
    {
        return false;
    }

    if (outcome == PrivacyStillProxyOutcome::GeneratedSameFormat)
    {
        return (fallbackReason == PrivacyStillProxyFallbackReason::None);
    }

    return ((outcome == PrivacyStillProxyOutcome::GeneratedGenericFallback) &&
            (renderedPresentation == PrivacyStillProxyPresentation::Generic) &&
            (fallbackReason != PrivacyStillProxyFallbackReason::None) &&
            !sourcePixelsUsed);
}

QSize PrivacyStillProxyGenerator::fixedPixelSize()
{
    return ProxyPixelSize;
}

QByteArray PrivacyStillProxyGenerator::canonicalFormatForFileName(const QString& fileName)
{
    return canonicalFormat(QFileInfo(fileName).suffix().toLatin1());
}

bool PrivacyStillProxyGenerator::isSameFormatCandidate(const QString& fileName)
{
    return isAllowedSameFormat(canonicalFormatForFileName(fileName));
}

PrivacyStillProxyResult PrivacyStillProxyGenerator::generate(
    const PrivacyStillProxyRequest& request) const
{
    PrivacyStillProxyResult result;

    QFileInfo sourceInfo;

    if (!isSafeSourcePath(request.sourcePath, &sourceInfo) ||
        !isSingleFileName(request.publicFileName) ||
        ((request.presentation != PrivacyStillProxyPresentation::Generic) &&
         (request.presentation != PrivacyStillProxyPresentation::Blurred)))
    {
        return result;
    }

    const QByteArray publicFormat = canonicalFormatForFileName(request.publicFileName);
    QByteArray outputFormat = publicFormat;
    QImage presentation;

    const bool allowedFormat = isAllowedSameFormat(publicFormat);
    const bool supportedWriter = allowedFormat && writerSupports(publicFormat);

    if (!allowedFormat)
    {
        result.fallbackReason = PrivacyStillProxyFallbackReason::UnsupportedPublicFormat;
    }
    else if (!supportedWriter)
    {
        result.fallbackReason = PrivacyStillProxyFallbackReason::SameFormatEncoderUnavailable;
    }

    if ((request.presentation == PrivacyStillProxyPresentation::Blurred) &&
        supportedWriter)
    {
        QFile source(request.sourcePath);

        if (!source.open(QIODevice::ReadOnly))
        {
            result.fallbackReason =
                PrivacyStillProxyFallbackReason::SourceDecodeFailed;
        }
        else if (source.size() > MaximumSourceFileBytes)
        {
            result.fallbackReason =
                PrivacyStillProxyFallbackReason::SourceSafetyLimitExceeded;
        }
        else
        {
            QImageReader reader(&source);
            const QByteArray sourceFormat = canonicalFormat(reader.format());

            if (sourceFormat.isEmpty())
            {
                result.fallbackReason =
                    PrivacyStillProxyFallbackReason::SourceDecodeFailed;
            }
            else if (sourceFormat != publicFormat)
            {
                result.fallbackReason =
                    PrivacyStillProxyFallbackReason::SourceFormatMismatch;
            }
            else
            {
                reader.setAutoTransform(true);
                const QSize sourceSize = reader.size();

                if (!isSafeSourceSize(sourceSize))
                {
                    result.fallbackReason =
                        sourceSize.isValid()
                        ? PrivacyStillProxyFallbackReason::SourceSafetyLimitExceeded
                        : PrivacyStillProxyFallbackReason::SourceDecodeFailed;
                }
                else
                {
                    reader.setScaledSize(sourceSize.scaled(
                        MaximumDecodedSampleSize, Qt::KeepAspectRatio));
                    const QImage decoded = reader.read();

                    if (decoded.isNull() ||
                        (decoded.width() > MaximumDecodedSampleSize.width()) ||
                        (decoded.height() > MaximumDecodedSampleSize.height()))
                    {
                        result.fallbackReason =
                            PrivacyStillProxyFallbackReason::SourceDecodeFailed;
                    }
                    else
                    {
                        presentation = blurredPresentation(decoded);
                        result.sourcePixelsUsed = !presentation.isNull();
                    }
                }
            }
        }
    }

    if (presentation.isNull())
    {
        presentation = genericPresentation();
        result.renderedPresentation = PrivacyStillProxyPresentation::Generic;
        result.sourcePixelsUsed = false;

        if (!supportedWriter)
        {
            outputFormat = QByteArray("png");

            if (!writerSupports(outputFormat))
            {
                result.error = PrivacyStillProxyError::GenericEncoderMissing;
                return result;
            }

            result.outcome = PrivacyStillProxyOutcome::GeneratedGenericFallback;
        }
        else
        {
            result.outcome = (result.fallbackReason == PrivacyStillProxyFallbackReason::None)
                           ? PrivacyStillProxyOutcome::GeneratedSameFormat
                           : PrivacyStillProxyOutcome::GeneratedGenericFallback;

        }
    }
    else
    {
        result.outcome = PrivacyStillProxyOutcome::GeneratedSameFormat;
        result.renderedPresentation = PrivacyStillProxyPresentation::Blurred;
    }

    if (!encodeImage(presentation, outputFormat, &result.encodedBytes))
    {
        result.outcome = PrivacyStillProxyOutcome::Failed;
        result.error = PrivacyStillProxyError::EncodeFailed;
        return result;
    }

    if (!validateEncodedImage(result.encodedBytes, outputFormat))
    {
        result.outcome = PrivacyStillProxyOutcome::Failed;
        result.error = PrivacyStillProxyError::EncodedOutputInvalid;
        result.encodedBytes.clear();
        return result;
    }

    result.encodedFormat = outputFormat;
    result.pixelSize = presentation.size();
    result.sha256 = QCryptographicHash::hash(result.encodedBytes,
                                             QCryptographicHash::Sha256);
    result.error = PrivacyStillProxyError::None;

    return result;
}

} // namespace Digikam
