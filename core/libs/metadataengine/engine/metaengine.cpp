/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface
 *               Exiv2: www.exiv2.org
 *               Exif : www.exif.org/Exif2-2.PDF
 *               Iptc : www.iptc.org/std/IIM/4.1/specification/IIMV4.1.pdf
 *               Xmp  : www.adobe.com/devnet/xmp/pdfs/xmp_specification.pdf
 *                      www.iptc.org/std/Iptc4xmpCore/1.0/specification/Iptc4xmpCore_1.0-spec-XMPSchema_8.pdf
 *               Paper: www.metadataworkinggroup.com/pdf/mwg_guidance.pdf
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metaengine_p.h"
#include "metaengine_data_p.h"

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace Digikam
{

MetaEngine::MetaEngine()
    : d(new Private(this))
{
}

MetaEngine::MetaEngine(const MetaEngine& metadata)
    : d(new Private(this))
{
    d->copyPrivateData(metadata.d);
}

MetaEngine::MetaEngine(const MetaEngineData& data)
    : d(new Private(this))
{
    setData(data);
}

MetaEngine::MetaEngine(const QString& filePath)
    : d(new Private(this))
{
    // NOTE: use dynamic binding as this virtual method can be re-implemented in derived classes.

    this->load(filePath);
}

MetaEngine::~MetaEngine()
{
    delete d;
}

MetaEngine& MetaEngine::operator=(const MetaEngine& metadata)
{
    d->copyPrivateData(metadata.d);

    return *this;
}

//-- Static methods ----------------------------------------------

bool MetaEngine::initializeExiv2()
{

#ifdef _XMP_SUPPORT_

    if (!Exiv2::XmpParser::initialize())
    {
        return false;
    }

    // The spaces at the end of the URIs are absolutely necessary.

    registerXmpNameSpace(QLatin1String("http://ns.google.com/photos/1.0/container/ "),            // krazy:exclude=insecurenet
                         QLatin1String("Container"));

    registerXmpNameSpace(QLatin1String("http://ns.google.com/photos/1.0/container/  "),           // krazy:exclude=insecurenet
                         QLatin1String("Container_1_"));

    registerXmpNameSpace(QLatin1String("http://ns.google.com/photos/1.0/container/   "),          // krazy:exclude=insecurenet
                         QLatin1String("GContainer"));

    registerXmpNameSpace(QLatin1String("http://ns.google.com/photos/1.0/container/item/ "),       // krazy:exclude=insecurenet
                         QLatin1String("Item"));

    registerXmpNameSpace(QLatin1String("http://ns.google.com/photos/1.0/container/item/  "),      // krazy:exclude=insecurenet
                         QLatin1String("Item_1_"));

    registerXmpNameSpace(QLatin1String("http://ns.google.com/photos/1.0/container/item/   "),     // krazy:exclude=insecurenet
                         QLatin1String("GItem"));

    registerXmpNameSpace(QLatin1String("http://ns.google.com/photos/1.0/container/item/    "),    // krazy:exclude=insecurenet
                         QLatin1String("GContainerItem"));

    registerXmpNameSpace(QLatin1String("http://ns.google.com/photos/1.0/camera/ "),               // krazy:exclude=insecurenet
                         QLatin1String("Camera"));

    registerXmpNameSpace(QLatin1String("http://ns.google.com/photos/1.0/camera/  "),              // krazy:exclude=insecurenet
                         QLatin1String("GCamera"));

    registerXmpNameSpace(QLatin1String("http://ns.adobe.com/hdr-gain-map/1.0/"),                  // krazy:exclude=insecurenet
                         QLatin1String("hdrgm"));

    registerXmpNameSpace(QLatin1String("http://ns.apple.com/faceinfo/1.0/"),                      // krazy:exclude=insecurenet
                         QLatin1String("apple-fi"));

    registerXmpNameSpace(QLatin1String("http://ns.excire.com/foto/1.0/"),                         // krazy:exclude=insecurenet
                         QLatin1String("excire"));

    registerXmpNameSpace(QLatin1String("http://ns.useplus.org/ldf/xmp/1.0/ImageCreator/ "),       // krazy:exclude=insecurenet
                         QLatin1String("plus_1_"));

    registerXmpNameSpace(QLatin1String("http://ns.useplus.org/ldf/xmp/1.0/ImageCreator/  "),      // krazy:exclude=insecurenet
                         QLatin1String("plus_1__1_"));

    /**
     * It cleans up memory used by Adobe XMP SDK automatically at application exit.
     * See Bug #166424 for details.
     */
    ::atexit(Exiv2::XmpParser::terminate);

#endif // _XMP_SUPPORT_

#if EXIV2_TEST_VERSION(0,28,3) && defined EXV_ENABLE_BMFF

    s_metaEngineSupportBmff = true;

#elif EXIV2_TEST_VERSION(0,27,4) && defined EXV_ENABLE_BMFF

    // For Base Media File Format (aka CR3, HEIF, HEIC, and AVIF)

    s_metaEngineSupportBmff = Exiv2::enableBMFF(true);

#endif

    return true;
}

bool MetaEngine::supportBmff()
{
    return s_metaEngineSupportBmff;
}

bool MetaEngine::supportXmp()
{

#ifdef _XMP_SUPPORT_

    return true;

#else

    return false;

#endif // _XMP_SUPPORT_

}

bool MetaEngine::supportJpegXL()
{

#ifdef EXV_HAVE_BROTLI

    return true;

#else

    return false;

#endif // EXV_HAVE_BROTLI

}

bool MetaEngine::supportMetadataWriting(const QString& /*typeMime*/)
{
/* FIXME : use Exiv2 API to return right writings support
    if      (typeMime == QLatin1String("image/jpeg"))
    {
        return true;
    }
    else if (typeMime == QLatin1String("image/tiff"))
    {
        return true;
    }
    else if (typeMime == QLatin1String("image/png"))
    {
        return true;
    }
    else if (typeMime == QLatin1String("image/jp2"))
    {
        return true;
    }
    else if (typeMime == QLatin1String("image/x-raw"))
    {
        return true;
    }
    else if (typeMime == QLatin1String("image/pgf"))
    {
        return true;
    }
*/
    return true;
}

QString MetaEngine::Exiv2Version()
{

#if EXIV2_TEST_VERSION(0,27,0)

    return QString::fromStdString(Exiv2::versionString());

#else

    return QLatin1String(Exiv2::version());

#endif

}

//-- General methods ----------------------------------------------

MetaEngineData MetaEngine::data() const
{
    MetaEngineData data;
    data.d = d->data;

    return data;
}

void MetaEngine::setData(const MetaEngineData& data)
{
    if (data.d)
    {
        d->data = data.d;
    }
    else
    {
        // MetaEngineData can have a null pointer,
        // but we never want a null pointer in Private.

        d->data->clear();
    }
}

bool MetaEngine::loadFromData(const QByteArray& imgData)
{
    if (imgData.isEmpty())
    {
        return false;
    }

    QMutexLocker lock(&s_metaEngineMutex);

    s_metaEngineWarnOrError = false;

    try
    {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(
            reinterpret_cast<Exiv2::byte*>(const_cast<char*>(imgData.data())), imgData.size());

        d->filePath.clear();
        image->readMetadata();

        // Size and mimetype ---------------------------------

        d->pixelSize = QSize(image->pixelWidth(), image->pixelHeight());
        d->mimeType  = QString::fromStdString(image->mimeType());

        // Image comments ---------------------------------

        d->itemComments()  = image->comment();

        // Exif metadata ----------------------------------

        d->exifMetadata()  = image->exifData();

        // Iptc metadata ----------------------------------

        d->iptcMetadata()  = image->iptcData();

#ifdef _XMP_SUPPORT_

        // Xmp metadata -----------------------------------

        d->xmpMetadata()   = image->xmpData();

#endif // _XMP_SUPPORT_

        // Exif byte order  -------------------------------

        d->exifByteOrder() = image->byteOrder();

        // ICC Profile ------------------------------------

        if (!getExifTagString("Exif.Image.Make").contains(QLatin1String("APPLE"), Qt::CaseInsensitive))
        {

#if EXIV2_TEST_VERSION(0,27,99)

            d->iccProfileBuf() = image->iccProfile();

#else

            d->iccProfileBuf() = *(image->iccProfile());

#endif

        }

        if (s_metaEngineWarnOrError)
        {
            d->itemComments().clear();
            d->exifMetadata().clear();
            d->iptcMetadata().clear();

#ifdef _XMP_SUPPORT_

            d->xmpMetadata().clear();

#endif // _XMP_SUPPORT_

            return false;
        }

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot load metadata with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::loadFromDataAndMerge(const QByteArray& imgData, const QStringList& exclude)
{
    if (imgData.isEmpty())
    {
        return false;
    }

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(
            reinterpret_cast<Exiv2::byte*>(const_cast<char*>(imgData.data())), imgData.size());

        d->filePath.clear();
        image->readMetadata();

        // Size and mimetype ---------------------------------

        d->pixelSize = QSize(image->pixelWidth(), image->pixelHeight());
        d->mimeType  = QString::fromStdString(image->mimeType());

        // Exif metadata ----------------------------------

        for (const QString& exTag : EXIV2_AS_CONST(exclude))
        {
            Exiv2::ExifKey exifKey(exTag.toLatin1().constData());
            Exiv2::ExifData::iterator it1 = image->exifData().findKey(exifKey);
            Exiv2::ExifData::iterator it2 = d->exifMetadata().findKey(exifKey);

            if ((it1 != image->exifData().end()) && (it2 != d->exifMetadata().end()))
            {
                image->exifData().erase(it1);
            }
        }

        ExifMetaEngineMergeHelper exifHelper;
        exifHelper.mergeAll(image->exifData(), d->exifMetadata());

        // Iptc metadata ----------------------------------

        IptcMetaEngineMergeHelper iptcHelper;
        iptcHelper.mergeAll(image->iptcData(), d->iptcMetadata());

#ifdef _XMP_SUPPORT_

        // Xmp metadata -----------------------------------

        XmpMetaEngineMergeHelper xmpHelper;
        xmpHelper.mergeAll(image->xmpData(), d->xmpMetadata());

#endif // _XMP_SUPPORT_

        // Exif byte order  -------------------------------

        d->exifByteOrder() = image->byteOrder();

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot load and merge metadata with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::isEmpty() const
{
    if (!hasComments() && !hasExif() && !hasIptc() && !hasXmp())
    {
        return true;
    }

    return false;
}

QSize MetaEngine::getPixelSize() const
{
    return d->pixelSize;
}

QString MetaEngine::getMimeType() const
{
    return d->mimeType;
}

void MetaEngine::setReadWithExifTool(const bool on)
{
    d->readWithExifTool = on;
}

bool MetaEngine::readWithExifTool() const
{
    return d->readWithExifTool;
}

void MetaEngine::setWriteWithExifTool(const bool on)
{
    d->writeWithExifTool = on;
}

bool MetaEngine::writeWithExifTool() const
{
    return d->writeWithExifTool;
}

void MetaEngine::setWriteRawFiles(const bool on)
{
    d->writeRawFiles = on;
}

bool MetaEngine::writeRawFiles() const
{
    return d->writeRawFiles;
}

void MetaEngine::setWriteDngFiles(const bool on)
{
    d->writeDngFiles = on;
}

bool MetaEngine::writeDngFiles() const
{
    return d->writeDngFiles;
}

void MetaEngine::setUseXMPSidecar4Reading(const bool on)
{
    d->useXMPSidecar4Reading = on;
}

bool MetaEngine::useXMPSidecar4Reading() const
{
    return d->useXMPSidecar4Reading;
}

void MetaEngine::setUseCompatibleFileName(const bool on)
{
    d->useCompatibleFileName = on;
}

bool MetaEngine::useCompatibleFileName() const
{
    return d->useCompatibleFileName;
}

void MetaEngine::setMetadataWritingMode(const int mode)
{
    d->metadataWritingMode = mode;
}

int MetaEngine::metadataWritingMode() const
{
    return d->metadataWritingMode;
}

void MetaEngine::setUpdateFileTimeStamp(bool on)
{
    d->updateFileTimeStamp = on;
}

bool MetaEngine::updateFileTimeStamp() const
{
    return d->updateFileTimeStamp;
}

bool MetaEngine::setProgramId() const
{
    QString version(digiKamVersion());
    QLatin1String software("digiKam");

    return setItemProgramId(software, version);
}

} // namespace Digikam

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif
