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

#pragma once

// C++ includes

#include <limits>

// Qt includes

#include <QMap>
#include <QString>
#include <QVariant>

// Local includes

#include "digikam_debug.h"
#include "digikam_export.h"
#include "dimg.h"

namespace Digikam
{

class DImgLoaderObserver;
class DMetadata;

class DIGIKAM_EXPORT DImgLoader
{
public:

    /**
     * @brief This is the list of loading modes usable by DImg image plugins
     */
    enum LoadFlag
    {
        /// Load image information without image data

        LoadItemInfo     = 1,           ///< Image info as width and height
        LoadMetadata     = 2,           ///< Image metadata
        LoadICCData      = 4,           ///< Image color profile

        LoadImageData    = 8,           ///< Full image data
        LoadUniqueHash   = 16,          ///< Image unique hash
        LoadImageHistory = 32,          ///< Image version history

        /// Special mode to load reduced image data

        LoadPreview      = 64,          ///< Load embedded preview image instead full size image

        /// Helper to load all information, metadata and full image.

        LoadAll          = LoadItemInfo | LoadMetadata | LoadICCData | LoadImageData | LoadUniqueHash | LoadImageHistory
    };
    Q_DECLARE_FLAGS(LoadFlags, LoadFlag)

public:

    void setLoadFlags(LoadFlags flags);

    virtual ~DImgLoader();

    virtual bool load(const QString& filePath, DImgLoaderObserver* const observer)        = 0;
    virtual bool save(const QString& filePath, DImgLoaderObserver* const observer)        = 0;

    virtual bool hasLoadedData()                                                    const;
    virtual bool hasAlpha()                                                         const = 0;
    virtual bool sixteenBit()                                                       const = 0;
    virtual bool isReadOnly()                                                       const = 0;

    static unsigned char*  new_failureTolerant(size_t unsecureSize);
    static unsigned char*  new_failureTolerant(quint64 w, quint64 h, uint typesPerPixel);
    static unsigned short* new_short_failureTolerant(size_t unsecureSize);
    static unsigned short* new_short_failureTolerant(quint64 w, quint64 h, uint typesPerPixel);

    static int convertCompressionForLibPng(int value);
    static int convertCompressionForLibJpeg(int value);

    /**
     * @return -1 : unsupported platform
     *          0 : parse failure from supported platform
     *          1 : parse done with success from supported platform
     */
    static qint64 checkAllocation(qint64 fullSize);

    template <typename Type> static Type* new_failureTolerant(size_t unsecureSize);
    template <typename Type> static Type* new_failureTolerant(quint64 w, quint64 h, uint typesPerPixel);

protected:

    explicit DImgLoader(DImg* const image);

    unsigned char*&         imageData();
    unsigned int&           imageWidth();
    unsigned int&           imageHeight();

    bool                    imageHasAlpha()                                         const;
    bool                    imageSixteenBit()                                       const;

    quint64                 imageNumBytes()                                         const;
    int                     imageBitsDepth()                                        const;
    int                     imageBytesDepth()                                       const;

    void                    imageSetIccProfile(const IccProfile& profile);
    QVariant                imageGetAttribute(const QString& key)                   const;
    void                    imageSetAttribute(const QString& key,
                                              const QVariant& value);

    QMap<QString, QString>& imageEmbeddedText()                                     const;
    QString                 imageGetEmbbededText(const QString& key)                const;
    void                    imageSetEmbbededText(const QString& key,
                                                 const QString& text);

    void                    loadingFailed();
    bool                    checkExifWorkingColorSpace()                            const;
    void                    purgeExifWorkingColorSpace();
    void                    storeColorProfileInMetadata();

    virtual bool            readMetadata(const QString& filePath);
    virtual bool            saveMetadata(const QString& filePath);
    virtual int             granularity(DImgLoaderObserver* const observer, int total, float progressSlice = 1.0F);

protected:

    DImg*     m_image       = nullptr;
    LoadFlags m_loadFlags   = LoadAll;

private:

    /// @note disabled
    DImgLoader() = delete;

private:

    Q_DISABLE_COPY(DImgLoader)
};

// ---------------------------------------------------------------------------------------------------

/**
 * @brief Allows safe multiplication of requested pixel number and bytes per pixel, avoiding particularly
 * 32 bits overflow and exceeding the size_t type
 */
template <typename Type>
Q_INLINE_TEMPLATE Type* DImgLoader::new_failureTolerant(quint64 w, quint64 h, uint typesPerPixel)
{
    quint64 requested = w * h * (quint64)typesPerPixel;

    if (requested >= std::numeric_limits<size_t>::max())
    {
        qCCritical(DIGIKAM_DIMG_LOG) << "Requested memory of" << requested * quint64(sizeof(Type))
                                     << "is larger than size_t supported by platform.";
        return nullptr;
    }

    return new_failureTolerant<Type>(requested);
}

template <typename Type>
Q_INLINE_TEMPLATE Type* DImgLoader::new_failureTolerant(size_t size)
{
    qint64 res = checkAllocation(size);

    switch (res)
    {
        case 0:       // parse failure from supported platform
        {
            return nullptr;
        }

        case -1:      // unsupported platform
        {
            // We will try to continue to allocate
            break;
        }

        default:     // parse done with success from supported platform
        {
            break;
        }
    }

    Type* const reserved = new (std::nothrow) Type[size];

    if (!reserved)
    {
        qCCritical(DIGIKAM_DIMG_LOG) << "Failed to allocate chunk of memory of size" << size;
    }

    return reserved;
}

Q_DECLARE_OPERATORS_FOR_FLAGS(DImgLoader::LoadFlags)

} // namespace Digikam
