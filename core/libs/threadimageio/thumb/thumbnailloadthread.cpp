/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-06-05
 * Description : Thumbnail loading
 *
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "thumbnailloadthread_p.h"

// C++ includes

#include <algorithm>

// Qt includes

#include <QDir>
#include <QRegularExpression>
#include <QSet>
#include <QUrl>
#include <QUrlQuery>

// Local includes

#include "thumbsdb.h"
#include "privacycachetransition.h"

namespace Digikam
{

Q_GLOBAL_STATIC(ThumbnailLoadThreadStaticPriv, static_d)
Q_GLOBAL_STATIC(ThumbnailLoadThread,           defaultObject)
Q_GLOBAL_STATIC(ThumbnailLoadThread,           defaultIconViewObject)

// --- Creating loading descriptions ---

LoadingDescription ThumbnailLoadThread::Private::createLoadingDescription(const ThumbnailIdentifier& identifier,
                                                                          int size,
                                                                          bool setLastDescription)
{
    size = thumbnailSizeForPixmapSize(size);

    LoadingDescription description(identifier.filePath, PreviewSettings(), size,
                                   LoadingDescription::NoColorConversion,
                                   LoadingDescription::PreviewParameters::Thumbnail);
    description.previewParameters.storageReference = identifier.id;

    if (!identifier.filePath.isEmpty() || identifier.id)
    {
        description.resolveSource();
    }

    if (IccSettings::instance()->useManagedPreviews())
    {
        description.postProcessingParameters.colorManagement = LoadingDescription::ConvertForDisplay;
        description.postProcessingParameters.setProfile(static_d->profile);
    }

    if (setLastDescription)
    {
        lastDescriptions.clear();
        lastDescriptions << description;
    }

    return description;
}

LoadingDescription ThumbnailLoadThread::Private::createLoadingDescription(const ThumbnailIdentifier& identifier,
                                                                          int size,
                                                                          const QRect& detailRect,
                                                                          bool setLastDescription)
{
    size                                          = thumbnailSizeForPixmapSize(size);

    LoadingDescription description(identifier.filePath, PreviewSettings(), size,
                                   LoadingDescription::NoColorConversion,
                                   LoadingDescription::PreviewParameters::DetailThumbnail);

    description.previewParameters.storageReference = identifier.id;
    description.previewParameters.extraParameter   = detailRect;

    if (!identifier.filePath.isEmpty() || identifier.id)
    {
        description.resolveSource();
    }

    if (IccSettings::instance()->useManagedPreviews())
    {
        description.postProcessingParameters.colorManagement = LoadingDescription::ConvertForDisplay;
        description.postProcessingParameters.setProfile(static_d->profile);
    }

    if (setLastDescription)
    {
        lastDescriptions.clear();
        lastDescriptions << description;
    }

    return description;
}

// ---------------------------------------------------------------------------

ThumbnailLoadThread::ThumbnailLoadThread(QObject* const parent)
    : ManagedLoadSaveThread(parent),
      d                    (new Private)
{
    static_d->firstThreadCreated = true;
    d->creator                   = new ThumbnailCreator(static_d->storageMethod);

    if (static_d->provider)
    {
        d->creator->setThumbnailInfoProvider(static_d->provider);
    }

    d->creator->setOnlyLargeThumbnails(true);
    d->creator->setRemoveAlphaChannel(true);

    connect(this, SIGNAL(thumbnailsAvailable()),
            this, SLOT(slotThumbnailsAvailable()));
}

ThumbnailLoadThread::~ThumbnailLoadThread()
{
    shutDown();

    delete d->creator;
    delete d;
}

ThumbnailLoadThread* ThumbnailLoadThread::defaultIconViewThread()
{
    return defaultIconViewObject;
}

ThumbnailLoadThread* ThumbnailLoadThread::defaultThread()
{
    return defaultObject;
}

void ThumbnailLoadThread::cleanUp()
{
    // NOTE: Nothing to do with Qt5 and Q_GLOBAL_STATIC. Qt clean up all automatically at end of application instance.
    // But stopping all running tasks to prevent a crash at end.

    defaultIconViewThread()->stopAllTasks();
    defaultThread()->stopAllTasks();

    defaultIconViewThread()->wait();
    defaultThread()->wait();
}

void ThumbnailLoadThread::initializeNoThumbnailStorage()
{
    if (static_d->firstThreadCreated)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Call initializeNoThumbnailStorage at application start. "
                                        "There are already thumbnail loading threads created, "
                                        "and these will not be switched to disable storage.";
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "No storage of thumbnails in the disk cache";
    static_d->storageMethod = ThumbnailCreator::NoThumbnailStorage;
}

void ThumbnailLoadThread::initializeThumbnailDatabase(const DbEngineParameters& params,
                                                      ThumbnailInfoProvider* const provider)
{
    if (static_d->firstThreadCreated)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Call initializeThumbnailDatabase at application start. "
                                        "There are already thumbnail loading threads created, "
                                        "and these will not be switched to use the database.";
    }

