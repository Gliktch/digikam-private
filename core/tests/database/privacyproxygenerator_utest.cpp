/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QBuffer>
#include <QByteArrayView>
#include <QCryptographicHash>
#include <QFile>
#include <QImage>
#include <QColorSpace>
#include <QImageReader>
#include <QImageWriter>
#include <QTemporaryDir>
#include <QTest>

// Local includes

#include "privacyproxygenerator.h"

using namespace Digikam;

namespace
{

const QString SecretMetadata = QLatin1String("synthetic-private-metadata-7c3f");

QImage syntheticImage(const QColor& first, const QColor& second)
{
    QImage image(160, 96, QImage::Format_RGB32);
    image.fill(first);

    for (int y = 0 ; y < image.height() ; ++y)
    {
        for (int x = image.width() / 2 ; x < image.width() ; ++x)
        {
            image.setPixelColor(x, y, second);
        }
    }

    image.setText(QLatin1String("Comment"), SecretMetadata);
    image.setColorSpace(QColorSpace::SRgb);
    image.setDevicePixelRatio(2.0);
    image.setDotsPerMeterX(12345);
    image.setDotsPerMeterY(23456);
    image.setOffset(QPoint(17, 29));

    return image;
}

bool writeSynthetic(const QString& path,
                    const QByteArray& format,
                    const QColor& first,
                    const QColor& second)
{
    QImageWriter writer(path, format);
    return writer.write(syntheticImage(first, second));
}

bool writeOversizedPngHeader(const QString& path)
{
    // Valid PNG signature/IHDR CRC, deliberately impossible dimensions and no
    // pixel payload. The generator must reject dimensions before decoding.
    const QByteArray bytes = QByteArray::fromHex(
        "89504e470d0a1a0a"
        "0000000d494844520001117000000fa00802000000592958be"
        "0000000849444154789c030000000001480689d2"
        "0000000049454e44ae426082");

    QFile file(path);

    return (file.open(QIODevice::WriteOnly | QIODevice::NewOnly) &&
            (file.write(bytes) == bytes.size()) && file.flush());
}

QImage decode(const PrivacyStillProxyResult& result)
{
    QBuffer buffer;
    buffer.setData(result.encodedBytes);

    if (!buffer.open(QIODevice::ReadOnly))
    {
        return QImage();
    }

    QImageReader reader(&buffer, result.encodedFormat);
    return reader.read();
}

QImage decode(const PrivacyClearThumbnailResult& result)
{
    QBuffer buffer;
    buffer.setData(result.encodedBytes);

    if (!buffer.open(QIODevice::ReadOnly))
    {
        return QImage();
    }

    QImageReader reader(&buffer, result.encodedFormat);
    return reader.read();
}

QByteArray decodedPixelHash(QImage image)
{
    image = image.convertToFormat(QImage::Format_RGB32);
    QCryptographicHash hash(QCryptographicHash::Sha256);

    for (int y = 0 ; y < image.height() ; ++y)
    {
        hash.addData(QByteArrayView(
            reinterpret_cast<const char*>(image.constScanLine(y)),
            image.width() * 4));
    }

    return hash.result();
}

QByteArray canonicalWriterFormat(const QByteArray& extension)
{
    return PrivacyStillProxyGenerator::canonicalFormatForFileName(
        QString::fromLatin1("proxy.") + QString::fromLatin1(extension));
}

bool writerSupports(const QByteArray& format)
{
    for (const QByteArray& candidate : QImageWriter::supportedImageFormats())
    {
        if (canonicalWriterFormat(candidate) == format)
        {
            return true;
        }
    }

    return false;
}

QByteArray encodedFormat(const PrivacyStillProxyResult& result)
{
    QBuffer buffer;
    buffer.setData(result.encodedBytes);

    if (!buffer.open(QIODevice::ReadOnly))
    {
        return {};
    }

    QImageReader reader(&buffer);

    return canonicalWriterFormat(reader.format());
}

PrivacyStillProxyRequest requestFor(const QString& sourcePath,
                                    const QString& publicFileName,
                                    PrivacyStillProxyPresentation presentation)
{
    PrivacyStillProxyRequest request;
    request.sourcePath = sourcePath;
    request.publicFileName = publicFileName;
    request.presentation = presentation;
    return request;
}

} // namespace

class PrivacyProxyGeneratorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testGenericIsMetadataFreeAndContentIndependent();
    void testGenericSameFormatCandidatesAreDeterministic();
    void testBlurredUsesPixelsWithoutCopyingMetadata();
    void testClearThumbnailUsesPixelsWithoutCopyingMetadata();
    void testRawLikeFormatUsesPngFallback();
    void testMismatchedSourceUsesGenericFallback();
    void testMalformedOversizedAndSymlinkSourcesFailClosed();
    void testInvalidRequestFailsClosed();
};

void PrivacyProxyGeneratorTest::testGenericIsMetadataFreeAndContentIndependent()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString firstPath = directory.filePath(QLatin1String("first.png"));
    const QString secondPath = directory.filePath(QLatin1String("second.png"));
    QVERIFY(writeSynthetic(firstPath, "png", Qt::red, Qt::green));
    QVERIFY(writeSynthetic(secondPath, "png", Qt::blue, Qt::yellow));

    PrivacyStillProxyGenerator generator;
    const PrivacyStillProxyResult first = generator.generate(
        requestFor(firstPath, QLatin1String("first.png"),
                   PrivacyStillProxyPresentation::Generic));
    const PrivacyStillProxyResult second = generator.generate(
        requestFor(secondPath, QLatin1String("second.png"),
                   PrivacyStillProxyPresentation::Generic));

    QVERIFY(first.isValid());
    QVERIFY(second.isValid());
    QCOMPARE(first.outcome, PrivacyStillProxyOutcome::GeneratedSameFormat);
    QCOMPARE(first.renderedPresentation, PrivacyStillProxyPresentation::Generic);
    QCOMPARE(first.encodedFormat, QByteArray("png"));
    QVERIFY(!first.sourcePixelsUsed);
    QCOMPARE(first.encodedBytes, second.encodedBytes);
    QCOMPARE(first.sha256, second.sha256);
    QVERIFY(!first.encodedBytes.contains(SecretMetadata.toUtf8()));

    const QImage image = decode(first);
    QCOMPARE(image.size(), PrivacyStillProxyGenerator::fixedPixelSize());
    QVERIFY(image.textKeys().isEmpty());
    QVERIFY(!image.colorSpace().isValid());
    QCOMPARE(image.devicePixelRatio(), 1.0);
    QVERIFY(image.offset() != QPoint(17, 29));
    QVERIFY(image.dotsPerMeterX() != 12345);
    QVERIFY(image.dotsPerMeterY() != 23456);

    // This pins every version-1 generic presentation pixel. Adding a badge,
    // border or source-dependent artwork must be an explicit schema change.
    QCOMPARE(decodedPixelHash(image).toHex(),
             QByteArray("5a70766bd0b8e4a75e3cf37de9343157f392e2e68ae33e094b5e4c25a1b93bef"));
}

void PrivacyProxyGeneratorTest::testGenericSameFormatCandidatesAreDeterministic()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString firstPath = directory.filePath(QLatin1String("first.png"));
    const QString secondPath = directory.filePath(QLatin1String("second.png"));
    QVERIFY(writeSynthetic(firstPath, "png", Qt::red, Qt::green));
    QVERIFY(writeSynthetic(secondPath, "png", Qt::blue, Qt::yellow));

    const QList<QByteArray> extensions = {
        QByteArray("bmp"), QByteArray("jpg"), QByteArray("png"),
        QByteArray("tif"), QByteArray("webp")
    };
    PrivacyStillProxyGenerator generator;
    int testedFormats = 0;

    for (const QByteArray& extension : extensions)
    {
        const QByteArray format = canonicalWriterFormat(extension);

        if (!writerSupports(format))
        {
            continue;
        }

        const QString publicName = QString::fromLatin1("proxy.") +
                                   QString::fromLatin1(extension);
        const auto first = generator.generate(requestFor(
            firstPath, publicName, PrivacyStillProxyPresentation::Generic));
        const auto second = generator.generate(requestFor(
            secondPath, publicName, PrivacyStillProxyPresentation::Generic));

        QVERIFY2(first.isValid(), format.constData());
        QVERIFY2(second.isValid(), format.constData());
        QCOMPARE(first.outcome, PrivacyStillProxyOutcome::GeneratedSameFormat);
        QCOMPARE(first.encodedFormat, format);
        QCOMPARE(encodedFormat(first), format);
        QCOMPARE(first.encodedBytes, second.encodedBytes);
        QCOMPARE(first.sha256, second.sha256);
        ++testedFormats;
    }

    QVERIFY(testedFormats >= 3);
}

