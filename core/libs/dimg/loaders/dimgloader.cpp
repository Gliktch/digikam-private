/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : DImg image loader interface
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgloader.h"

// C++ includes

#include <new>
#include <cstddef>

// Qt includes

#include <QScopedPointer>

// KDE includes

#include <kmemoryinfo.h>

// Local includes

#include "dimg_p.h"
#include "dmetadata.h"
#include "dimgloaderobserver.h"

namespace Digikam
{

DImgLoader::DImgLoader(DImg* const image)
    : m_image(image)
{
}

DImgLoader::~DImgLoader()
{
}

void DImgLoader::setLoadFlags(LoadFlags flags)
{
    m_loadFlags = flags;
}

bool DImgLoader::hasLoadedData() const
{
    return ((m_loadFlags & (LoadImageData | LoadPreview)) && m_image->m_priv->data);
}

int DImgLoader::granularity(DImgLoaderObserver* const observer, int total, float progressSlice)
{
    // Splits expect total value into the chunks where checks shall occur
    // and combines this with a possible correction factor from observer.
    // Progress slice is the part of 100% concerned with the current granularity
    // (E.g. in a loop only the values from 10% to 90% are used, then progressSlice is 0.8)
    // Current default is 1/20, that is progress info every 5%

    int granularity = 0;

    if (observer)
    {
        granularity = (int)((total / (20 * progressSlice)) / observer->granularity());
    }

    return (granularity ? granularity : 1);
}

unsigned char*& DImgLoader::imageData()
{
    return m_image->m_priv->data;
}

quint64 DImgLoader::imageNumBytes() const
{
    return m_image->numBytes();
}

unsigned int& DImgLoader::imageWidth()
{
    return m_image->m_priv->width;
}

unsigned int& DImgLoader::imageHeight()
{
    return m_image->m_priv->height;
}

bool DImgLoader::imageHasAlpha() const
{
    return m_image->hasAlpha();
}

bool DImgLoader::imageSixteenBit() const
{
    return m_image->sixteenBit();
}

int DImgLoader::imageBitsDepth() const
{
    return m_image->bitsDepth();
}

int DImgLoader::imageBytesDepth() const
{
    return m_image->bytesDepth();
}

void DImgLoader::imageSetIccProfile(const IccProfile& profile)
{
    m_image->setIccProfile(profile);
}

QVariant DImgLoader::imageGetAttribute(const QString& key) const
{
    return m_image->attribute(key);
}

QString DImgLoader::imageGetEmbbededText(const QString& key) const
{
    return m_image->embeddedText(key);
}

void DImgLoader::imageSetAttribute(const QString& key, const QVariant& value)
{
    m_image->setAttribute(key, value);
}

QMap<QString, QString>& DImgLoader::imageEmbeddedText() const
{
    return m_image->m_priv->embeddedText;
}

void DImgLoader::imageSetEmbbededText(const QString& key, const QString& text)
{
    m_image->setEmbeddedText(key, text);
}

void DImgLoader::loadingFailed()
{
    if (m_image->m_priv->data)
    {
        delete [] m_image->m_priv->data;
    }

    m_image->m_priv->data   = nullptr;
    m_image->m_priv->width  = 0;
    m_image->m_priv->height = 0;
}

qint64 DImgLoader::checkAllocation(qint64 fullSize)
{
    if ((quint64)fullSize >= std::numeric_limits<size_t>::max())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Cannot allocate buffer of size" << fullSize;
        return 0;
    }

    // Do extra check if allocating serious amounts of memory.
    // At the time of writing (2011), I consider 100 MB as "serious".

    if (fullSize > (qint64)(100 * 1024 * 1024))
    {
        KMemoryInfo memInfo;

        if (memInfo.isNull())
        {
            qCWarning(DIGIKAM_DIMG_LOG) << "Not a recognized platform to get memory information";

            return -1;
        }

        qint64 available = memInfo.availablePhysical();

        if (available == 0)
        {
            qCWarning(DIGIKAM_DIMG_LOG) << "Error to get physical memory information form a recognized platform";

            return 0;
        }

        if (fullSize > available)
        {
            qCWarning(DIGIKAM_DIMG_LOG) << "Not enough memory to allocate buffer of size " << fullSize;
            qCWarning(DIGIKAM_DIMG_LOG) << "Available memory size is "                     << available;

            return 0;
        }
    }