    ThumbsDbAccess::setParameters(params);

    if (ThumbsDbAccess::checkReadyForUse(nullptr))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Thumbnails database ready for use";
        static_d->storageMethod = ThumbnailCreator::ThumbnailDatabase;
        static_d->provider      = provider;
    }
    else
    {
        QMessageBox::information(qApp->activeWindow(),
                                 i18nc("@title:window",
                                       "Failed to Initialize Thumbnails Database"),
                                 i18n("Error message: %1", ThumbsDbAccess().lastError()));
    }
}

bool ThumbnailLoadThread::privacyLegacyDetailRectangles(
    const PrivacyCacheTransitionToken& token, QList<QRect>* const rectangles)
{
    if (!rectangles)
    {
        return false;
    }

    rectangles->clear();
    const QString cleanPath = QDir::cleanPath(token.logicalFilePath());

    if (!PrivacyCacheTransition::isActive(token) ||
        !QDir::isAbsolutePath(cleanPath) ||
        (static_d->storageMethod != ThumbnailCreator::ThumbnailDatabase) ||
        !ThumbsDbAccess::isInitialized())
    {
        return false;
    }

    QStringList identifiers;
    ThumbsDbAccess access;

    if (!access.db() ||
        (access.db()->customIdentifiers(&identifiers) != BdEngineBackend::NoErrors))
    {
        return false;
    }

    static const QRegularExpression rectExpression(
        QLatin1String("^(-?[0-9]+),(-?[0-9]+)-([1-9][0-9]*)x([1-9][0-9]*)$"));
    QSet<QRect> uniqueRectangles;

    for (const QString& identifier : std::as_const(identifiers))
    {
        const QUrl url(identifier);

        if (!url.isValid() || (url.scheme() != QLatin1String("detail")))
        {
            continue;
        }

        const QString identifierPath = QDir::cleanPath(url.path());

        if (identifierPath != cleanPath)
        {
            continue;
        }

        const QUrlQuery query(url);
        const QList<QPair<QString, QString> > items = query.queryItems();

        if ((items.size() != 1) ||
            (items.constFirst().first != QLatin1String("rect")))
        {
            return false;
        }

        const QRegularExpressionMatch match =
            rectExpression.match(items.constFirst().second);

        if (!match.hasMatch())
        {
            return false;
        }

        bool valuesValid = false;
        const int x = match.captured(1).toInt(&valuesValid);

        if (!valuesValid)
        {
            return false;
        }

        const int y = match.captured(2).toInt(&valuesValid);

        if (!valuesValid)
        {
            return false;
        }

        const int width = match.captured(3).toInt(&valuesValid);

        if (!valuesValid)
        {
            return false;
        }

        const int height = match.captured(4).toInt(&valuesValid);
        const QRect rect(x, y, width, height);

        if (!valuesValid || !rect.isValid() || rect.isEmpty())
        {
            return false;
        }

        QUrl canonical = QUrl::fromLocalFile(cleanPath);
        canonical.setScheme(QLatin1String("detail"));
        QUrlQuery canonicalQuery;
        canonicalQuery.addQueryItem(QLatin1String("rect"),
                                    items.constFirst().second);
        canonical.setQuery(canonicalQuery);

        if (canonical.toString() != identifier)
        {
            return false;
        }

        uniqueRectangles.insert(rect);
    }

    *rectangles = uniqueRectangles.values();
    std::sort(rectangles->begin(), rectangles->end(),
              [](const QRect& first, const QRect& second)
              {
                  if (first.x() != second.x())
                  {
                      return first.x() < second.x();
                  }

                  if (first.y() != second.y())
                  {
                      return first.y() < second.y();
                  }

                  if (first.width() != second.width())
                  {
                      return first.width() < second.width();
                  }

                  return first.height() < second.height();
              });

    return true;
}