void PrivacyProxyGeneratorTest::testBlurredUsesPixelsWithoutCopyingMetadata()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QLatin1String("source.png"));
    QVERIFY(writeSynthetic(path, "png", QColor(220, 30, 20), QColor(15, 40, 220)));

    PrivacyStillProxyGenerator generator;
    const PrivacyStillProxyResult blurred = generator.generate(
        requestFor(path, QLatin1String("source.png"),
                   PrivacyStillProxyPresentation::Blurred));
    const PrivacyStillProxyResult generic = generator.generate(
        requestFor(path, QLatin1String("source.png"),
                   PrivacyStillProxyPresentation::Generic));

    QVERIFY(blurred.isValid());
    QCOMPARE(blurred.outcome, PrivacyStillProxyOutcome::GeneratedSameFormat);
    QCOMPARE(blurred.renderedPresentation, PrivacyStillProxyPresentation::Blurred);
    QVERIFY(blurred.sourcePixelsUsed);
    QVERIFY(blurred.encodedBytes != generic.encodedBytes);
    QVERIFY(!blurred.encodedBytes.contains(SecretMetadata.toUtf8()));

    const QImage image = decode(blurred);
    QCOMPARE(image.size(), PrivacyStillProxyGenerator::fixedPixelSize());
    QVERIFY(image.textKeys().isEmpty());
    QVERIFY(!image.colorSpace().isValid());
    QCOMPARE(image.devicePixelRatio(), 1.0);
    QVERIFY(image.offset() != QPoint(17, 29));
    QVERIFY(image.dotsPerMeterX() != 12345);
    QVERIFY(image.dotsPerMeterY() != 23456);
}

void PrivacyProxyGeneratorTest::testClearThumbnailUsesPixelsWithoutCopyingMetadata()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString firstPath = directory.filePath(QLatin1String("first.png"));
    const QString secondPath = directory.filePath(QLatin1String("second.png"));
    QVERIFY(writeSynthetic(firstPath, "png", QColor(220, 30, 20),
                           QColor(15, 40, 220)));
    QVERIFY(writeSynthetic(secondPath, "png", Qt::green, Qt::yellow));

    PrivacyStillProxyGenerator generator;
    const PrivacyClearThumbnailResult first =
        generator.generateClearThumbnail(firstPath);
    const PrivacyClearThumbnailResult second =
        generator.generateClearThumbnail(secondPath);

    QVERIFY(first.isValid());
    QVERIFY(second.isValid());
    QCOMPARE(first.encodedFormat, QByteArray("jpeg"));
    QVERIFY(first.encodedBytes != second.encodedBytes);
    QVERIFY(first.sha256 != second.sha256);
    QVERIFY(!first.encodedBytes.contains(SecretMetadata.toUtf8()));
    QCOMPARE(first.sha256,
             QCryptographicHash::hash(first.encodedBytes,
                                      QCryptographicHash::Sha256));

    const QImage image = decode(first);
    QCOMPARE(image.size(), PrivacyStillProxyGenerator::fixedPixelSize());
    QVERIFY(image.textKeys().isEmpty());
    QVERIFY(!image.colorSpace().isValid());
    QCOMPARE(image.devicePixelRatio(), 1.0);
    QVERIFY(image.offset() != QPoint(17, 29));
    QVERIFY(image.dotsPerMeterX() != 12345);
    QVERIFY(image.dotsPerMeterY() != 23456);

    const PrivacyClearThumbnailResult missing =
        generator.generateClearThumbnail(directory.filePath(
            QLatin1String("missing.png")));
    QVERIFY(!missing.isValid());
    QCOMPARE(missing.error, PrivacyClearThumbnailError::InvalidSource);
}

