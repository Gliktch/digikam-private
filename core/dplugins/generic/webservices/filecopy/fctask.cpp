/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-03-27
 * Description : file copy actions using threads.
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fctask.h"

// C++ includes

#include <algorithm>

// Qt includes

#include <QDir>
#include <QFile>
#include <QMimeDatabase>
#include <QSet>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "dfileoperations.h"
#include "previewloadthread.h"
#include "dmetadata.h"
#include "dimg.h"
#include "metaenginesettings.h"

namespace DigikamGenericFileCopyPlugin
{

class Q_DECL_HIDDEN FCTask::Private
{
public:

    Private() = default;

public:

    DItemAccessEntry                  item;
    FCContainer                       settings;
    QSharedPointer<DItemAccessHandle> accessHandle;
    QSharedPointer<DItemAccessCancellationToken> cancellation;
};

FCTask::FCTask(
    const DItemAccessEntry& item,
    const FCContainer& settings,
    const QSharedPointer<DItemAccessHandle>& accessHandle)
    : ActionJob(),
      d        (new Private)
{
    d->item         = item;
    d->settings     = settings;
    d->accessHandle = accessHandle;
    d->cancellation.reset(new DItemAccessCancellationToken);
}

FCTask::~FCTask()
{
    cancel();
    delete d;
}

void FCTask::cancel()
{
    if (d->cancellation)
    {
        d->cancellation->cancel();
    }

    ActionJob::cancel();
}

void FCTask::run()
{
    const QSharedPointer<DItemAccessHandle> accessHandle = d->accessHandle;
    const QSharedPointer<DItemAccessCancellationToken> cancellation =
        d->cancellation;
    d->accessHandle.clear();

    ActionJob::run();       // To customize thread name

    if (m_cancel || !cancellation || cancellation->isCanceled())
    {
        return;
    }

    bool ok   = true;
    QUrl dest = d->settings.destUrl.adjusted(QUrl::StripTrailingSlash);
    const QUrl logicalUrl = d->item.logicalUrl;
    const QSharedPointer<DItemAccessSourceHandle> source =
        accessHandle
        ? accessHandle->acquireSource(logicalUrl, cancellation)
        : QSharedPointer<DItemAccessSourceHandle>();

    if (!d->item.isValid() || !source || !source->validateAccess() ||
        cancellation->isCanceled())
    {
        Q_EMIT signalDone();
        return;
    }

    const QUrl physicalUrl = source->entry().physicalUrl;
    const QList<DItemAssociatedAccessEntry> associatedEntries =
        source->associatedEntries();
    const bool hasPreparedSidecars = std::any_of(
        associatedEntries.cbegin(), associatedEntries.cend(),
        [](const DItemAssociatedAccessEntry& associated)
        {
            return (associated.role ==
                    static_cast<int>(DItemAssociatedRole::XmpSidecar)) ||
                   (associated.role ==
                    static_cast<int>(DItemAssociatedRole::ConfiguredSidecar));
        });

    if (d->settings.iface && d->settings.iface->supportAlbums() && d->settings.albumPath)
    {
        DItemInfo info(d->settings.iface->itemInfo(logicalUrl));
        DAlbumInfo album(d->settings.iface->albumInfo(info.albumId()));

        dest.setPath(dest.path() + album.albumPath());
        dest = dest.adjusted(QUrl::StripTrailingSlash);

        if (!QDir(dest.toLocalFile()).exists())
        {
            ok = QDir().mkpath(dest.toLocalFile());
        }
    }

    dest.setPath(dest.path() +
                 QLatin1Char('/') +
                 logicalUrl.fileName());

    if (logicalUrl == dest)
    {
        Q_EMIT signalDone();

        return;
    }

    QSet<QString> destinationNames {
        logicalUrl.fileName().toCaseFolded()
    };

    for (const DItemAssociatedAccessEntry& associated : associatedEntries)
    {
        const QString name = associated.logicalUrl.fileName();
        const QString foldedName = name.toCaseFolded();

        // File Copy flattens one item's associated assets into the same
        // directory. Reject ambiguous names before writing the primary.
        if (name.isEmpty() || destinationNames.contains(foldedName))
        {
            Q_EMIT signalDone();
            return;
        }

        destinationNames.insert(foldedName);
    }

    QUrl sidecarDest = DMetadata::sidecarUrl(dest);

    if      (ok && (d->settings.behavior == FCContainer::CopyFile))
    {
        QFileInfo srcInfo(logicalUrl.toLocalFile());
        QString suffix = srcInfo.suffix().toUpper();

        QMimeDatabase mimeDB;
        QString mimeType(mimeDB.mimeTypeForUrl(logicalUrl).name());

        if (d->settings.changeImageProperties             &&
            (
             mimeType.startsWith(QLatin1String("image/")) ||
             (suffix == QLatin1String("PGF"))             ||
             (suffix == QLatin1String("JXL"))             ||
             (suffix == QLatin1String("AVIF"))            ||
             (suffix == QLatin1String("KRA"))             ||
             (suffix == QLatin1String("HIF"))             ||
             (suffix == QLatin1String("HEIC"))            ||
             (suffix == QLatin1String("HEIF"))
            )
           )
        {
            ok = !cancellation->isCanceled() &&
                 imageResize(physicalUrl.toLocalFile(), dest);

            if (ok && cancellation->isCanceled())
            {
                QFile::remove(dest.toLocalFile());
                ok = false;
            }
        }
        else
        {
            dest = getUrlOrDelete(dest);
            ok   = DFileOperations::copyFileCancellable(
                physicalUrl.toLocalFile(), dest.toLocalFile(),
                [cancellation]()
                {
                    return cancellation->isCanceled();
                });

            if (d->settings.sidecars && !hasPreparedSidecars &&
                DMetadata::hasSidecar(physicalUrl.toLocalFile()))
            {
                sidecarDest = getUrlOrDelete(sidecarDest);
                DFileOperations::copyFileCancellable(
                    DMetadata::sidecarUrl(physicalUrl).toLocalFile(),
                    sidecarDest.toLocalFile(),
                    [cancellation]()
                    {
                        return cancellation->isCanceled();
                    });
            }

            if (d->settings.writeMetadataToFile &&
                (MetaEngineSettings::instance()->settings().metadataWritingMode == MetaEngine::WRITE_TO_SIDECAR_ONLY))
            {
                QScopedPointer<DMetadata> meta(new DMetadata);

                if (meta->load(physicalUrl.toLocalFile()))
                {
                    meta->setMetadataWritingMode(DMetadata::WRITE_TO_FILE_ONLY);
                    meta->save(dest.toLocalFile());
                }
            }
        }

        if (ok && source->entry().fileFacts.available &&
            !DFileOperations::setPermissionsAndModificationTime(
                dest.toLocalFile(), source->entry().fileFacts.permissions,
                source->entry().fileFacts.modificationDate))
        {
            QFile::remove(dest.toLocalFile());
            ok = false;
        }

        if (ok && !associatedEntries.isEmpty())
        {
            const QDir destinationDirectory = QFileInfo(dest.toLocalFile()).dir();

            for (const DItemAssociatedAccessEntry& associated : associatedEntries)
            {
                if (cancellation->isCanceled())
                {
                    ok = false;
                    break;
                }

                const bool sidecar =
                    (associated.role ==
                     static_cast<int>(DItemAssociatedRole::XmpSidecar)) ||
                    (associated.role ==
                     static_cast<int>(DItemAssociatedRole::ConfiguredSidecar));

                if (sidecar && !d->settings.sidecars)
                {
                    continue;
                }

                QUrl associatedDestination = getUrlOrDelete(
                    QUrl::fromLocalFile(destinationDirectory.filePath(
                        associated.logicalUrl.fileName())));
                ok = DFileOperations::copyFileCancellable(
                    associated.physicalUrl.toLocalFile(),
                    associatedDestination.toLocalFile(),
                    [cancellation]()
                    {
                        return cancellation->isCanceled();
                    });

                if (ok && associated.fileFacts.available &&
                    !DFileOperations::setPermissionsAndModificationTime(
                        associatedDestination.toLocalFile(),
                        associated.fileFacts.permissions,
                        associated.fileFacts.modificationDate))
                {
                    QFile::remove(associatedDestination.toLocalFile());
                    ok = false;
                }

                if (!ok)
                {
                    break;
                }
            }
        }
    }
    else if (ok                                                     &&
             ((d->settings.behavior == FCContainer::FullSymLink)    ||
              (d->settings.behavior == FCContainer::RelativeSymLink)))
    {

#ifdef Q_OS_WIN

        dest.setPath(dest.path() + QLatin1String(".lnk"));
        sidecarDest.setPath(sidecarDest.path() + QLatin1String(".lnk"));

#endif

        dest        = getUrlOrDelete(dest);
        sidecarDest = getUrlOrDelete(sidecarDest);

        if (d->settings.behavior == FCContainer::FullSymLink)
        {
            ok = QFile::link(physicalUrl.toLocalFile(),
                             dest.toLocalFile());

            if (d->settings.sidecars &&
                DMetadata::hasSidecar(physicalUrl.toLocalFile()))
            {
                QFile::link(DMetadata::sidecarUrl(physicalUrl).toLocalFile(),
                            sidecarDest.toLocalFile());
            }
        }
        else
        {
            QDir dir(d->settings.destUrl.toLocalFile());
            QString path = dir.relativeFilePath(physicalUrl.toLocalFile());
            QUrl srcUrl  = QUrl::fromLocalFile(path);
            ok           = QFile::link(srcUrl.toLocalFile(),
                                       dest.toLocalFile());

            if (d->settings.sidecars &&
                DMetadata::hasSidecar(physicalUrl.toLocalFile()))
            {
                QFile::link(DMetadata::sidecarUrl(srcUrl).toLocalFile(),
                            sidecarDest.toLocalFile());
            }
        }
    }

    if (ok)
    {
        Q_EMIT signalUrlProcessed(logicalUrl, dest);
    }

    Q_EMIT signalDone();
}

bool FCTask::imageResize(const QString& orgPath, QUrl& destUrl)
{
    QFileInfo fi(orgPath);

    if (!fi.exists() || !fi.isReadable())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error opening input file"
                                         << fi.filePath();
        return false;
    }

