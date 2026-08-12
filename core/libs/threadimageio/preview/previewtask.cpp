/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-12-26
 * Description : Multithreaded loader for previews
 *
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "previewtask.h"

// Qt includes

#include <QVariant>
#include <QScopedPointer>

// Local includes

#include "dimgloader.h"
#include "drawdecoder.h"
#include "digikam_debug.h"
#include "dmetadata.h"
#include "jpegutils.h"
#include "metaenginesettings.h"
#include "previewloadthread.h"

namespace Digikam
{

PreviewLoadingTask::PreviewLoadingTask(LoadSaveThread* const thread, const LoadingDescription& description)
    : SharedLoadingTask(thread, description, LoadSaveThread::AccessModeRead, LoadingTaskStatusLoading)
{
}

void PreviewLoadingTask::execute()
{
    if (m_loadingTaskStatus == LoadingTaskStatusStopping)
    {
        if (m_thread)
        {
            m_thread->taskHasFinished();
        }

        return;
    }

    if (m_thread)
    {
        m_thread->imageStartedLoading(m_loadingDescription);
    }

    // Check if preview is in cache first.

    LoadingCache* const cache = LoadingCache::cache();
    {
        LoadingCache::CacheLock lock(cache);

        // find possible cached images

        DImg* cachedImg        = nullptr;
        QStringList lookupKeys = m_loadingDescription.lookupCacheKeys();

        // lookupCacheKeys returns "best first". Prepend the cache key to make the list "fastest first":
        // Scaling a full version takes longer!

        lookupKeys.prepend(m_loadingDescription.cacheKey());

        for (const QString& key : std::as_const(lookupKeys))
        {
            if ((cachedImg = cache->retrieveImage(key)))
            {
                if      (!comparePostProcessing(cachedImg))
                {
                    cachedImg = nullptr;
                }
                else if (m_loadingDescription.needCheckRawDecoding())
                {
                    if (cachedImg->rawDecodingSettings() == m_loadingDescription.rawDecodingSettings)
                    {
                        break;
                    }
                    else
                    {
                        cachedImg = nullptr;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        if (cachedImg)
        {
            // image is found in image cache, loading is successful

            m_img = *cachedImg;
        }
        else
        {
            // find possible running loading process

            LoadingProcess* usedProcess = nullptr;

            for (QStringList::const_iterator it = lookupKeys.constBegin() ; it != lookupKeys.constEnd() ; ++it)
            {
                if ((usedProcess = cache->retrieveLoadingProcess(*it)))
                {
                    break;
                }
            }

            if (usedProcess)
            {
                // Other process is right now loading this image.
                // Add this task to the list of listeners and
                // attach this thread to the other thread, wait until loading
                // has finished.

                usedProcess->addListener(this);

                // break loop when either the loading has completed, or this task is being stopped

                // cppcheck-suppress knownConditionTrueFalse
                while ((m_loadingTaskStatus != LoadingTaskStatusStopping) && !usedProcess->completed())
                {
                    lock.timedWait();
                }

                // remove listener from process

                usedProcess->removeListener(this);

                // wake up the process which is waiting until all listeners have removed themselves

                lock.wakeAll();

                // m_img is now set to the result
            }
        }
    }

    if (continueQuery() && m_img.isNull())
    {
        {
            LoadingCache::CacheLock lock(cache);

            // Neither in cache, nor currently loading in different thread.
            // Load it here and now, add this LoadingProcess to cache list.

            cache->addLoadingProcess(this);

            // Notify other processes that we are now loading this image.
            // They might be interested - see notifyNewLoadingProcess below

            cache->notifyNewLoadingProcess(this, m_loadingDescription);
        }

        // Preview is not in cache, we will load image from file.

        DImg::FORMAT format      = DImg::fileFormat(m_loadingDescription.filePath);
        m_fromRawEmbeddedPreview = false;

        if (format == DImg::RAW)
        {
            MetaEnginePreviews previews(m_loadingDescription.filePath);

            // Check original image size using Exiv2.

            QSize originalSize  = previews.originalSize();

            // If not valid, get original size from LibRaw

            if (!originalSize.isValid())
            {
                DRawInfo container;

                if (DRawDecoder::rawFileIdentify(container, m_loadingDescription.filePath))
                {
                    originalSize = container.imageSize;
                }
            }

            switch (m_loadingDescription.previewParameters.previewSettings.quality)
            {
                case PreviewSettings::FastPreview:
                case PreviewSettings::FastButLargePreview:
                {
                    // Size calculations

                    int sizeLimit = -1;
                    int bestSize  = qMax(originalSize.width(), originalSize.height());

                    // for RAWs, the alternative is the half preview, so best size is already originalSize / 2

                    bestSize     /= 2;

                    if (m_loadingDescription.previewParameters.previewSettings.quality == PreviewSettings::FastButLargePreview)
                    {
                        sizeLimit = qMin(m_loadingDescription.previewParameters.size, bestSize);
                    }

                    if (loadExiv2Preview(previews, sizeLimit))
                    {
                        break;
                    }

                    if (loadLibRawPreview(sizeLimit))
                    {
                        break;
                    }

                    loadHalfSizeRaw();
                    break;
                }

                case PreviewSettings::HighQualityPreview:
                {
                    switch (m_loadingDescription.previewParameters.previewSettings.rawLoading)
                    {
                        case PreviewSettings::RawPreviewAutomatic:
                        {
                            // If we find a preview that is larger than half size (which is what we get from half-size original data), we take it

                            int acceptableSize = qMax(lround(originalSize.width()  * 0.48), lround(originalSize.height() * 0.48));

                            if (loadExiv2Preview(previews, acceptableSize))
                            {
                                break;
                            }

                            if (loadLibRawPreview(acceptableSize))
                            {
                                break;
                            }

                            loadHalfSizeRaw();
                            break;
                        }

                        case PreviewSettings::RawPreviewFromEmbeddedPreview:
                        {
                            if (loadExiv2Preview(previews))
                            {
                                break;
                            }

                            if (loadLibRawPreview())
                            {
                                break;
                            }

                            loadHalfSizeRaw();
                            break;
                        }

                        case PreviewSettings::RawPreviewFromRawHalfSize:
                        {
                            loadHalfSizeRaw();
                            break;
                        }

                        case PreviewSettings::RawPreviewFromRawFullSize:
                        {
                            loadFullSizeRaw();
                            break;
                        }
                    }
                }
            }

            // So far, everything loaded QImage. Convert to DImg.

            convertQImageToDImg();
        }

        // Non-RAW images or failed RAW image load

        if (continueQuery() && m_img.isNull())
        {
            m_fromRawEmbeddedPreview = false;

            qCDebug(DIGIKAM_GENERAL_LOG) << "Try to get preview from" << m_loadingDescription.filePath;
            qCDebug(DIGIKAM_GENERAL_LOG) << "Preview quality: " << m_loadingDescription.previewParameters.previewSettings.quality;

            bool isFast = (m_loadingDescription.previewParameters.previewSettings.quality == PreviewSettings::FastPreview);

            switch (m_loadingDescription.previewParameters.previewSettings.quality)
            {
                case PreviewSettings::FastPreview:
                case PreviewSettings::FastButLargePreview:
                {
                    if (isFast && loadImagePreview(m_loadingDescription.previewParameters.size))
                    {
                        convertQImageToDImg();
                        break;
                    }

                    // Set a hint to try to load a JPEG or PGF with the fast scale-before-decoding method

                    if (isFast)
                    {
                        m_img.setAttribute(QLatin1String("scaledLoadingSize"), m_loadingDescription.previewParameters.size);
                    }

                    m_img.load(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);
                    break;
                }

                case PreviewSettings::HighQualityPreview:
                {
                    m_img.load(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);
                    break;
                }
            }
        }

        if (continueQuery() && !m_img.isNull())
        {
            if (needToScale())
            {
                QSize scaledSize = m_img.size();
                scaledSize.scale(m_loadingDescription.previewParameters.size,
                                 m_loadingDescription.previewParameters.size,
                                 Qt::KeepAspectRatio);
                m_img = m_img.smoothScale(scaledSize.width(), scaledSize.height());
            }

            if (MetaEngineSettings::instance()->settings().exifRotate)
            {
                m_img.exifRotate(m_loadingDescription.filePath);
            }

            if (needsPostProcessing())
            {
                postProcess();
            }
        }

        {
            LoadingCache::CacheLock lock(cache);

            // remove this from the list of loading processes in cache

            cache->removeLoadingProcess(this);

            // put valid image into cache of loaded images

            if (continueQuery() && !m_img.isNull())
            {
                cache->putImage(m_loadingDescription.cacheKey(), m_img,
                                m_loadingDescription.filePath);

                // dispatch image to all listeners

                for (int i = 0 ; i < m_listeners.count() ; ++i)
                {
                    LoadingProcessListener* const l = m_listeners.at(i);

                    if (l->accessMode() == LoadSaveThread::AccessModeReadWrite)
                    {
                        // If a listener requested ReadWrite access, it gets a deep copy.
                        // DImg is explicitly shared.

                        l->setResult(m_loadingDescription, m_img.copy());
                    }
                    else
                    {
                        l->setResult(m_loadingDescription, m_img);
                    }
                }
            }

            // indicate that loading has finished so that listeners can stop waiting

            m_completed = true;

            // wake all listeners waiting on cache condVar, so that they remove themselves

            lock.wakeAll();

            // wait until all listeners have removed themselves

            while (m_listeners.count() != 0)
            {
                lock.timedWait();
            }
        }
    }

    // following the golden rule to avoid deadlocks, do this when CacheLock is not held

    if (continueQuery() && !m_img.isNull())
    {
        // We check before to find out if we need to provide a deep copy

        const bool needConvertToEightBit = m_loadingDescription.previewParameters.previewSettings.convertToEightBit;

        if ((accessMode() == LoadSaveThread::AccessModeReadWrite) || needConvertToEightBit)
        {
            m_img.detach();
        }

        if (needConvertToEightBit)
        {
            m_img.convertToEightBit();
        }
    }
    else if (continueQuery())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot extract preview for" << m_loadingDescription.filePath;
    }
    else
    {
        m_img = DImg();
    }

    if (m_thread)
    {
        m_thread->taskHasFinished();
        m_thread->imageLoaded(m_loadingDescription, m_img);
    }
}

void PreviewLoadingTask::progressInfo(float progress)
{
    SharedLoadingTask::progressInfo(progress);

    if ((m_loadingTaskStatus == LoadingTaskStatusLoading) && m_thread)
    {
        m_thread->loadingProgress(m_loadingDescription, progress);
    }
}

bool PreviewLoadingTask::needToScale()
{
    switch (m_loadingDescription.previewParameters.previewSettings.quality)
    {
        case PreviewSettings::FastPreview:
        {
            if (m_loadingDescription.previewParameters.size > 0)
            {
                int maxSize             = qMax(m_img.width(), m_img.height());
                int acceptableUpperSize = lround(1.25 * (double)m_loadingDescription.previewParameters.size);

                return (maxSize >= acceptableUpperSize);
            }

            break;
        }

        case PreviewSettings::FastButLargePreview:
        case PreviewSettings::HighQualityPreview:
        {
            break;
        }
    }

    return false;
}

// -- Exif/IPTC preview extraction using Exiv2 --------------------------------------------------------

bool PreviewLoadingTask::loadExiv2Preview(MetaEnginePreviews& previews, int sizeLimit)
{
    if (previews.isEmpty() || !continueQuery())
    {
        return false;
    }

    if ((sizeLimit == -1) || (qMax(previews.width(), previews.height()) >= sizeLimit))
    {
        m_qimage = previews.image();

        if (!m_qimage.isNull())
        {
            m_fromRawEmbeddedPreview = true;

            return true;
        }
    }

    return false;
}

bool PreviewLoadingTask::loadLibRawPreview(int sizeLimit)
{
    if (!continueQuery())
    {
        return false;
    }

    QImage rawPreview;
    DRawDecoder::loadEmbeddedPreview(rawPreview, m_loadingDescription.filePath);

    if (!rawPreview.isNull() &&
        ((sizeLimit == -1) || (qMax(rawPreview.width(), rawPreview.height()) >= sizeLimit)))
    {
        m_qimage                 = rawPreview;
        m_fromRawEmbeddedPreview = true;

        return true;
    }

    return false;
}

bool PreviewLoadingTask::loadHalfSizeRaw()
{
    if (!continueQuery())
    {
        return false;
    }

    DRawDecoder::loadHalfPreview(m_qimage, m_loadingDescription.filePath);

    return (!m_qimage.isNull());
}

bool PreviewLoadingTask::loadFullSizeRaw()
{
    if (!continueQuery())
    {
        return false;
    }

    DRawDecoder::loadFullImage(m_qimage, m_loadingDescription.filePath);

    return (!m_qimage.isNull());
}

void PreviewLoadingTask::convertQImageToDImg()
{
    if (!continueQuery())
    {
        return;
    }

    // convert from QImage

    m_img               = DImg(m_qimage);
    DImg::FORMAT format = DImg::fileFormat(m_loadingDescription.filePath);

    m_img.setAttribute(QLatin1String("detectedFileFormat"), format);
    m_img.setAttribute(QLatin1String("originalFilePath"),   m_loadingDescription.filePath);

    QScopedPointer<DMetadata> metadata(new DMetadata(m_loadingDescription.filePath));
    QSize orgSize = metadata->getPixelSize();

    if ((format == DImg::RAW) && LoadSaveThread::infoProvider())
    {
        orgSize = LoadSaveThread::infoProvider()->dimensionsHint(m_loadingDescription.filePath);
    }

    // In case we don't get the original size from the metadata.

    if      (orgSize.isNull())
    {
        orgSize = QSize(m_img.width(), m_img.height());
    }
    else if (
             ((m_img.width() < m_img.height()) && (orgSize.width() > orgSize.height())) ||
             ((m_img.width() > m_img.height()) && (orgSize.width() < orgSize.height()))
            )
    {
        // Set the ratio of width and height of the original
        // size to the same ratio of the loaded image.

        orgSize.transpose();
    }

    m_img.setAttribute(QLatin1String("originalSize"), orgSize);

    m_img.setMetadata(metadata->data());

    // mark as embedded preview (for Exif rotation)

    if (m_fromRawEmbeddedPreview)
    {
        m_img.setAttribute(QLatin1String("fromRawEmbeddedPreview"), true);

        // If we loaded the embedded preview, the Exif of the RAW indicates
        // the color space of the preview (see bug 195950 for NEF files)

        m_img.setIccProfile(metadata->getIccProfile());
    }

    // free memory

    m_qimage = QImage();
}

bool PreviewLoadingTask::loadImagePreview(int sizeLimit)
{
    QScopedPointer<DMetadata> metadata(new DMetadata(m_loadingDescription.filePath));

    QImage previewImage;

    if (metadata->getItemPreview(previewImage))
    {
        if (
            (sizeLimit == -1) ||
            (qMax(previewImage.width(), previewImage.height()) > sizeLimit)
           )
        {
            m_qimage = previewImage;

            return true;
        }
    }

    qDebug(DIGIKAM_GENERAL_LOG) << "Try to load DImg preview from:" << m_loadingDescription.filePath;

    DImg img;
    DImgLoader::LoadFlags loadFlags = DImgLoader::LoadItemInfo |
                                      DImgLoader::LoadMetadata |
                                      DImgLoader::LoadICCData  |
                                      DImgLoader::LoadPreview;

    if (img.load(m_loadingDescription.filePath, loadFlags, this))
    {
        if (
            (sizeLimit == -1) ||
            (qMax(img.width(), img.height()) > (uint)sizeLimit)
           )
        {
            m_qimage = img.copyQImage();

            return true;
        }
    }

    return false;
}

} // namespace Digikam
