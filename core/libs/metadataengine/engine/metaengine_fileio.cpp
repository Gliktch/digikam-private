/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               File I/O methods
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metaengine_p.h"

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace Digikam
{

void MetaEngine::setFilePath(const QString& path)
{
    d->filePath = path;
}

QString MetaEngine::getFilePath() const
{
    return d->filePath;
}

QString MetaEngine::sidecarFilePathForFile(const QString& path)
{
    if (path.isEmpty())
    {
        return QString();
    }

    if (MetaEngineSettings::instance()->settings().useCompatibleFileName)
    {
        QFileInfo info(path);
        QString pathBaseXmp = path;
        pathBaseXmp.chop(info.suffix().size());

        return (pathBaseXmp + QLatin1String("xmp"));
    }

    return (path + QLatin1String(".xmp"));
}

QUrl MetaEngine::sidecarUrl(const QUrl& url)
{
    return sidecarUrl(url.toLocalFile());
}

QUrl MetaEngine::sidecarUrl(const QString& path)
{
    return QUrl::fromLocalFile(sidecarFilePathForFile(path));
}

QString MetaEngine::sidecarPath(const QString& path)
{
    return sidecarFilePathForFile(path);
}

bool MetaEngine::hasSidecar(const QString& path)
{
    return QFileInfo::exists(sidecarFilePathForFile(path));
}

QString MetaEngine::backendName(Backend t)
{
    switch (t)
    {
        case LibRawBackend:
        {
            return QLatin1String("LibRaw");
        }

        case LibHeifBackend:
        {
            return QLatin1String("LibHeif");
        }

        case ImageMagickBackend:
        {
            return QLatin1String("ImageMagick");
        }

        case FFMpegBackend:
        {
            return QLatin1String("FFMpeg");
        }

        case ExifToolBackend:
        {
            return QLatin1String("ExifTool");
        }

        case VideoMergeBackend:
        {
            return QLatin1String("VideoMerge");
        }

        case NoBackend:
        {
            return QLatin1String("No Backend");
        }

        default:
        {
            return QLatin1String("Exiv2");
        }
    }
}

bool MetaEngine::load(const QString& filePath, Backend* backend)
{
    if (backend)
    {
        *backend = NoBackend;
    }

    if (filePath.isEmpty())
    {
        return false;
    }

    d->filePath = filePath;

    if (d->readWithExifTool)
    {
        return false;
    }

    QMutexLocker lock(&s_metaEngineMutex);

    s_metaEngineWarnOrError = false;
    bool hasLoaded          = false;

    try
    {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filePath.toUtf8().constData());

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

        // Exif byte order --------------------------------

        d->exifByteOrder() = image->byteOrder();

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

        if (backend)
        {
            *backend = Exiv2Backend;
        }

        hasLoaded = true;

    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QString::fromUtf8("Cannot load metadata from file with Exiv2 backend: %1").arg(getFilePath()), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return hasLoaded;
}

bool MetaEngine::loadFromSidecarAndMerge(const QString& filePath)
{
    if (filePath.isEmpty())
    {
        return false;
    }

    d->filePath    = filePath;
    bool hasLoaded = false;

#ifdef _XMP_SUPPORT_

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        if (d->useXMPSidecar4Reading)
        {
            QString xmpSidecarPath = sidecarFilePathForFile(filePath);
            QFileInfo xmpSidecarFileInfo(xmpSidecarPath);

            if (xmpSidecarFileInfo.exists() && xmpSidecarFileInfo.isReadable())
            {
                // Read sidecar data

                Exiv2::Image::AutoPtr xmpsidecar = Exiv2::ImageFactory::open(xmpSidecarPath.toUtf8().constData());

                xmpsidecar->readMetadata();

                // Merge

#if EXIV2_TEST_VERSION(0,27,99)

                d->loadSidecarData(std::move(xmpsidecar));

#else

                d->loadSidecarData(xmpsidecar);

#endif

                hasLoaded = true;
            }
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QString::fromUtf8("Cannot load XMP sidecar from file with Exiv2 backend: %1").arg(getFilePath()), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#endif // _XMP_SUPPORT_

    return hasLoaded;
}

bool MetaEngine::save(const QString& imageFilePath, bool setVersion) const
{
    if (setVersion && !setProgramId())
    {
        return false;
    }

    // If our image is really a symlink, we should follow the symlink so that
    // when we delete the file and rewrite it, we are honoring the symlink
    // (rather than just deleting it and putting a file there).
    //
    // However, this may be surprising to the user when they are writing sidecar
    // files.  They might expect them to show up where the symlink is.  So, we
    // shouldn't follow the link when figuring out what the filename for the
    // sidecar should be.
    //
    // Note, we are not yet handling the case where the sidecar itself is a
    // symlink.

    QString regularFilePath = imageFilePath; // imageFilePath might be a
                                             // symlink.  Below we will change
                                             // regularFile to the pointed to
                                             // file if so.
    QFileInfo givenFileInfo(imageFilePath);

    if (givenFileInfo.isSymLink())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "filePath" << imageFilePath << "is a symlink."
                                        << "Using target" << givenFileInfo.canonicalFilePath();

        regularFilePath = givenFileInfo.canonicalFilePath(); // Walk all the symlinks
    }

    bool writeToFile                     = false;
    bool writeToSidecar                  = false;
    bool writeToSidecarIfFileNotPossible = false;
    bool writtenToFile                   = false;
    bool writtenToSidecar                = false;

    qCDebug(DIGIKAM_METAENGINE_LOG) << "MetaEngine::metadataWritingMode" << d->metadataWritingMode;

    switch (d->metadataWritingMode)
    {
        case WRITE_TO_SIDECAR_ONLY:
        {
            writeToSidecar = true;
            break;
        }

        case WRITE_TO_FILE_ONLY:
        {
            writeToFile    = true;
            break;
        }

        case WRITE_TO_SIDECAR_AND_FILE:
        {
            writeToFile    = true;
            writeToSidecar = true;
            break;
        }

        case WRITE_TO_SIDECAR_ONLY_FOR_READ_ONLY_FILES:
        {
            writeToFile                     = true;
            writeToSidecarIfFileNotPossible = true;
            break;
        }
    }

    // NOTE: see bugs #137770 & #138540 : never touch the file if is read only.

    QFileInfo finfo(regularFilePath);
    QFileInfo dinfo(finfo.path());

    if (writeToFile)
    {
        if (!dinfo.isWritable())
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Dir" << dinfo.filePath() << "is read-only. Metadata not saved.";
            writtenToFile = false;
        }
        else
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Will write Metadata to file" << finfo.filePath();
            writtenToFile = d->saveToFile(finfo);
        }

        if (writtenToFile)
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Metadata for file" << finfo.fileName() << "written to file.";
        }
    }

    if (
        writeToSidecar                                                 ||
        (writeToSidecarIfFileNotPossible && !writtenToFile)            ||
        (writeToSidecarIfFileNotPossible && hasSidecar(finfo.filePath()))
       )
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Will write XMP sidecar for file" << finfo.fileName();

        if (!dinfo.isWritable())
        {
            writtenToSidecar = d->saveToXMPSidecar(QFileInfo(imageFilePath));
        }
        else
        {
            writtenToSidecar = d->saveToXMPSidecar(QFileInfo(regularFilePath));
        }

        if (writtenToSidecar)
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Metadata for file" << finfo.fileName() << "written to XMP sidecar.";
        }
    }

    return (writtenToFile || writtenToSidecar);
}

