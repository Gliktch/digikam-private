/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : digiKam 8/16 bits image management API.
 *               Metadata operations.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimg_p.h"

namespace Digikam
{

QByteArray DImg::getUniqueHash()
{
    if (hasAttribute(QLatin1String("uniqueHash")))
    {
        return attribute(QLatin1String("uniqueHash")).toByteArray();
    }

    if (!hasAttribute(QLatin1String("originalFilePath")))
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "DImg::getUniqueHash called without originalFilePath property set!";
        return QByteArray();
    }

    QString filePath = attribute(QLatin1String("originalFilePath")).toString();

    if (filePath.isEmpty())
    {
        return QByteArray();
    }

    FileReadLocker lock(filePath);

    QScopedPointer<DMetadata> meta(new DMetadata(getMetadata()));
    QByteArray ba   = meta->getExifEncoded();

    QByteArray hash = DImg::createUniqueHash(filePath, ba);
    setAttribute(QLatin1String("uniqueHash"), hash);

    return hash;
}

QByteArray DImg::getUniqueHash(const QString& filePath)
{
    QScopedPointer<DMetadata> meta(new DMetadata(filePath));
    QByteArray ba = meta->getExifEncoded();

    return DImg::createUniqueHash(filePath, ba);
}

QByteArray DImg::getUniqueHashVersion(int version)
{
    const QString uniqueHash(QString::fromLatin1("uniqueHashV%1").arg(version));

    if (hasAttribute(uniqueHash))
    {
        return attribute(uniqueHash).toByteArray();
    }

    if (!hasAttribute(QLatin1String("originalFilePath")))
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "DImg::getUniqueHashVersion called without "
                                       "originalFilePath property set!";

        return QByteArray();
    }

    QString filePath = attribute(QLatin1String("originalFilePath")).toString();

    if (filePath.isEmpty())
    {
        return QByteArray();
    }

    FileReadLocker lock(filePath);

    QByteArray hash = DImg::getUniqueHashVersion(filePath, version);
    setAttribute(uniqueHash, hash);

    return hash;
}

QByteArray DImg::getUniqueHashVersion(const QString& filePath, int version)
{
    if      (version == 2)
    {
        return DImg::createUniqueHashV2(filePath);
    }
    else if (version == 3)
    {
        return DImg::createUniqueHashV3(filePath);
    }

    qCWarning(DIGIKAM_DIMG_LOG) << "DImg::getUniqueHashVersion called with "
                                   "unsupported version:" << version;

    return QByteArray();
}

QByteArray DImg::createImageUniqueId()
{
    NonDeterministicRandomData randomData(16);
    QByteArray imageUUID = randomData.toHex();
    imageUUID           += getUniqueHashVersion(2);

    return imageUUID;
}

void DImg::prepareMetadataToSave(const QString& intendedDestPath, const QString& destMimeType,
                                 bool resetExifOrientationTag)
{
    PrepareMetadataFlags flags = PrepareMetadataFlagsAll;

    if (!resetExifOrientationTag)
    {
        flags &= ~ResetExifOrientationTag;
    }

    QUrl url = QUrl::fromLocalFile(originalFilePath());
    prepareMetadataToSave(intendedDestPath, destMimeType, url.fileName(), flags);
}

void DImg::prepareMetadataToSave(const QString& intendedDestPath, const QString& destMimeType,
                                 const QString& originalFileName, PrepareMetadataFlags flags)
{
    if (isNull())
    {
        return;
    }

    // Get image Exif/IPTC data.

    QScopedPointer<DMetadata> meta(new DMetadata(getMetadata()));

    qCDebug(DIGIKAM_DIMG_LOG) << "Prepare Metadata to save for" << intendedDestPath;

    if ((flags & RemoveOldMetadataPreviews) || (flags & CreateNewMetadataPreview))
    {
        // Clear IPTC preview

        meta->removeIptcTag("Iptc.Application2.Preview");
        meta->removeIptcTag("Iptc.Application2.PreviewFormat");
        meta->removeIptcTag("Iptc.Application2.PreviewVersion");

        // Clear Exif thumbnail

        meta->removeExifThumbnail();

        // Clear Tiff thumbnail

        MetaEngine::MetaDataMap tiffThumbTags = meta->getExifTagsDataList(QStringList() << QLatin1String("SubImage1"));

        for (MetaEngine::MetaDataMap::iterator it = tiffThumbTags.begin() ; it != tiffThumbTags.end() ; ++it)
        {
            meta->removeExifTag(it.key().toLatin1().constData());
        }

        // Clear Xmp preview from digiKam namespace

        meta->removeXmpTag("Xmp.digiKam.Preview");
    }

    // Refuse preview creation for images with transparency
    // as long as we have no format to support this. See bug 286127

    if ((flags & CreateNewMetadataPreview) && !hasTransparentPixels())
    {
        const QSize standardPreviewSize(1280, 1280);
        QSize previewSize = size();

        // Scale to standard preview size. Only scale down, not up

        if (width() > (uint)standardPreviewSize.width() && height() > (uint)standardPreviewSize.height())
        {
            previewSize.scale(standardPreviewSize, Qt::KeepAspectRatio);
        }

        // Only store a new preview if it is worth it - the original should be significantly larger than the preview

        bool storeNewPreview = (2 * (uint)previewSize.width() <= width());

        // Create the preview and thumb QImage

        QImage preview;
        QImage thumb;
        {
            if (!IccManager::isSRGB(*this))
            {
                DImg previewDImg;

                if (previewSize.width() >= (int)width())
                {
                    previewDImg = copy();
                }
                else
                {
                    previewDImg = smoothScale(previewSize.width(),
                                              previewSize.height(),
                                              Qt::IgnoreAspectRatio);
                }

                IccManager manager(previewDImg);
                manager.transformToSRGB();
                preview = previewDImg.copyQImage();
            }
            else
            {
                // Ensure that preview is not upscaled

                if (previewSize.width() >= (int)width())
                {
                    preview = copyQImage();
                }
                else
                {
                    preview = smoothScale(previewSize.width(),
                                          previewSize.height(),
                                          Qt::IgnoreAspectRatio).copyQImage();
                }
            }

            thumb = preview.scaled(160, 120, Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);
        }

        // Update IPTC preview.
        // see bug #130525. a JPEG segment is limited to 64K. If the IPTC byte array is
        // bigger than 64K during of image preview tag size, the target JPEG image will be
        // broken. Note that IPTC image preview tag is limited to 256K!!!
        // There is no limitation with TIFF and PNG about IPTC byte array size.
        // So for a JPEG file, we don't store the IPTC preview.

        if (
            storeNewPreview                                    &&
            ((destMimeType.toUpper() == QLatin1String("PNG"))  ||
             (destMimeType.toUpper() == QLatin1String("TIF"))  ||
             (destMimeType.toUpper() == QLatin1String("TIFF")))
           )
        {
            // Non JPEG file, we update IPTC and XMP preview

            meta->setItemPreview(preview);
        }

        if (
            (destMimeType.toUpper() == QLatin1String("TIF")) ||
            (destMimeType.toUpper() == QLatin1String("TIFF"))
           )
        {
            // With TIFF file, we don't store JPEG thumbnail, we even need to erase it and store
            // a thumbnail at a special location. See bug #211758

            meta->setTiffThumbnail(thumb);

            // Update Exif Image dimensions.
            // Disable Exif.Photo.PixelX(Y)Dimension in TIFF files.

            meta->setItemDimensions(size(), false);
        }
        else
        {
            // Update Exif thumbnail.

            meta->setExifThumbnail(thumb);

            // Update Exif Image dimensions.

            meta->setItemDimensions(size());
        }
    }

    // Update Exif Document Name tag with the original file name.

    if (!originalFileName.isEmpty())
    {
        meta->setExifTagString("Exif.Image.DocumentName", originalFileName);
    }

    // Update Exif Orientation tag if necessary.

    if (flags & ResetExifOrientationTag)
    {
        meta->setItemOrientation(DMetadata::ORIENTATION_NORMAL);
    }

    if (!m_priv->imageHistory.isEmpty())
    {
        DImageHistory forSaving(m_priv->imageHistory);
        forSaving.adjustReferredImages();

        QUrl url         = QUrl::fromLocalFile(intendedDestPath);
        QString filePath = url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toLocalFile() + QLatin1Char('/');
        QString fileName = url.fileName();

        if (!filePath.isEmpty() && !fileName.isEmpty())
        {
            forSaving.purgePathFromReferredImages(filePath, fileName);
        }

        QString imageHistoryXml = forSaving.toXml();
        meta->setItemHistory(imageHistoryXml);
    }

    if (flags & CreateNewImageHistoryUUID)
    {
        meta->setItemUniqueId(QString::fromUtf8(createImageUniqueId()));
    }

    // Store new Exif/IPTC/XMP data into image.

    setMetadata(meta->data());
}

