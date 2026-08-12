/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-13
 * Description : Image files selection dialog - list view icon provider.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imagedialog_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImageDialogIconProvider::Private
{

public:

    Private() = default;

public:

    QMutex                mutex;

    QTimer*               timer     = nullptr;
    ThumbnailLoadThread*  thread    = nullptr;    ///< The separated thread to render thumbnail images.

    QHash<QString, QIcon> iconHash;
};

ImageDialogIconProvider::ImageDialogIconProvider()
    : QObject          (),
      QFileIconProvider(),
      d                (new Private)
{
    d->timer  = new QTimer(this);
    d->timer->setInterval(100);
    d->timer->setSingleShot(true);

    d->thread = new ThumbnailLoadThread;
    d->thread->setPixmapRequested(true);
    d->thread->setThumbnailSize(256);

    connect(d->timer, SIGNAL(timeout()),
            this, SIGNAL(signalThumbnailRefresh()));

    connect(d->thread, SIGNAL(signalQImageThumbnailLoaded(LoadingDescription,QImage)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QImage)));
}

ImageDialogIconProvider::~ImageDialogIconProvider()
{
    d->thread->stopAllTasks();
    d->thread->wait();
    d->timer->stop();

    delete d->thread;
    delete d;
}

QIcon ImageDialogIconProvider::icon(const QFileInfo& info) const
{
    // We will only process image files.

    if (info.isFile() && !info.isSymLink() && !info.isRoot())
    {
        QString path = info.absoluteFilePath();

        if (!path.isEmpty())
        {
            {
                QMutexLocker locker(&d->mutex);

                if (d->iconHash.contains(path))
                {
                    return d->iconHash.value(path);
                }
            }

            QString mimeType = QMimeDatabase().mimeTypeForFile(path).name();
            QString suffix   = info.suffix().toUpper();

            if (
                mimeType.startsWith(QLatin1String("image/")) ||
                (suffix == QLatin1String("PGF"))             ||
                (suffix == QLatin1String("JXL"))             ||
                (suffix == QLatin1String("AVIF"))            ||
                (suffix == QLatin1String("KRA"))             ||
                (suffix == QLatin1String("CR3"))             ||
                (suffix == QLatin1String("HIF"))             ||
                (suffix == QLatin1String("HEIC"))            ||
                (suffix == QLatin1String("HEIF"))
               )
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "request thumb icon for " << path;

                d->thread->find(ThumbnailIdentifier(path));
            }
        }
    }

    // For non-iages, we will use default provider implementation.

    return QFileIconProvider::icon(info);
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

QIcon ImageDialogIconProvider::icon(QAbstractFileIconProvider::IconType type) const

#else

QIcon ImageDialogIconProvider::icon(IconType type) const

#endif

{
    return QFileIconProvider::icon(type);
}

void ImageDialogIconProvider::slotThumbnailLoaded(const LoadingDescription& desc, const QImage& img)
{
    // Resize and center pixmap on target icon.

    if (!img.isNull())
    {
        QPixmap pix = QPixmap::fromImage(img);
        pix         = pix.scaled(QSize(256, 256),
                             Qt::KeepAspectRatio,
                             Qt::FastTransformation);

        QPixmap icn(QSize(256, 256));
        icn.fill(Qt::transparent);
        QPainter p(&icn);
        p.drawPixmap((icn.width()  - pix.width() ) / 2,
                     (icn.height() - pix.height()) / 2,
                     pix);

        QMutexLocker locker(&d->mutex);

        d->iconHash.insert(desc.filePath, icn);
    }
    else
    {
        QMutexLocker locker(&d->mutex);

        d->iconHash.insert(desc.filePath, QIcon::fromTheme(QLatin1String("image-missing")));
    }

    if (!d->timer->isActive())
    {
        d->timer->start();
    }
}

} // namespace Digikam