    return fullSize;
}

bool DImgLoader::readMetadata(const QString& filePath)
{
    if (!((m_loadFlags & LoadMetadata) || (m_loadFlags & LoadUniqueHash) || (m_loadFlags & LoadImageHistory)))
    {
        return false;
    }

    QScopedPointer<DMetadata> metaDataFromFile(new DMetadata);

    if (!metaDataFromFile->load(filePath))
    {
        m_image->setMetadata(MetaEngineData());
        return false;
    }

    m_image->setMetadata(metaDataFromFile->data());

    if (m_loadFlags & LoadImageHistory)
    {
        DImageHistory history = DImageHistory::fromXml(metaDataFromFile->getItemHistory());
        HistoryImageId id     = m_image->createHistoryImageId(filePath, HistoryImageId::Current);
        history << id;

        m_image->setItemHistory(history);
        imageSetAttribute(QLatin1String("originalImageHistory"), QVariant::fromValue(history));
    }

    return true;
}

bool DImgLoader::saveMetadata(const QString& filePath)
{
    QScopedPointer<DMetadata> metaDataToFile(new DMetadata(filePath));
    metaDataToFile->setData(m_image->getMetadata());

    QVariant writingMode = imageGetAttribute(QLatin1String("metadataWritingMode"));

    if      (writingMode.isValid())
    {
        metaDataToFile->setMetadataWritingMode(writingMode.toInt());
    }
    else if (metaDataToFile->metadataWritingMode() == (int)DMetadata::WRITE_TO_SIDECAR_ONLY)
    {
        metaDataToFile->setMetadataWritingMode((int)DMetadata::WRITE_TO_SIDECAR_AND_FILE);
    }

    return metaDataToFile->applyChanges(true);
}

bool DImgLoader::checkExifWorkingColorSpace() const
{
    QScopedPointer<DMetadata> metaData(new DMetadata(m_image->getMetadata()));
    IccProfile profile = metaData->getIccProfile();

    if (!profile.isNull())
    {
        m_image->setIccProfile(profile);
        return true;
    }

    return false;
}

void DImgLoader::storeColorProfileInMetadata()
{
    IccProfile profile = m_image->getIccProfile();

    if (profile.isNull())
    {
        return;
    }

    QScopedPointer<DMetadata> metaData(new DMetadata(m_image->getMetadata()));
    metaData->setIccProfile(profile);
    m_image->setMetadata(metaData->data());
}

void DImgLoader::purgeExifWorkingColorSpace()
{
    QScopedPointer<DMetadata> meta(new DMetadata(m_image->getMetadata()));
    meta->removeExifColorSpace();
    m_image->setMetadata(meta->data());
}

unsigned char* DImgLoader::new_failureTolerant(size_t unsecureSize)
{
    return new_failureTolerant<unsigned char>(unsecureSize);
}

unsigned char* DImgLoader::new_failureTolerant(quint64 w, quint64 h, uint typesPerPixel)
{
    return new_failureTolerant<unsigned char>(w, h, typesPerPixel);
}

unsigned short* DImgLoader::new_short_failureTolerant(size_t unsecureSize)
{
    return new_failureTolerant<unsigned short>(unsecureSize);
}

unsigned short* DImgLoader::new_short_failureTolerant(quint64 w, quint64 h, uint typesPerPixel)
{
    return new_failureTolerant<unsigned short>(w, h, typesPerPixel);
}

int DImgLoader::convertCompressionForLibPng(int value)
{
    // PNG compression slider settings : 1 - 9 ==> libpng settings : 100 - 1.

    return ((int)(((1.0 - 100.0) / 8.0) * (float)value + 100.0 - ((1.0 - 100.0) / 8.0)));
}

int DImgLoader::convertCompressionForLibJpeg(int value)
{
    // JPEG quality slider settings : 1 - 100 ==> libjpeg settings : 25 - 100.

    return ((int)((75.0 / 100.0) * (float)value + 26.0 - (75.0 / 100.0)));
}

} // namespace Digikam