HistoryImageId DImg::createHistoryImageId(const QString& filePath, HistoryImageId::Type type)
{
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists())
    {
        return HistoryImageId();
    }

    QScopedPointer<DMetadata> metadata(new DMetadata(getMetadata()));
    HistoryImageId id(metadata->getItemUniqueId());

    QDateTime dt = metadata->getItemDateTime();

    if (dt.isNull())
    {
        dt = creationDateFromFilesystem(fileInfo);
    }

    id.setCreationDate(dt);
    id.setFileName(fileInfo.fileName());
    id.setPath(fileInfo.path());
    id.setUniqueHash(QString::fromUtf8(getUniqueHashVersion(2)), fileInfo.size());
    id.setType(type);

    return id;
}

HistoryImageId DImg::addAsReferredImage(const QString& filePath, HistoryImageId::Type type)
{
    HistoryImageId id = createHistoryImageId(filePath, type);
    m_priv->imageHistory.purgePathFromReferredImages(id.path(), id.fileName());
    addAsReferredImage(id);

    return id;
}

void DImg::addAsReferredImage(const HistoryImageId& id)
{
    m_priv->imageHistory << id;
}

void DImg::insertAsReferredImage(int afterHistoryStep, const HistoryImageId& id)
{
    m_priv->imageHistory.insertReferredImage(afterHistoryStep, id);
}

void DImg::addCurrentUniqueImageId(const QString& uuid)
{
    m_priv->imageHistory.adjustCurrentUuid(uuid);
}

void DImg::addFilterAction(const Digikam::FilterAction& action)
{
    m_priv->imageHistory << action;
}

const DImageHistory& DImg::getItemHistory() const
{
    return m_priv->imageHistory;
}

DImageHistory& DImg::getItemHistory()
{
    return m_priv->imageHistory;
}

void DImg::setItemHistory(const DImageHistory& history)
{
    m_priv->imageHistory = history;
}

bool DImg::hasImageHistory() const
{
    if (m_priv->imageHistory.isEmpty())
    {
        return false;
    }
    else
    {
        return true;
    }
}

DImageHistory DImg::getOriginalImageHistory() const
{
    return attribute(QLatin1String("originalImageHistory")).value<DImageHistory>();
}

void DImg::setHistoryBranch(bool isBranch)
{
    setHistoryBranchAfter(getOriginalImageHistory(), isBranch);
}

void DImg::setHistoryBranchAfter(const DImageHistory& historyBeforeBranch, bool isBranch)
{
    int addedSteps = m_priv->imageHistory.size() - historyBeforeBranch.size();
    setHistoryBranchForLastSteps(addedSteps, isBranch);
}

void DImg::setHistoryBranchForLastSteps(int numberOfLastHistorySteps, bool isBranch)
{
    int firstStep = m_priv->imageHistory.size() - numberOfLastHistorySteps;

    if (firstStep < m_priv->imageHistory.size())
    {
        if (isBranch)
        {
            m_priv->imageHistory[firstStep].action.addFlag(FilterAction::ExplicitBranch);
        }
        else
        {
            m_priv->imageHistory[firstStep].action.removeFlag(FilterAction::ExplicitBranch);
        }
    }
}

QString DImg::colorModelToString(COLORMODEL colorModel)
{
    switch (colorModel)
    {
        case RGB:
        {
            return i18nc("Color Model: RGB", "RGB");
        }

        case GRAYSCALE:
        {
            return i18nc("Color Model: Grayscale", "Grayscale");
        }

        case MONOCHROME:
        {
            return i18nc("Color Model: Monochrome", "Monochrome");
        }

        case INDEXED:
        {
            return i18nc("Color Model: Indexed", "Indexed");
        }

        case YCBCR:
        {
            return i18nc("Color Model: YCbCr", "YCbCr");
        }

        case CMYK:
        {
            return i18nc("Color Model: CMYK", "CMYK");
        }

        case CIELAB:
        {
            return i18nc("Color Model: CIE L*a*b*", "CIE L*a*b*");
        }

        case COLORMODELRAW:
        {
            return i18nc("Color Model: Uncalibrated (RAW)", "Uncalibrated (RAW)");
        }

        case COLORMODELUNKNOWN:
        default:
        {
            return i18nc("Color Model: Unknown", "Unknown");
        }
    }
}