void PrivacyProxyGeneratorTest::testRawLikeFormatUsesPngFallback()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QLatin1String("synthetic.nef"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write("synthetic-raw-like-bytes", 24), 24LL);
    file.close();

    PrivacyStillProxyGenerator generator;
    const PrivacyStillProxyResult result = generator.generate(
        requestFor(path, QLatin1String("synthetic.nef"),
                   PrivacyStillProxyPresentation::Blurred));

    QVERIFY(result.isValid());
    QCOMPARE(result.outcome, PrivacyStillProxyOutcome::GeneratedGenericFallback);
    QCOMPARE(result.fallbackReason,
             PrivacyStillProxyFallbackReason::UnsupportedPublicFormat);
    QCOMPARE(result.renderedPresentation, PrivacyStillProxyPresentation::Generic);
    QCOMPARE(result.encodedFormat, QByteArray("png"));
    QVERIFY(!result.sourcePixelsUsed);
    QCOMPARE(decode(result).size(), PrivacyStillProxyGenerator::fixedPixelSize());

    const PrivacyStillProxyResult genericPng = generator.generate(
        requestFor(path, QLatin1String("generic.png"),
                   PrivacyStillProxyPresentation::Generic));
    QVERIFY(genericPng.isValid());
    QCOMPARE(result.encodedBytes, genericPng.encodedBytes);
}

void PrivacyProxyGeneratorTest::testMismatchedSourceUsesGenericFallback()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QLatin1String("source.png"));
    QVERIFY(writeSynthetic(path, "png", Qt::cyan, Qt::magenta));

    PrivacyStillProxyGenerator generator;
    const PrivacyStillProxyResult result = generator.generate(
        requestFor(path, QLatin1String("public.jpg"),
                   PrivacyStillProxyPresentation::Blurred));

    QVERIFY(result.isValid());
    QCOMPARE(result.outcome, PrivacyStillProxyOutcome::GeneratedGenericFallback);
    QCOMPARE(result.fallbackReason,
             PrivacyStillProxyFallbackReason::SourceFormatMismatch);
    QCOMPARE(result.encodedFormat, QByteArray("jpeg"));
    QCOMPARE(encodedFormat(result), QByteArray("jpeg"));
    QVERIFY(!result.sourcePixelsUsed);
}

