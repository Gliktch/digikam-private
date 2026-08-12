/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-11
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2007-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "presentationloader.h"

// Qt includes

#include <QMap>
#include <QPainter>
#include <QThread>
#include <QMutex>
#include <QFileInfo>

// Local includes

#include "dimg.h"
#include "iccmanager.h"
#include "iccsettings.h"
#include "actionthreadbase.h"
#include "digikam_debug.h"
#include "previewloadthread.h"
#include "iccsettingscontainer.h"
#include "presentationcontainer.h"

using namespace Digikam;

namespace DigikamGenericPresentationPlugin
{

typedef QMap<QUrl, QImage> LoadedImages;

class Q_DECL_HIDDEN LoadThread : public QThread
{
    Q_OBJECT

public:

    LoadThread(LoadedImages* const loadedImages,
               QMutex* const imageLock,
               QWidget* const display,
               const QUrl& path,
               int width,
               int height)
        : m_imageLock   (imageLock),
          m_loadedImages(loadedImages),
          m_display     (display),
          m_path        (path),
          m_swidth      (width),
          m_sheight     (height)
    {
    }

    ~LoadThread() override = default;

protected:

    void run() override
    {
        ActionThreadBase::setCurrentThreadName(QLatin1String("LoadThread"));       // To customize thread name

        QImage newImage;

        ICCSettingsContainer settings = IccSettings::instance()->settings();

        if (settings.enableCM && settings.useManagedPreviews)
        {
            IccProfile profile(IccManager::displayProfile(m_display));

            newImage = PreviewLoadThread::loadHighQualitySynchronously(m_path.toLocalFile(),
                                                                       PreviewSettings::RawPreviewAutomatic,
                                                                       profile).copyQImage();
        }
        else
        {
            newImage = PreviewLoadThread::loadHighQualitySynchronously(m_path.toLocalFile()).copyQImage();
        }

        m_imageLock->lock();

        if (newImage.isNull())
        {
            m_loadedImages->insert(m_path, newImage);
        }
        else
        {
            m_loadedImages->insert(m_path, newImage.scaled(m_swidth,
                                                           m_sheight,
                                                           Qt::KeepAspectRatio,
                                                           Qt::SmoothTransformation));
        }

        m_imageLock->unlock();
    }

private:

    // Disable
    explicit LoadThread(QObject*);

private:

    QMutex*       m_imageLock       = nullptr;
    LoadedImages* m_loadedImages    = nullptr;
    QWidget*      m_display         = nullptr;
    QUrl          m_path;
    QString       m_filename;
    int           m_swidth          = 0;
    int           m_sheight         = 0;
};

typedef QMap<QUrl, LoadThread*> LoadingThreads;

// -----------------------------------------------------------------------------------------

class Q_DECL_HIDDEN PresentationLoader::Private
{

public:

    Private() = default;

    PresentationContainer* sharedData       = nullptr;
    LoadingThreads*        loadingThreads   = nullptr;
    LoadedImages*          loadedImages     = nullptr;

    QMutex*                imageLock        = nullptr;
    QMutex*                threadLock       = nullptr;