bool MetaEngine::applyChanges(bool setVersion) const
{
    if (d->filePath.isEmpty())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Failed to apply changes: file path is empty!";
        return false;
    }

    return save(d->filePath, setVersion);
}

bool MetaEngine::exportChanges(const QString& exvTmpFile) const
{
    if (exvTmpFile.isEmpty())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Failed to export changes: temp path is empty!";
        return false;
    }

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        // Create target EXV container.

        Exiv2::Image::AutoPtr targetExv = Exiv2::ImageFactory::create(Exiv2::ImageType::exv, exvTmpFile.toStdString());
        targetExv->setComment(d->itemComments());
        targetExv->setExifData(d->exifMetadata());
        targetExv->setIptcData(d->iptcMetadata());

#ifdef _XMP_SUPPORT_

        targetExv->setXmpData(d->xmpMetadata());

#endif // _XMP_SUPPORT_

#if EXIV2_TEST_VERSION(0,27,99)

        if (!d->iccProfileBuf().empty())
        {
            targetExv->setIccProfile(Exiv2::DataBuf(d->iccProfileBuf()));
        }

#else

        if (d->iccProfileBuf().size_ > 0)
        {
            targetExv->setIccProfile(d->iccProfileBuf());
        }

#endif

        targetExv->writeMetadata();

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot export changes with Exiv2 backend: "), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

} // namespace Digikam

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif
