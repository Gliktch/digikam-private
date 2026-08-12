/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : digiKam 8/16 bits image management API
 *               Files input output
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

bool DImg::loadItemInfo(const QString& filePath, bool loadMetadata, bool loadICCData, bool loadUniqueHash, bool loadImageHistory)
{
    DImgLoader::LoadFlags loadFlags = DImgLoader::LoadItemInfo;

    if (loadMetadata)
    {
        loadFlags |= DImgLoader::LoadMetadata;
    }

    if (loadICCData)
    {
        loadFlags |= DImgLoader::LoadICCData;
    }

    if (loadUniqueHash)
    {
        loadFlags |= DImgLoader::LoadUniqueHash;
    }

    if (loadImageHistory)
    {
        loadFlags |= DImgLoader::LoadImageHistory;
    }

    return load(filePath, loadFlags, nullptr, DRawDecoding());
}

bool DImg::load(const QString& filePath,
                DImgLoaderObserver* const observer,
                const DRawDecoding& rawDecodingSettings)
{
    return load(filePath, DImgLoader::LoadAll, observer, rawDecodingSettings);
}

bool DImg::load(const QString& filePath,
                bool loadMetadata,
                bool loadICCData,
                bool loadUniqueHash,
                bool loadImageHistory,
                DImgLoaderObserver* const observer,
                const DRawDecoding& rawDecodingSettings)
{
    DImgLoader::LoadFlags loadFlags = DImgLoader::LoadItemInfo | DImgLoader::LoadImageData;

    if (loadMetadata)
    {
        loadFlags |= DImgLoader::LoadMetadata;
    }

    if (loadICCData)
    {
        loadFlags |= DImgLoader::LoadICCData;
    }

    if (loadUniqueHash)
    {
        loadFlags |= DImgLoader::LoadUniqueHash;
    }

    if (loadImageHistory)
    {
        loadFlags |= DImgLoader::LoadImageHistory;
    }

    return load(filePath, loadFlags, observer, rawDecodingSettings);
}

bool DImg::load(const QString& filePath,
                int loadFlagsInt,
                DImgLoaderObserver* const observer,
                const DRawDecoding& rawDecodingSettings)
{
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists() || !fileInfo.isReadable())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "File" << filePath << "does not exist";
        return false;
    }

    DPluginDImg* plugin             = DImgStaticPriv::pluginForFile(fileInfo, false);
    DImgLoader::LoadFlags loadFlags = (DImgLoader::LoadFlags)loadFlagsInt;
    setAttribute(QLatin1String("originalFilePath"), filePath);

    FileReadLocker lock(filePath);

    // First step we check the file extension to find the right loader.

    if (observer && !observer->continueQuery())
    {
        return false;
    }

    if (plugin && (!(loadFlags & DImgLoader::LoadPreview) || plugin->previewSupported()))
    {
        qCDebug(DIGIKAM_DIMG_LOG) << filePath << ":" << plugin->loaderName() << "file identified";
        FORMAT frmt              = DImgStaticPriv::loaderNameToFormat(plugin->loaderName());
        DImgLoader* const loader = plugin->loader(this, rawDecodingSettings);
        setAttribute(QLatin1String("detectedFileFormat"), frmt);
        loader->setLoadFlags(loadFlags);

        if (loader->load(filePath, observer))
        {
            m_priv->null       = !loader->hasLoadedData();
            m_priv->alpha      = loader->hasAlpha();
            m_priv->sixteenBit = loader->sixteenBit();
            setAttribute(QLatin1String("isReadOnly"), loader->isReadOnly());
            delete loader;

            return true;
        }

        delete loader;
    }

    // In the second step we check the magic bytes to find the right loader.

    plugin = DImgStaticPriv::pluginForFile(fileInfo, true);

    if (observer && !observer->continueQuery())
    {
        return false;
    }

    if (plugin && (!(loadFlags & DImgLoader::LoadPreview) || plugin->previewSupported()))
    {
        qCDebug(DIGIKAM_DIMG_LOG) << filePath << ":" << plugin->loaderName() << "file identified (magic)";
        FORMAT frmt              = DImgStaticPriv::loaderNameToFormat(plugin->loaderName());
        DImgLoader* const loader = plugin->loader(this, rawDecodingSettings);
        setAttribute(QLatin1String("detectedFileFormat"), frmt);
        loader->setLoadFlags(loadFlags);

        if (loader->load(filePath, observer))
        {
            m_priv->null       = !loader->hasLoadedData();
            m_priv->alpha      = loader->hasAlpha();
            m_priv->sixteenBit = loader->sixteenBit();
            setAttribute(QLatin1String("isReadOnly"), loader->isReadOnly());
            delete loader;

            return true;
        }

        delete loader;
    }

    if (!plugin && !(loadFlags & DImgLoader::LoadPreview))
    {
        qCDebug(DIGIKAM_DIMG_LOG) << filePath << ": Unknown image format !!!";
        return false;
    }

    if (observer                  &&
        observer->continueQuery() &&
        !(loadFlags & DImgLoader::LoadPreview))
    {
        qCWarning(DIGIKAM_DIMG_LOG) << filePath << ": Cannot load file !!!";
    }

    return false;
}

