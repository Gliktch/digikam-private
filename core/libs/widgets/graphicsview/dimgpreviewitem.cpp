/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View items for DImg
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgpreviewitem.h"
#include "dimgitems_p.h"

// Qt includes

#include <QApplication>
#include <QScreen>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "iccsettings.h"
#include "loadingcacheinterface.h"
#include "loadingdescription.h"
#include "previewloadthread.h"
#include "previewsettings.h"

namespace Digikam
{

DImgPreviewItem::DImgPreviewItem(QGraphicsItem* const parent)
    : GraphicsDImgItem(*new DImgPreviewItemPrivate, parent)
{
    Q_D(DImgPreviewItem);

    d->init(this);
}

DImgPreviewItem::DImgPreviewItem(DImgPreviewItemPrivate& dd, QGraphicsItem* const parent)
    : GraphicsDImgItem(dd, parent)
{
    Q_D(DImgPreviewItem);

    d->init(this);
}

void DImgPreviewItem::DImgPreviewItemPrivate::init(DImgPreviewItem* const q)
{
    previewThread = new PreviewLoadThread;
    preloadThread = new PreviewLoadThread;
    preloadThread->setPriority(QThread::LowPriority);

    QObject::connect(previewThread, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
                     q, SLOT(slotGotImagePreview(LoadingDescription,DImg)),
                     Qt::QueuedConnection);

    QObject::connect(previewThread, SIGNAL(signalImageStartedLoading(LoadingDescription)),
                     q, SIGNAL(signalStartedLoading()),
                     Qt::QueuedConnection);

    QObject::connect(previewThread, SIGNAL(signalLoadingProgress(LoadingDescription,float)),
                     q, SLOT(slotLoadingProgress(LoadingDescription,float)),
                     Qt::QueuedConnection);

    QObject::connect(preloadThread, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
                     q, SLOT(preloadNext()),
                     Qt::QueuedConnection);

    // get preview size from screen size, but limit from VGA to WQXGA

    previewSize = qBound(640,
                         qMax(qApp->primaryScreen()->availableGeometry().height(),
                              qApp->primaryScreen()->availableGeometry().width()),
                         2560);

    LoadingCacheInterface::connectToSignalFileChanged(q, SLOT(slotFileChanged(QString)));

    QObject::connect(IccSettings::instance(), SIGNAL(signalICCSettingsChanged(ICCSettingsContainer,ICCSettingsContainer)),
                     q, SLOT(iccSettingsChanged(ICCSettingsContainer,ICCSettingsContainer)));
}

DImgPreviewItem::~DImgPreviewItem()
{
    Q_D(DImgPreviewItem);

    delete d->previewThread;
    delete d->preloadThread;
}

void DImgPreviewItem::setDisplayingWidget(QWidget* const widget)
{
    Q_D(DImgPreviewItem);

    d->previewThread->setDisplayingWidget(widget);
}

void DImgPreviewItem::setPreviewSettings(const PreviewSettings& settings)
{
    Q_D(DImgPreviewItem);

    if (settings == d->previewSettings)
    {
        return;
    }

    d->previewSettings = settings;
    reload();
}

QString DImgPreviewItem::path() const
{
    Q_D(const DImgPreviewItem);

    return d->path;
}

void DImgPreviewItem::setPath(const QString& path, bool rePreview)
{
    Q_D(DImgPreviewItem);

    if ((path == d->path) && !rePreview)
    {
        return;
    }

    d->path = path;

    if (d->path.isNull())
    {
        d->state = NoImage;

        Q_EMIT stateChanged(d->state);
    }
    else
    {
        d->state = Loading;
        d->previewThread->load(d->path, d->previewSettings, d->previewSize);

        Q_EMIT stateChanged(d->state);
    }

    d->preloadThread->stopLoading();
}

void DImgPreviewItem::setPreloadPaths(const QStringList& pathsToPreload)
{
    Q_D(DImgPreviewItem);

    d->pathsToPreload = pathsToPreload;
    preloadNext();
}

static bool approximates(const QSizeF& s1, const QSizeF& s2)
{
    if (s1 == s2)
    {
        return true;
    }

    double widthRatio = s1.width() / s2.width();

    if ((widthRatio < 0.98) || (widthRatio > 1.02))
    {
        return false;
    }

    double heightRatio = s1.height() / s2.height();

    if ((heightRatio < 0.98) || (heightRatio > 1.02))
    {
        return false;
    }

    return true;
}

QString DImgPreviewItem::userLoadingHint() const
{
    Q_D(const DImgPreviewItem);

    switch (d->state)
    {
        case NoImage:
        {
            return QString();
        }

        case Loading:
        {
            return i18n("Loading...");
        }

        case ImageLoaded:
        {
            if (d->image.detectedFormat() == DImg::RAW)
            {
                if      (d->image.attribute(QLatin1String("fromRawEmbeddedPreview")).toBool())
                {
                    return i18n("Embedded JPEG Preview");
                }
                else if (approximates(d->image.originalSize(), d->image.size()))
                {
                    return QString();
                }
                else
                {
                    return i18n("Half Size Raw Preview");
                }
            }
            else
            {
                if (approximates(d->image.originalSize(), d->image.size()))
                {
                    //return i18n("Full Size Preview");
                    return QString();
                }
                else
                {
                    return i18n("Reduced Size Preview");
                }
            }

            return QString();   // To please compiler without warnings.
        }

        default: // ImageLoadingFailed:
        {
            break;
        }
    }

    return i18n("Failed to load image");
}

void DImgPreviewItem::reload()
{
    Q_D(DImgPreviewItem);

    QString pth = d->path;
    d->path.clear();
    setPath(pth);
}

DImgPreviewItem::State DImgPreviewItem::state() const
{
    Q_D(const DImgPreviewItem);

    return d->state;
}

bool DImgPreviewItem::isLoaded() const
{
    Q_D(const DImgPreviewItem);

    return (d->state == ImageLoaded);
}

void DImgPreviewItem::slotLoadingProgress(const LoadingDescription&, float progress)
{
    Q_EMIT signalLoadingProgress(progress);
}

void DImgPreviewItem::slotGotImagePreview(const LoadingDescription& description, const DImg& image)
{
    Q_D(DImgPreviewItem);

    if ((description.filePath != d->path) || description.isThumbnail())
    {
        return;
    }

    if (image.isNull())
    {
        setImage(DImg());

        d->state = ImageLoadingFailed;

        Q_EMIT stateChanged(d->state);
        Q_EMIT loadingFailed();
    }
    else
    {
        setImage(image);

        d->state = ImageLoaded;

        Q_EMIT stateChanged(d->state);
        Q_EMIT loaded();
    }

    Q_EMIT signalLoadingComplete();

    preloadNext();
}

void DImgPreviewItem::preloadNext()
{
    Q_D(DImgPreviewItem);

    if (!isLoaded() || d->pathsToPreload.isEmpty())
    {
        return;
    }

    QString preloadPath = d->pathsToPreload.takeFirst();
    d->preloadThread->load(preloadPath, d->previewSettings, d->previewSize);
}

void DImgPreviewItem::slotFileChanged(const QString& path)
{
    Q_D(DImgPreviewItem);            // cppcheck-suppress constVariablePointer

    if (d->path == path)
    {
        reload();
    }
}

void DImgPreviewItem::iccSettingsChanged(const ICCSettingsContainer& current, const ICCSettingsContainer& previous)
{
    if (
        (current.enableCM           != previous.enableCM)           ||
        (current.useManagedPreviews != previous.useManagedPreviews) ||
        (current.monitorProfile     != previous.monitorProfile)
       )
    {
        reload();
    }
}

} // namespace Digikam

#include "moc_dimgpreviewitem.cpp"