bool DImg::isAnimatedImage(const QString& filePath)
{
    QImageReader reader(filePath);
    reader.setDecideFormatFromContent(true);

    if (reader.supportsAnimation() &&
       (reader.imageCount() > 1))
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "File \"" << filePath << "\" is an animated image";
        return true;
    }

    return false;
}

int DImg::exifOrientation(const QString& filePath)
{
    QVariant preview(attribute(QLatin1String("fromRawEmbeddedPreview")));

    return LoadSaveThread::exifOrientation(filePath,
                                           DMetadata(getMetadata()),
                                           (detectedFormat() == DImg::RAW),
                                           (preview.isValid() && preview.toBool()));
}

QByteArray DImg::createUniqueHash(const QString& filePath, const QByteArray& ba)
{
    // Create the unique ID

    QCryptographicHash md5(QCryptographicHash::Md5);

    // First, read the Exif data into the hash

    md5.addData(ba);

    // Second, read in the first 8KB of the file

    QFile qfile(filePath);

    char databuf[8192] = { 0 };
    QByteArray hash;

    if (qfile.open(QIODevice::Unbuffered | QIODevice::ReadOnly))
    {
        int readlen = 0;

        if ((readlen = qfile.read(databuf, 8192)) > 0)
        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))

            md5.addData(QByteArrayView(databuf, readlen));

#else

            md5.addData(databuf, readlen);

#endif

            QByteArray bsize;
            md5.addData(bsize.setNum(qfile.size()));

            hash = md5.result().toHex();
        }

        qfile.close();
    }

    return hash;
}

QByteArray DImg::createUniqueHashV2(const QString& filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::Unbuffered | QIODevice::ReadOnly))
    {
        return QByteArray();
    }

    QCryptographicHash md5(QCryptographicHash::Md5);

    // Specified size: 100 kB; but limit to file size

    const qint64 specifiedSize = 100 * 1024; // 100 kB
    qint64 fsize               = qMin(file.size(), specifiedSize);

    if (fsize)
    {
        QScopedArrayPointer<char> databuf(new char[fsize] { 0 });
        int read;

        // Read first 100 kB

        if ((read = file.read(databuf.data(), fsize)) > 0)
        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))

            md5.addData(QByteArrayView(databuf.data(), read));

#else

            md5.addData(databuf.data(), read);

#endif

        }

        // Read last 100 kB

        file.seek(file.size() - fsize);

        if ((read = file.read(databuf.data(), fsize)) > 0)
        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))

            md5.addData(QByteArrayView(databuf.data(), read));

#else

            md5.addData(databuf.data(), read);

#endif

        }
    }

    file.close();

    return md5.result().toHex();
}

QByteArray DImg::createUniqueHashV3(const QString& filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::Unbuffered | QIODevice::ReadOnly))
    {
        return QByteArray();
    }

    QCryptographicHash md5(QCryptographicHash::Md5);

    // maximum size: 100 kB for the first block, all other
    // possible 5 blocks up to 25 kB; but limit to file size

    const qint64 firstSize = 100 * 1024;         // 100 kB
    const qint64 nextSize  = 25  * 1024;         //  25 kB
    const qint64 fsize     = qMin(file.size(), firstSize);

    const qint64 bsize     = file.size() - fsize;
    const qint64 block     = qMin(bsize, nextSize);
    const qint64 step      = (bsize - block) / 5 + 1;

    if (fsize)
    {
        QScopedArrayPointer<char> databuf(new char[fsize] { 0 });
        qint64 read;

        for (int sp = 0 ; sp < 6 ; ++sp)
        {
            qint64 rsize = !sp ? fsize : block;
            qint64 rstep = !sp ? 0 : step * sp + fsize;

            file.seek(qMin(rstep, file.size() - rsize));

            if ((read = file.read(databuf.data(), rsize)) > 0)
            {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))

                md5.addData(QByteArrayView(databuf.data(), read));

#else

                md5.addData(databuf.data(), read);

#endif

            }

            if (file.atEnd())
            {
                break;
            }
        }
    }

    file.close();

    return md5.result().toHex();
}

} // namespace Digikam