void ThumbnailLoadThread::setDisplayingWidget(QWidget* const widget)
{
    static_d->profile = IccManager::displayProfile(widget);
}

void ThumbnailLoadThread::setThumbnailSize(int size, bool forFace)
{
    d->size = size;

    if (forFace)
    {
        d->creator->setThumbnailSize(size);
    }
}

int ThumbnailLoadThread::maximumThumbnailSize()
{
    return ThumbnailSize::maxThumbsSize();
}

int ThumbnailLoadThread::maximumThumbnailPixmapSize(bool highlight)
{
    if (highlight)
    {
        return ThumbnailSize::maxThumbsSize();
    }
    else
    {
        return ThumbnailSize::maxThumbsSize() + 2;    // see slotThumbnailLoaded
    }
}

void ThumbnailLoadThread::setSendSurrogatePixmap(bool send)
{
    d->sendSurrogate = send;
}

void ThumbnailLoadThread::setPixmapRequested(bool wantPixmap)
{
    d->wantPixmap = wantPixmap;
}

void ThumbnailLoadThread::setHighlightPixmap(bool highlight)
{
    d->highlight = highlight;
}

ThumbnailCreator* ThumbnailLoadThread::thumbnailCreator() const
{
    return d->creator;
}

int ThumbnailLoadThread::thumbnailToPixmapSize(int size) const
{
    return d->pixmapSizeForThumbnailSize(size);
}

int ThumbnailLoadThread::thumbnailToPixmapSize(bool withHighlight, int size)
{
    if (withHighlight && (size >= 10))
    {
        return (size + 2);
    }

    return size;
}

int ThumbnailLoadThread::pixmapToThumbnailSize(int size) const
{
    return d->thumbnailSizeForPixmapSize(size);
}

bool ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier,
                               int size,
                               QPixmap* retPixmap,
                               bool emitSignal,
                               const QRect& detailRect,
                               bool onlyStorage)
{
    QPixmap pix;
    LoadingDescription description;

    if (detailRect.isNull())
    {
        description = d->createLoadingDescription(identifier, size);
    }
    else
    {
        description = d->createLoadingDescription(identifier, size, detailRect);
    }

    if (onlyStorage)
    {
        description.previewParameters.flags |= LoadingDescription::PreviewParameters::OnlyFromStorage;
    }

    QString cacheKey = description.cacheKey();

    if (!description.isSourceDenied() &&
        description.sourceResolutionIsCurrent())
    {
        {
            LoadingCache* const cache = LoadingCache::cache();
            LoadingCache::CacheLock lock(cache);
            const QPixmap* cachePix   = cache->retrieveThumbnailPixmap(cacheKey);

            if (cachePix)
            {
                pix = *cachePix;
            }
        }

        if (!description.sourceResolutionIsCurrent())
        {
            pix = QPixmap();
        }
    }

    if (!pix.isNull()                              &&
        !description.isSourceDenied()              &&
        description.sourceResolutionIsCurrent())
    {
        if (retPixmap)
        {
            *retPixmap = pix;
        }

        if (emitSignal)
        {
            load(description);

            if (description.isSourceDenied() ||
                !description.sourceResolutionIsCurrent())
            {
                if (retPixmap)
                {
                    *retPixmap = QPixmap();
                }

                return false;
            }

            Q_EMIT signalThumbnailLoaded(description, pix);
        }

        return true;
    }

    if (!description.isSourceDenied() &&
        description.sourceResolutionIsCurrent())
    {
        // If there is a result waiting for conversion to pixmap, return false - pixmap will come shortly

        QMutexLocker lock(&d->resultsMutex);

        if (d->collectedResults.contains(cacheKey))
        {
            return false;
        }
    }

    load(description);

    return false;
}