void PrivacyProxyGeneratorTest::testMalformedOversizedAndSymlinkSourcesFailClosed()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    PrivacyStillProxyGenerator generator;

    const QString malformedPath = directory.filePath(QLatin1String("malformed.png"));
    QFile malformed(malformedPath);
    QVERIFY(malformed.open(QIODevice::WriteOnly));
    const QByteArray malformedBytes = QByteArray::fromHex(
        "89504e470d0a1a0a0000000d494844520000000100000001");
    QCOMPARE(malformed.write(malformedBytes), qint64(malformedBytes.size()));
    malformed.close();

    const auto malformedResult = generator.generate(requestFor(
        malformedPath, QLatin1String("malformed.png"),
        PrivacyStillProxyPresentation::Blurred));
    QVERIFY(malformedResult.isValid());
    QCOMPARE(malformedResult.outcome,
             PrivacyStillProxyOutcome::GeneratedGenericFallback);
    QCOMPARE(malformedResult.fallbackReason,
             PrivacyStillProxyFallbackReason::SourceDecodeFailed);
    QCOMPARE(malformedResult.encodedFormat, QByteArray("png"));
    QVERIFY(!malformedResult.sourcePixelsUsed);

    const QString oversizedPath = directory.filePath(QLatin1String("oversized.png"));
    QVERIFY(writeSynthetic(oversizedPath, "png", Qt::black, Qt::white));
    QFile oversized(oversizedPath);
    QVERIFY(oversized.open(QIODevice::ReadWrite));
    QVERIFY(oversized.resize((1024LL * 1024LL * 1024LL) + 1));
    oversized.close();

    const auto oversizedResult = generator.generate(requestFor(
        oversizedPath, QLatin1String("oversized.png"),
        PrivacyStillProxyPresentation::Blurred));
    QVERIFY(oversizedResult.isValid());
    QCOMPARE(oversizedResult.outcome,
             PrivacyStillProxyOutcome::GeneratedGenericFallback);
    QCOMPARE(oversizedResult.fallbackReason,
             PrivacyStillProxyFallbackReason::SourceSafetyLimitExceeded);
    QCOMPARE(oversizedResult.encodedFormat, QByteArray("png"));
    QVERIFY(!oversizedResult.sourcePixelsUsed);

    const QString oversizedDimensionsPath =
        directory.filePath(QLatin1String("oversized-dimensions.png"));
    QVERIFY(writeOversizedPngHeader(oversizedDimensionsPath));
    const auto oversizedDimensionsResult = generator.generate(requestFor(
        oversizedDimensionsPath, QLatin1String("oversized-dimensions.png"),
        PrivacyStillProxyPresentation::Blurred));
    QVERIFY(oversizedDimensionsResult.isValid());
    QCOMPARE(oversizedDimensionsResult.outcome,
             PrivacyStillProxyOutcome::GeneratedGenericFallback);
    QCOMPARE(oversizedDimensionsResult.fallbackReason,
             PrivacyStillProxyFallbackReason::SourceSafetyLimitExceeded);
    QCOMPARE(oversizedDimensionsResult.encodedFormat, QByteArray("png"));
    QVERIFY(!oversizedDimensionsResult.sourcePixelsUsed);

    const QString sourcePath = directory.filePath(QLatin1String("source.png"));
    QVERIFY(writeSynthetic(sourcePath, "png", Qt::cyan, Qt::magenta));
    const QString symlinkPath = directory.filePath(QLatin1String("source-link.png"));
    QVERIFY(QFile::link(sourcePath, symlinkPath));

    const auto symlinkResult = generator.generate(requestFor(
        symlinkPath, QLatin1String("source-link.png"),
        PrivacyStillProxyPresentation::Blurred));
    QVERIFY(!symlinkResult.isValid());
    QCOMPARE(symlinkResult.error, PrivacyStillProxyError::InvalidRequest);
    QVERIFY(symlinkResult.encodedBytes.isEmpty());
}

void PrivacyProxyGeneratorTest::testInvalidRequestFailsClosed()
{
    PrivacyStillProxyGenerator generator;
    const PrivacyStillProxyResult result = generator.generate(
        requestFor(QLatin1String("/synthetic/missing.png"),
                   QLatin1String("missing.png"),
                   PrivacyStillProxyPresentation::Generic));

    QVERIFY(!result.isValid());
    QCOMPARE(result.outcome, PrivacyStillProxyOutcome::Failed);
    QCOMPARE(result.error, PrivacyStillProxyError::InvalidRequest);
    QVERIFY(result.encodedBytes.isEmpty());

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QLatin1String("source.png"));
    QVERIFY(writeSynthetic(path, "png", Qt::red, Qt::green));
    const auto pathLikePublicName = generator.generate(
        requestFor(path, QLatin1String("nested/source.png"),
                   PrivacyStillProxyPresentation::Generic));
    QVERIFY(!pathLikePublicName.isValid());
    QCOMPARE(pathLikePublicName.error, PrivacyStillProxyError::InvalidRequest);
}

QTEST_GUILESS_MAIN(PrivacyProxyGeneratorTest)

#include "privacyproxygenerator_utest.moc"