QString DImg::formatToMimeType(FORMAT frm)
{
    QString frmstr;

    switch (frm)
    {
        case (NONE):
        {
            return frmstr;
        }

        case (JPEG):
        {
            frmstr = QLatin1String("JPG");
            break;
        }

        case (TIFF):
        {
            frmstr = QLatin1String("TIF");
            break;
        }

        case (PNG):
        {
            frmstr = QLatin1String("PNG");
            break;
        }

        case (JP2K):
        {
            frmstr = QLatin1String("JP2");
            break;
        }

        case (PGF):
        {
            frmstr = QLatin1String("PGF");
            break;
        }

        case (HEIF):
        {
            frmstr = QLatin1String("HEIC");
            break;
        }

        default:
        {
            // For QImage or ImageMagick based.

            break;
        }
    }

    return frmstr;
}

bool DImg::save(const QString& filePath, FORMAT frm, DImgLoaderObserver* const observer)
{
    if (isNull())
    {
        return false;
    }

    return(save(filePath, formatToMimeType(frm), observer));
}

bool DImg::save(const QString& filePath, const QString& format, DImgLoaderObserver* const observer)
{
    qCDebug(DIGIKAM_DIMG_LOG) << "Saving to " << filePath << " with format: " << format;

    if (isNull())
    {
        return false;
    }

    if (format.isEmpty())
    {
        return false;
    }

    QString frm = format.toUpper();
    setAttribute(QLatin1String("savedFilePath"), filePath);
    setAttribute(QLatin1String("savedFormat"),   frm);

    FileWriteLocker lock(filePath);

    const DPluginDImg* const plug = DImgStaticPriv::pluginForFormat(frm);
    DImg copyForSave              = copy();

    if ((frm == QLatin1String("JPEG")) || (frm == QLatin1String("JPG")) || (frm == QLatin1String("JPE")))
    {
        // JPEG does not support transparency, so we shall provide an image without alpha channel.
        // This is only necessary if the image has an alpha channel, and there are actually transparent pixels

        if (hasTransparentPixels())
        {
            copyForSave.removeAlphaChannel();
        }
    }

    if (plug)
    {
        DImgLoader* const loader = plug->loader(&copyForSave);
        copyForSave.setAttribute(QLatin1String("format"), frm);
        copyForSave.setAttribute(QLatin1String("savedFormat-isReadOnly"), loader->isReadOnly());
        bool ret                 = loader->save(filePath, observer);
        delete loader;

        return ret;
    }

    qCWarning(DIGIKAM_DIMG_LOG) << filePath << " : Unknown save format !!!";

    return false;
}

DImg::FORMAT DImg::fileFormat(const QString& filePath)
{
    FORMAT frmt = DImg::NONE;

    if (filePath.isEmpty())
    {
        return frmt;
    }

    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists() || !fileInfo.isReadable())
    {
        return frmt;
    }

    const DPluginDImg* const plugin = DImgStaticPriv::pluginForFile(fileInfo, false);

    if (plugin)
    {
        QString name = plugin->loaderName();
        frmt         = DImgStaticPriv::loaderNameToFormat(name);
    }

    return frmt;
}

QDateTime DImg::creationDateFromFilesystem(const QFileInfo& fileInfo) const
{
    // creation date is not what it seems on Unix

    QDateTime ctime = asDateTimeUTC(fileInfo.birthTime());
    QDateTime mtime = asDateTimeUTC(fileInfo.lastModified());

    if (ctime.isValid() && mtime.isValid())
    {
        return qMin(ctime, mtime);
    }

    if (mtime.isValid())
    {
        return mtime;
    }

    if (ctime.isValid())
    {
        return ctime;
    }

    return asDateTimeUTC(QDateTime::currentDateTime());
}

} // namespace Digikam