// --- Normal thumbnails ---

bool ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier,
                               QPixmap& retPixmap, int size, bool onlyStorage)
{
    return find(identifier, size, &retPixmap, false, QRect(), onlyStorage);
}

bool ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier, QPixmap& retPixmap)
{
    return find(identifier, retPixmap, d->size);
}

void ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier)
{
    find(identifier, d->size);
}

void ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier, int size)
{
    find(identifier, size, nullptr, true, QRect());
}

void ThumbnailLoadThread::findGroup(QList<ThumbnailIdentifier>& identifiers)
{
    findGroup(identifiers, d->size);
}

void ThumbnailLoadThread::findGroup(QList<ThumbnailIdentifier>& identifiers, int size)
{
    if (!checkSize(size))
    {
        return;
    }

    QList<LoadingDescription> descriptions = d->makeDescriptions(identifiers, size);
    ManagedLoadSaveThread::prependThumbnailGroup(descriptions);
}

// --- Detail thumbnails ---

bool ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier, const QRect& rect, QPixmap& pixmap)
{
    return find(identifier, rect, pixmap, d->size);
}

bool ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier,
                               const QRect& rect, QPixmap& pixmap, int size, bool onlyStorage)
{
    return find(identifier, size, &pixmap, false, rect, onlyStorage);
}

void ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier, const QRect& rect)
{
    find(identifier, rect, d->size);
}

void ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier, const QRect& rect, int size)
{
    find(identifier, size, nullptr, true, rect);
}

void ThumbnailLoadThread::findGroup(const QList<QPair<ThumbnailIdentifier, QRect> >& idsAndRects)
{
    findGroup(idsAndRects, d->size);
}

void ThumbnailLoadThread::findGroup(const QList<QPair<ThumbnailIdentifier, QRect> >& idsAndRects, int size)
{
    if (!checkSize(size))
    {
        return;
    }

    QList<LoadingDescription> descriptions = d->makeDescriptions(idsAndRects, size);
    ManagedLoadSaveThread::prependThumbnailGroup(descriptions);
}

bool ThumbnailLoadThread::findBuffered(const ThumbnailIdentifier& identifier,
                                       const QRect& rect, QPixmap& pixmap, int size)
{
    LoadingDescription description;
    bool found = false;

    if (rect.isNull())
    {
        description = d->createLoadingDescription(identifier, size);
    }
    else
    {
        description = d->createLoadingDescription(identifier, size, rect);
    }

    QString cacheKey = description.cacheKey();

    if (description.isSourceDenied() ||
        !description.sourceResolutionIsCurrent())
    {
        return false;
    }

    {
        LoadingCache* const cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        const QPixmap* cachePix   = cache->retrieveBufferedTPixmap(cacheKey);

        if (cachePix)
        {
            pixmap = *cachePix;
            found  = true;
        }
    }

    if (!description.sourceResolutionIsCurrent())
    {
        pixmap = QPixmap();

        return false;
    }

    return found;
}

// --- Preloading ---

void ThumbnailLoadThread::preload(const ThumbnailIdentifier& identifier)
{
    preload(identifier, d->size);
}

void ThumbnailLoadThread::preload(const ThumbnailIdentifier& identifier, int size)
{
    LoadingDescription description = d->createLoadingDescription(identifier, size);

    if (d->checkDescription(description))
    {
        load(description, true);
    }
}

void ThumbnailLoadThread::preloadGroup(QList<ThumbnailIdentifier>& identifiers)
{
    preloadGroup(identifiers, d->size);
}