    QFileInfo destInfo(destUrl.toLocalFile());
    QFileInfo tmpDir(destInfo.dir().absolutePath());

    if (!tmpDir.exists() || !tmpDir.isWritable())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error opening target folder"
                                         << tmpDir.dir();
        return false;
    }

    DImg img = PreviewLoadThread::loadHighQualitySynchronously(orgPath);

    if (img.isNull())
    {
        img.load(orgPath);
    }

    if (!img.isNull())
    {
        uint sizeFactor = d->settings.imageResize;

        if ((img.width() > sizeFactor) || (img.height() > sizeFactor))
        {
            DImg scaledImg = img.smoothScale(sizeFactor,
                                             sizeFactor,
                                             Qt::KeepAspectRatio);

            if ((scaledImg.width() > sizeFactor) || (scaledImg.height() > sizeFactor))
            {
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Cannot resize image";
                return false;
            }

            img = scaledImg;
        }

        QString destFile = destInfo.path()  +
                           QLatin1Char('/') +
                           destInfo.completeBaseName();

        if      (d->settings.imageFormat == FCContainer::JPEG)
        {
            destFile.append(QLatin1String(".jpg"));
            destUrl  = getUrlOrDelete(QUrl::fromLocalFile(destFile));
            destFile = destUrl.toLocalFile();

            img.setAttribute(QLatin1String("quality"), d->settings.imageCompression);

            if (!img.save(destFile, DImg::JPEG))
            {
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Cannot save resized image (JPEG)";
                return false;
            }
        }
        else if (d->settings.imageFormat == FCContainer::PNG)
        {
            destFile.append(QLatin1String(".png"));
            destUrl  = getUrlOrDelete(QUrl::fromLocalFile(destFile));
            destFile = destUrl.toLocalFile();

            if (!img.save(destFile, DImg::PNG))
            {
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Cannot save resized image (PNG)";
                return false;
            }
        }

        QScopedPointer<DMetadata> meta(new DMetadata);

        if (!meta->load(destFile))
        {
            return false;
        }

        if (d->settings.removeMetadata)
        {
            meta->setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);

            meta->clearExif();
            meta->clearIptc();
            meta->clearXmp();
        }
        else
        {
            meta->setItemOrientation(MetaEngine::ORIENTATION_NORMAL);
        }

        if (!meta->save(destFile))
        {
            return false;
        }

        // Remove possible sidecar file

        if (
            (!d->settings.sidecars        ||
              d->settings.removeMetadata) &&
            QFile::exists(DMetadata::sidecarUrl(destUrl).toLocalFile())
           )
        {
            QFile::remove(DMetadata::sidecarUrl(destUrl).toLocalFile());
        }

        DFileOperations::copyModificationTime(orgPath, destFile);

        return true;
    }

    return false;
}

QUrl FCTask::getUrlOrDelete(const QUrl& fileUrl) const
{
    if (
        d->settings.overwrite              &&
        QFile::exists(fileUrl.toLocalFile())
       )
    {
        QFile::remove(fileUrl.toLocalFile());

        return fileUrl;
    }

    return DFileOperations::getUniqueFileUrl(fileUrl);
}

} // namespace DigikamGenericFileCopyPlugin

#include "moc_fctask.cpp"