    uint                   cacheSize        = 0;
    int                    currIndex        = 0;
    int                    swidth           = 0;
    int                    sheight          = 0;
};

PresentationLoader::PresentationLoader(PresentationContainer* const sharedData,
                                       int width,
                                       int height,
                                       int beginAtIndex)
    : d(new Private)
{
    d->sharedData     = sharedData;
    d->currIndex      = beginAtIndex;
    d->cacheSize      = d->sharedData->enableCache ? d->sharedData->cacheSize : 1;
    d->swidth         = width;
    d->sheight        = height;
    d->loadingThreads = new LoadingThreads();
    d->loadedImages   = new LoadedImages();
    d->imageLock      = new QMutex();
    d->threadLock     = new QMutex();

    QUrl filePath;

    for (uint i = 0 ; (i < uint(d->cacheSize / 2)) && (i < uint(d->sharedData->urlList.count())) ; ++i)
    {
        filePath                    = d->sharedData->urlList[i];
        LoadThread* const newThread = new LoadThread(d->loadedImages, d->imageLock, d->sharedData->display,
                                                     filePath, d->swidth, d->sheight);
        d->threadLock->lock();
        d->loadingThreads->insert(filePath, newThread);
        newThread->start();
        d->threadLock->unlock();
    }

    for (uint i = 0 ; (i < (((d->cacheSize % 2) == 0) ? (d->cacheSize % 2) : uint(d->cacheSize / 2) + 1)) ; ++i)
    {
        int toLoad                  = (d->currIndex - i) % d->sharedData->urlList.count();
        filePath                    = d->sharedData->urlList[toLoad];
        LoadThread* const newThread = new LoadThread(d->loadedImages, d->imageLock, d->sharedData->display,
                                                     filePath, d->swidth, d->sheight);
        d->threadLock->lock();
        d->loadingThreads->insert(filePath, newThread);
        newThread->start();
        d->threadLock->unlock();
    }
}

PresentationLoader::~PresentationLoader()
{
    d->threadLock->lock();
    LoadingThreads::Iterator it;

    for (it = d->loadingThreads->begin() ; it != d->loadingThreads->end() ; ++it)
    {
        // better check for a valid pointer here

        if (it.value())
        {
            it.value()->wait();
        }

        delete it.value();
    }

    d->loadingThreads->clear();

    d->threadLock->unlock();

    delete d->loadedImages;
    delete d->loadingThreads;
    delete d->imageLock;
    delete d->threadLock;
    delete d;
}

void PresentationLoader::next()
{
    int victim   = (
                    d->currIndex -
                    (
                     ((d->cacheSize % 2) == 0) ? (d->cacheSize / 2) - 1
                                               : int(d->cacheSize / 2)
                    )
                   ) % d->sharedData->urlList.count();

    int newBorn  = (d->currIndex + int(d->cacheSize / 2) + 1) % d->sharedData->urlList.count();
    d->currIndex = (d->currIndex + 1) % d->sharedData->urlList.count();

    if (victim == newBorn)
    {
        return;
    }

    d->threadLock->lock();

    LoadThread* const oldThread = d->loadingThreads->value(d->sharedData->urlList[victim]);

    if (oldThread)
    {
        oldThread->wait();
    }

    delete oldThread;

    d->loadingThreads->remove(d->sharedData->urlList[victim]);
    d->imageLock->lock();
    d->loadedImages->remove(d->sharedData->urlList[victim]);
    d->imageLock->unlock();
    d->threadLock->unlock();

    QUrl filePath               = d->sharedData->urlList[newBorn];
    LoadThread* const newThread = new LoadThread(d->loadedImages, d->imageLock, d->sharedData->display,
                                                 filePath, d->swidth, d->sheight);

    d->threadLock->lock();

    d->loadingThreads->insert(filePath, newThread);
    newThread->start();

    d->threadLock->unlock();
}

void PresentationLoader::prev()
{
    int victim   = (d->currIndex + int(d->currIndex / 2)) % d->sharedData->urlList.count();
    int newBorn  = (
                    d->currIndex -
                    (
                     ((d->cacheSize & 2) == 0) ? (d->cacheSize / 2)
                                               : int(d->cacheSize / 2) + 1
                    )
                   ) % d->sharedData->urlList.count();

    d->currIndex = d->currIndex > 0 ? d->currIndex - 1 : d->sharedData->urlList.count() - 1;

    if (victim == newBorn)
    {
        return;
    }

    d->threadLock->lock();
    d->imageLock->lock();

    LoadThread* const oldThread = d->loadingThreads->value(d->sharedData->urlList[victim]);

    if (oldThread)
    {
        oldThread->wait();
    }

    delete oldThread;

    d->loadingThreads->remove(d->sharedData->urlList[victim]);
    d->loadedImages->remove(d->sharedData->urlList[victim]);

    d->imageLock->unlock();
    d->threadLock->unlock();

    QUrl filePath               = d->sharedData->urlList[newBorn];
    LoadThread* const newThread = new LoadThread(d->loadedImages, d->imageLock, d->sharedData->display,
                                                 filePath, d->swidth, d->sheight);

    d->threadLock->lock();

    d->loadingThreads->insert(filePath, newThread);
    newThread->start();

    d->threadLock->unlock();
}

QImage PresentationLoader::getCurrent() const
{
    checkIsIn(d->currIndex);
    d->imageLock->lock();
    QImage returned = (*d->loadedImages)[d->sharedData->urlList[d->currIndex]];
    d->imageLock->unlock();

    return returned;
}

QString PresentationLoader::currFileName() const
{
    return d->sharedData->urlList[d->currIndex].fileName();
}

QUrl PresentationLoader::currPath() const
{
    return d->sharedData->urlList[d->currIndex];
}

void PresentationLoader::checkIsIn(int index) const
{
    d->threadLock->lock();

    if (d->loadingThreads->contains(d->sharedData->urlList[index]))
    {
        if ((*d->loadingThreads)[d->sharedData->urlList[index]]->isRunning())
        {
            (*d->loadingThreads)[d->sharedData->urlList[index]]->wait();
        }

        d->threadLock->unlock();
    }
    else
    {
        QUrl filePath               = d->sharedData->urlList[index];
        LoadThread* const newThread = new LoadThread(d->loadedImages, d->imageLock, d->sharedData->display,
                                                     filePath, d->swidth, d->sheight);

        d->loadingThreads->insert(d->sharedData->urlList[index], newThread);
        newThread->start();
        (*d->loadingThreads)[d->sharedData->urlList[index]]->wait();
        d->threadLock->unlock();
    }
}

} // namespace DigikamGenericPresentationPlugin

#include "presentationloader.moc"