void ThumbnailLoadThread::preloadGroup(QList<ThumbnailIdentifier>& identifiers, int size)
{
    if (!checkSize(size))
    {
        return;
    }

    QList<LoadingDescription> descriptions = d->makeDescriptions(identifiers, size);
    ManagedLoadSaveThread::preloadThumbnailGroup(descriptions);
}

void ThumbnailLoadThread::pregenerateGroup(const QList<ThumbnailIdentifier>& identifiers)
{
    pregenerateGroup(identifiers, d->size);
}

void ThumbnailLoadThread::pregenerateGroup(const QList<ThumbnailIdentifier>& identifiers, int size)
{
    if (!checkSize(size))
    {
        return;
    }

    QList<LoadingDescription> descriptions = d->makeDescriptions(identifiers, size);

    for (int i = 0 ; i < descriptions.size() ; ++i)
    {
        descriptions[i].previewParameters.flags |= LoadingDescription::PreviewParameters::OnlyPregenerate;
    }

    ManagedLoadSaveThread::preloadThumbnailGroup(descriptions);
}

// --- Basic load() ---

void ThumbnailLoadThread::load(const LoadingDescription& desc)
{
    load(desc, false);
}

void ThumbnailLoadThread::load(const LoadingDescription& description, bool preload)
{
    if (!checkSize(description.previewParameters.size))
    {
        return;
    }

    if (preload)
    {
        ManagedLoadSaveThread::preloadThumbnail(description);
    }
    else
    {
        ManagedLoadSaveThread::loadThumbnail(description);
    }
}

QList<LoadingDescription> ThumbnailLoadThread::lastDescriptions() const
{
    return d->lastDescriptions;
}

bool ThumbnailLoadThread::checkSize(int size)
{
    size             = d->thumbnailSizeForPixmapSize(size);
    double dpr       = qApp->devicePixelRatio();
    int maxThumbSize = (dpr > 1.0) ? ThumbnailSize::MAX
                                   : ThumbnailSize::maxThumbsSize();

    if      (size <= 0)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "ThumbnailLoadThread::load: No thumbnail size specified. Refusing to load thumbnail.";
        return false;
    }
    else if (size > maxThumbSize)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "ThumbnailLoadThread::load: Thumbnail size " << size
                                     << " is larger than " << maxThumbSize << ". Refusing to load.";
        return false;
    }

    return true;
}

// --- Receiving ---

/**
 * virtual method overridden from LoadSaveNotifier, implemented first by LoadSaveThread
 * called by ThumbnailTask from working thread
 */
void ThumbnailLoadThread::thumbnailLoaded(const LoadingDescription& loadingDescription, const QImage& img)
{
    const bool sourceCurrent = !loadingDescription.isSourceDenied() &&
                               loadingDescription.sourceResolutionIsCurrent();
    const QImage safeImage = sourceCurrent ? img
                                           : QImage();

    // call parent to send signalThumbnailLoaded(LoadingDescription, QImage) - signal is part of public API

    ManagedLoadSaveThread::thumbnailLoaded(loadingDescription, safeImage);

    if (!d->wantPixmap)
    {
        return;
    }

    // Store result in our list and fire one signal
    // This means there can be several results per pixmap,
    // to speed up cases where inter-thread communication is the limiting factor

    const bool sourceStillCurrent = !loadingDescription.isSourceDenied() &&
                                    loadingDescription.sourceResolutionIsCurrent();
    const QImage collectedImage = sourceStillCurrent ? safeImage
                                                     : QImage();

    QMutexLocker lock(&d->resultsMutex);
    d->collectedResults.insert(loadingDescription.cacheKey(),
                               ThumbnailResult(loadingDescription, collectedImage));

    // only sent signal when flag indicates there is no signal on the way currently

    if (!d->notifiedForResults)
    {
        d->notifiedForResults = true;

        Q_EMIT thumbnailsAvailable();
    }
}

void ThumbnailLoadThread::slotThumbnailsAvailable()
{
    // harvest collected results safely into our thread

    QList<ThumbnailResult> results;
    {
        QMutexLocker lock(&d->resultsMutex);
        results               = d->collectedResults.values();
        d->collectedResults.clear();

        // reset flag so that for next result, the signal is sent again

        d->notifiedForResults = false;
    }

    for (const ThumbnailResult& result : std::as_const(results))
    {
        slotThumbnailLoaded(result.loadingDescription, result.image);
    }
}

void ThumbnailLoadThread::slotThumbnailLoaded(const LoadingDescription& description, const QImage& thumb)
{
    QPixmap pix;
    const auto sourceIsCurrent = [&description]()
    {
        return (!description.isSourceDenied() &&
                description.sourceResolutionIsCurrent());
    };

    if (thumb.isNull() || !sourceIsCurrent())
    {
        pix = surrogatePixmap(description);
    }
    else
    {
        int w = thumb.width();
        int h = thumb.height();

        // highlight only when requested and when thumbnail
        // width and height are greater than 10

        if (d->highlight && ((w >= 10) && (h >= 10)))
        {
            pix = QPixmap(w + 2, h + 2);
            QPainter p(&pix);
            p.setPen(QPen(Qt::black, 1));
            p.drawRect(0, 0, w + 1, h + 1);
            p.drawImage(1, 1, thumb);
        }
        else
        {
            pix = QPixmap::fromImage(thumb);
        }
    }

    if (!sourceIsCurrent())
    {
        pix = surrogatePixmap(description);
    }

    // put into cache

    if (!pix.isNull() && sourceIsCurrent())
    {
        LoadingCache* const cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        cache->putThumbnail(description.cacheKey(), pix, description.filePath);
    }

    if (!sourceIsCurrent())
    {
        pix = surrogatePixmap(description);
    }

    Q_EMIT signalThumbnailLoaded(description, pix);
}

QPixmap ThumbnailLoadThread::surrogatePixmap(const LoadingDescription& description)
{
    if (!d->sendSurrogate)
    {
        return QPixmap();
    }

    QPixmap pix;

    const bool sourceMayBeInspected = !description.isSourceDenied() &&
                                      description.sourceResolutionIsCurrent();
    const QMimeDatabase::MatchMode matchMode = sourceMayBeInspected
                                                   ? QMimeDatabase::MatchDefault
                                                   : QMimeDatabase::MatchExtension;
    QMimeType mimeType = QMimeDatabase().mimeTypeForFile(description.filePath, matchMode);

    if (mimeType.isValid())
    {
        pix = QIcon::fromTheme(mimeType.genericIconName()).pixmap(128);
    }

    if (pix.isNull())
    {
        pix = QIcon::fromTheme(QLatin1String("application-x-zerosize")).pixmap(128);
    }

    if (pix.isNull())
    {
        // give up
        return QPixmap();
    }

    // Resize icon to the right size depending of current settings.

    QSize size(pix.size());
    size.scale(description.previewParameters.size, description.previewParameters.size, Qt::KeepAspectRatio);

    if (!pix.isNull() && (size.width() < pix.width()) && (size.height() < pix.height()))
    {
        // only scale down
        // do not scale up, looks bad

        pix = pix.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    return pix;
}

// --- Utilities ---

void ThumbnailLoadThread::storeDetailThumbnail(const QString& filePath, const QRect& detailRect, const QImage& image, bool isFace)
{
    Q_UNUSED(isFace);
    d->creator->storeDetailThumbnail(filePath, detailRect, image);
}

int ThumbnailLoadThread::storedSize() const
{
    return d->creator->storedSize();
}

void ThumbnailLoadThread::deleteThumbnail(const QString& filePath)
{
    deleteThumbnail(ThumbnailIdentifier(filePath));
}

void ThumbnailLoadThread::deleteThumbnail(const ThumbnailIdentifier& identifier)
{
    {
        LoadingCache* const cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        cache->removeThumbnailsForFilePath(identifier.filePath);
    }

    deleteThumbnailFromPersistentCache(identifier);
}

bool ThumbnailLoadThread::deleteThumbnailFromPersistentCache(
    const ThumbnailIdentifier& identifier,
    const QRect& detailRect)
{
    ThumbnailCreator creator(static_d->storageMethod);

    if (static_d->provider)
    {
        creator.setThumbnailInfoProvider(static_d->provider);
    }

    return creator.deleteThumbnailsFromDisk(identifier, detailRect);
}

ThumbnailImageCatcher::ThumbnailImageCatcher(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
}

ThumbnailImageCatcher::ThumbnailImageCatcher(ThumbnailLoadThread* const thread, QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    setThumbnailLoadThread(thread);
}

ThumbnailImageCatcher::~ThumbnailImageCatcher()
{
    delete d;
}

ThumbnailLoadThread* ThumbnailImageCatcher::thread() const
{
    return d->thread;
}

void ThumbnailImageCatcher::setThumbnailLoadThread(ThumbnailLoadThread* const thread)
{
    if (d->thread == thread)
    {
        return;
    }

    d->state = Private::Inactive;

    if (d->thread)
    {
        disconnect(d->thread, SIGNAL(signalQImageThumbnailLoaded(LoadingDescription,QImage)),
                   this, SLOT(slotThumbnailLoaded(LoadingDescription,QImage)));
    }

    d->thread = thread;

    {
        QMutexLocker lock(&d->mutex);
        d->reset();
    }

    if (d->thread)
    {
        connect(d->thread, SIGNAL(signalQImageThumbnailLoaded(LoadingDescription,QImage)),
                this, SLOT(slotThumbnailLoaded(LoadingDescription,QImage)),
                Qt::DirectConnection);
    }
}

void ThumbnailImageCatcher::setActive(bool active)
{
    if (d->active == active)
    {
        return;
    }

    if (!active)
    {
        cancel();
    }

    QMutexLocker lock(&d->mutex);
    d->active = active;
    d->reset();
}

void ThumbnailImageCatcher::cancel()
{
    QMutexLocker lock(&d->mutex);

    if (d->state == Private::Waiting)
    {
        d->state = Private::Quitting;
        d->condVar.wakeOne();
    }
}

void ThumbnailImageCatcher::slotThumbnailLoaded(const LoadingDescription& description, const QImage& image)
{
    // We are in the thumbnail thread here, DirectConnection!

    QMutexLocker lock(&d->mutex);

    switch (d->state)
    {
        case Private::Inactive:
        {
            break;
        }

        case Private::Accepting:
        {
            d->intermediate << Private::CatcherResult(description, image);
            break;
        }

        case Private::Waiting:
        {
            d->harvest(description, image);
            break;
        }

        case Private::Quitting:
        {
            break;
        }
    }
}

int ThumbnailImageCatcher::enqueue()
{
    QList<LoadingDescription> descriptions = d->thread->lastDescriptions();

    QMutexLocker lock(&d->mutex);

    for (const LoadingDescription& description : std::as_const(descriptions))
    {
        d->tasks << Private::CatcherResult(description);
    }

    return descriptions.size();
}

QList<QImage> ThumbnailImageCatcher::waitForThumbnails()
{
    if (!d->thread || d->tasks.isEmpty() || !d->active)
    {
        return QList<QImage>();
    }

    QMutexLocker lock(&d->mutex);
    d->state = Private::Waiting;

    // first, handle results received between request and calling this method

    for (const Private::CatcherResult& result : std::as_const(d->intermediate))
    {
        d->harvest(result.description, result.image);
    }

    d->intermediate.clear();

    // Now wait for the rest to arrive. If already finished, state will be Quitting

    while (d->state == Private::Waiting)
    {
        d->condVar.wait(&d->mutex);
    }

    QList<QImage> result;

    for (const Private::CatcherResult& task : std::as_const(d->tasks))
    {
        result << task.image;
    }

    d->reset();

    return result;
}

} // namespace Digikam

#include "moc_thumbnailloadthread.cpp"
