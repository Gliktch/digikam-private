/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : slide show tool using preview of pictures.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2004 by Enrico Ros <eros dot kde at email dot it>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "slideshowloader.h"

// Qt includes

#include <QMimeDatabase>
#include <QApplication>
#include <QScreen>
#include <QWindow>
#include <QCursor>
#include <QTimer>
#include <QColor>
#include <QFont>

#ifdef HAVE_DBUS
#   include <QDBusConnection>
#   include <QDBusMessage>
#   include <QDBusReply>
#endif

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "slidetoolbar.h"
#include "slideimage.h"
#include "slideerror.h"
#include "slideosd.h"
#include "slideend.h"

#ifdef HAVE_MEDIAPLAYER
#   include "slidevideo.h"
#endif //HAVE_MEDIAPLAYER

#ifdef Q_OS_WIN
#   include "windows.h"
#endif

#ifdef Q_OS_MACOS
#   include <IOKit/pwr_mgt/IOPMLib.h>
#endif

using namespace Digikam;

namespace DigikamGenericSlideShowPlugin
{

class Q_DECL_HIDDEN SlideShowLoader::Private
{
public:

    Private() = default;

public:

    int                    fileIndex            = -1;
    int                    screenSaverCookie    = -1;

    QTimer*                mouseMoveTimer       = nullptr;  ///< To hide cursor when not moved.

    SlideImage*            imageView            = nullptr;

#ifdef HAVE_MEDIAPLAYER

    SlideVideo*            videoView            = nullptr;

#endif

    SlideError*            errorView            = nullptr;
    SlideEnd*              endView              = nullptr;
    SlideOSD*              osd                  = nullptr;

    SlideShowSettings*     settings             = nullptr;

    QUrl                   startUrl;                        ///< The item to start with, if any.

    QMap<QString, QString> shortcutPrefixes;

#ifdef Q_OS_MACOS

    IOPMAssertionID        powerAssertion       = 0;        ///< For the screen saver lock/unlock under macOS.

#endif

};

SlideShowLoader::SlideShowLoader(SlideShowSettings* const settings)
    : QStackedWidget(nullptr),
      d             (new Private)
{
    d->settings = settings;

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::FramelessWindowHint);
    setContextMenuPolicy(Qt::PreventContextMenu);
    setWindowState(windowState() | Qt::WindowFullScreen);

    setWindowTitle(i18nc("@title:window", "Slideshow"));
    setObjectName(QLatin1String("Slideshow"));
    setMouseTracking(true);

    // ---------------------------------------------------------------

    d->errorView = new SlideError(this);

    insertWidget(ErrorView, d->errorView);

    // ---------------------------------------------------------------

    d->imageView = new SlideImage(this);
    d->imageView->setSlideShowSettings(d->settings);

    connect(d->imageView, SIGNAL(signalImageLoaded(bool)),
            this, SLOT(slotImageLoaded(bool)));

    insertWidget(ImageView, d->imageView);

    // ---------------------------------------------------------------

#ifdef HAVE_MEDIAPLAYER

    d->videoView = new SlideVideo(this);
    d->videoView->setInfoInterface(d->settings->iface);

    insertWidget(VideoView, d->videoView);

#endif

    // ---------------------------------------------------------------

    d->endView = new SlideEnd(this);

    insertWidget(EndView, d->endView);

    // ---------------------------------------------------------------

    d->osd = new SlideOSD(d->settings, this);

    // ---------------------------------------------------------------

    d->mouseMoveTimer = new QTimer(this);
    d->mouseMoveTimer->setSingleShot(true);
    d->mouseMoveTimer->setInterval(1000);

    connect(d->mouseMoveTimer, SIGNAL(timeout()),
            this, SLOT(slotMouseMoveTimeOut()));

    // ---------------------------------------------------------------

    d->errorView->installEventFilter(this);
    d->imageView->installEventFilter(this);
    d->endView->installEventFilter(this);
    d->osd->installEventFilter(this);

#ifdef HAVE_MEDIAPLAYER

    d->videoView->installEventFilter(this);

    connect(d->videoView, SIGNAL(signalVideoLoaded(bool)),
            this, SLOT(slotVideoLoaded(bool)));

    connect(d->videoView, SIGNAL(signalVideoFinished()),
            this, SLOT(slotVideoFinished()));

    connect(d->videoView, SIGNAL(signalVideoPosition(qint64)),
            d->osd, SLOT(slotPositionChanged(qint64)));

    connect(d->videoView, SIGNAL(signalVideoDuration(qint64)),
            d->osd, SLOT(slotDurationChanged(qint64)));

    connect(d->videoView, SIGNAL(signalVideoVolume(int)),
            d->osd, SLOT(slotVolumeChanged(int)));

    connect(d->osd, SIGNAL(signalVideoPosition(int)),
            d->videoView, SLOT(slotPositionChanged(int)));

    connect(d->osd, SIGNAL(signalVideoVolume(int)),
            d->videoView, SLOT(slotVolumeChanged(int)));

#endif

    // ---------------------------------------------------------------

    QScreen* screen = qApp->primaryScreen();

    if (QWidget* const widget = qApp->activeWindow())
    {
        if (QWindow* const window = widget->windowHandle())
        {
            screen = window->screen();
        }
    }

    const int activeScreenIndex = qMax(qApp->screens().indexOf(screen), 0);
    const int preferenceScreen  = d->settings->slideScreen;
    int screenIndex             = 0;

    if      (preferenceScreen == -2)
    {
        screenIndex = activeScreenIndex;
    }
    else if (preferenceScreen == -1)
    {
        QScreen* const primaryScreen = qApp->primaryScreen();
        screenIndex                  = qApp->screens().indexOf(primaryScreen);
    }
    else if ((preferenceScreen >= 0) && (preferenceScreen < qApp->screens().count()))
    {
        screenIndex = preferenceScreen;
    }
    else
    {
        screenIndex              = activeScreenIndex;
        d->settings->slideScreen = -2;
        d->settings->writeToConfig();
    }

    slotScreenSelected(screenIndex);

    // ---------------------------------------------------------------

    inhibitScreenSaver();
    slotMouseMoveTimeOut();
    setCurrentIndex(ImageView);
}

SlideShowLoader::~SlideShowLoader()
{
    Q_EMIT signalLastItemUrl(currentItem());

    d->mouseMoveTimer->stop();

    allowScreenSaver();

    delete d->settings;
    delete d;
}

void SlideShowLoader::setCurrentView(SlideShowViewMode view)
{
    switch (view)
    {
        case ErrorView:
        {
            d->osd->video(false);
            d->errorView->setCurrentUrl(currentItem());

            setCurrentIndex(view);
            d->osd->setCurrentUrl(currentItem());

            break;
        }

        case ImageView:
        {

#ifdef HAVE_MEDIAPLAYER

            d->videoView->stop();
            d->osd->video(false);

#endif

            setCurrentIndex(view);
            d->osd->setCurrentUrl(currentItem());

            break;
        }

#ifdef HAVE_MEDIAPLAYER

        case VideoView:
        {
            d->osd->video(true);
            d->osd->pause(false);
            setCurrentIndex(view);
            d->osd->setCurrentUrl(currentItem());

            break;
        }

#endif

        default: // EndView
        {

#ifdef HAVE_MEDIAPLAYER

            d->videoView->stop();
            d->osd->video(false);

#endif

            d->osd->pause(true);
            setCurrentIndex(view);

            break;
        }
    }
}

void SlideShowLoader::setCurrentItem(const QUrl& url)
{
    int index = d->settings->indexOf(url);

    if (index != -1)
    {
        d->startUrl  = url;
        d->fileIndex = index - 1;
    }
}

void SlideShowLoader::resolveStartItem()
{
    if (!d->startUrl.isEmpty())
    {
        setCurrentItem(d->startUrl);
    }
}

void SlideShowLoader::reshuffleImages()
{
    QUrl url  = currentItem();

    d->settings->suffleImages();

    int index = d->settings->indexOf(url);

    if (index != -1)
    {
        d->fileIndex = index;
    }
}

QUrl SlideShowLoader::currentItem() const
{
    return d->settings->fileList.value(d->fileIndex);
}

void SlideShowLoader::setShortCutPrefixes(const QMap<QString, QString>& prefixes)
{
    d->shortcutPrefixes = prefixes;
}

void SlideShowLoader::slotLoadNextItem()
{
    int num = d->settings->count();

    if (d->fileIndex == (num - 1))
    {
        if (d->settings->loop)
        {
            d->fileIndex = -1;
        }
    }

    d->fileIndex++;

    loadCurrentItem();
}

void SlideShowLoader::slotLoadPrevItem()
{
    int num = d->settings->count();

    if (d->fileIndex == 0)
    {
        if (d->settings->loop)
        {
            d->fileIndex = num;
        }
    }

    d->fileIndex--;

    loadCurrentItem();
}

void SlideShowLoader::loadCurrentItem()
{
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "fileIndex: " << d->fileIndex;

    int num = d->settings->count();

    if (!d->settings->loop)
    {
        d->osd->toolBar()->setEnabledPrev(d->fileIndex > 0);
        d->osd->toolBar()->setEnabledNext(d->fileIndex < (num - 1));
    }

    d->osd->toolBar()->setEnabledDelete((d->fileIndex >= 0) && (d->fileIndex < num));

    if ((d->fileIndex >= 0) && (d->fileIndex < num))
    {

#ifdef HAVE_MEDIAPLAYER

        QMimeDatabase mimeDB;

        if (mimeDB.mimeTypeForFile(currentItem().toLocalFile())
                                   .name().startsWith(QLatin1String("video/")))
        {
            d->videoView->setCurrentUrl(currentItem());

            return;
        }

#endif

        d->imageView->setLoadUrl(currentItem());
    }
    else
    {
        endOfSlide();
    }
}

void SlideShowLoader::slotImageLoaded(bool loaded)
{
    if (loaded)
    {
        setCurrentView(ImageView);

        if (d->fileIndex != -1)
        {
            if (!d->osd->isPaused())
            {
                d->osd->pause(false);
            }

            preloadNextItem();
        }
    }
    else
    {

#ifdef HAVE_MEDIAPLAYER

        // Try to load only GIF Images

        QMimeDatabase mimeDB;

        if (mimeDB.mimeTypeForFile(currentItem().toLocalFile())
                                   .name() == QLatin1String("image/gif"))
        {
            d->videoView->setCurrentUrl(currentItem());
        }

#else

        preloadNextItem();

#endif

    }

    d->osd->setLoadingReady(true);
}

void SlideShowLoader::slotRemoveImageFromList()
{
    QUrl url = currentItem();

    // Delete or move to trash by url

    d->settings->iface->deleteImage(url);

    // Delete from list of slide show

    d->settings->fileList.removeOne(url);

    loadCurrentItem();
}

void SlideShowLoader::slotVideoLoaded(bool loaded)
{
    if (loaded)
    {

#ifdef HAVE_MEDIAPLAYER

        setCurrentView(VideoView);

#endif

    }
    else
    {
        // Failed to load item

        setCurrentView(ErrorView);

        if (d->fileIndex != -1)
        {
            if (!d->osd->isPaused())
            {
                d->osd->pause(false);
            }
        }
    }

    preloadNextItem();
}

void SlideShowLoader::slotVideoFinished()
{
    if (d->fileIndex != -1)
    {
        d->osd->video(false);
        slotLoadNextItem();
    }
}

void SlideShowLoader::endOfSlide()
{
    setCurrentView(EndView);
    d->fileIndex = -1;
    d->osd->toolBar()->setEnabledPlay(false);
    d->osd->toolBar()->setEnabledNext(false);
    d->osd->toolBar()->setEnabledPrev(false);
    d->osd->toolBar()->setEnabledDelete(false);
}

void SlideShowLoader::preloadNextItem()
{
    int index = d->fileIndex + 1;
    int num   = d->settings->count();

    if (index >= num)
    {
        if (d->settings->loop)
        {
            index = 0;
        }
    }

    if (index < num)
    {
        QUrl nextItem = d->settings->fileList.value(index);

#ifdef HAVE_MEDIAPLAYER

        QMimeDatabase mimeDB;

        if (mimeDB.mimeTypeForFile(nextItem.toLocalFile())
                                   .name().startsWith(QLatin1String("video/")))
        {
            return;
        }

#endif

        d->imageView->setPreloadUrl(nextItem);
    }
}

void SlideShowLoader::wheelEvent(QWheelEvent* e)
{
    if (e->angleDelta().y() < 0)
    {
        d->osd->pause(true);
        slotLoadNextItem();
    }

    if (e->angleDelta().y() > 0)
    {
        if (d->fileIndex == -1)
        {
            // EndView => backward.

            d->fileIndex = d->settings->count();
        }

        d->osd->pause(true);
        slotLoadPrevItem();
    }
}

void SlideShowLoader::mousePressEvent(QMouseEvent* e)
{
    if      (e->button() == Qt::LeftButton)
    {
        if (d->fileIndex == -1)
        {
            // EndView => close Slideshow view.

            close();

            return;
        }

        d->osd->pause(true);
        slotLoadNextItem();
    }
    else if (e->button() == Qt::RightButton)
    {
        if (d->fileIndex == -1)
        {
            // EndView => backward.

            d->fileIndex = d->settings->count();
        }

        d->osd->pause(true);
        slotLoadPrevItem();
    }
}

void SlideShowLoader::keyPressEvent(QKeyEvent* e)
{
    if (!e)
    {
        return;
    }

    if (
        (e->key()       == Qt::Key_Y) &&
        (e->modifiers() == Qt::AltModifier)
       )
    {
        d->osd->setVisible(!d->osd->isVisible());

        return;
    }

#ifdef HAVE_MEDIAPLAYER

    if (
        (currentIndex() == VideoView) &&
        (e->modifiers() == Qt::ControlModifier)
       )
    {
        if      (e->key() == Qt::Key_Right)
        {
            d->osd->showVideoIndicator(true);
            d->videoView->forward();
            d->mouseMoveTimer->start();

            return;
        }
        else if (e->key() == Qt::Key_Left)
        {
            d->osd->showVideoIndicator(true);
            d->videoView->backward();
            d->mouseMoveTimer->start();

            return;
        }
    }

#endif

    d->osd->toolBar()->keyPressEvent(e);
}

bool SlideShowLoader::eventFilter(QObject* obj, QEvent* ev)
{
    if (ev->type() == QEvent::MouseMove)
    {
        setCursor(QCursor(Qt::ArrowCursor));

#ifdef HAVE_MEDIAPLAYER

        d->osd->showVideoIndicator(true);

#endif

        d->mouseMoveTimer->start();

        return false;
    }

    // pass the event on to the parent class

    return QWidget::eventFilter(obj, ev);
}

void SlideShowLoader::slotMouseMoveTimeOut()
{
    if (!d->osd->isUnderMouse())
    {
        setCursor(QCursor(Qt::BlankCursor));

#ifdef HAVE_MEDIAPLAYER

        d->osd->showVideoIndicator(false);

#endif

    }

}

void SlideShowLoader::inhibitScreenSaver()
{

#ifdef Q_OS_WIN

    SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS);

#elif defined HAVE_DBUS

    QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.ScreenSaver"),
                                                          QLatin1String("/ScreenSaver"),
                                                          QLatin1String("org.freedesktop.ScreenSaver"),
                                                          QLatin1String("Inhibit"));
    message << QLatin1String("digiKam");
    message << i18nc("Reason for inhibiting the screensaver activation, when the presentation mode is active",
                     "Giving a slideshow");

    QDBusReply<uint> reply = QDBusConnection::sessionBus().call(message);

    if (reply.isValid())
    {
        d->screenSaverCookie = reply.value();
    }

#elif defined Q_OS_MACOS

    QString reason       = i18nc("Reason for inhibiting the screensaver activation, when the presentation mode is active",
                                 "Giving a slideshow");
    CFStringRef cfReason = reason.toCFString();

    IOPMAssertionCreateWithName(
                                kIOPMAssertionTypePreventUserIdleDisplaySleep,
                                kIOPMAssertionLevelOn,
                                cfReason,
                                &d->powerAssertion
                               );

#endif

}

void SlideShowLoader::allowScreenSaver()
{

#ifdef Q_OS_WIN

    SetThreadExecutionState(ES_CONTINUOUS);

#elif defined HAVE_DBUS

    if (d->screenSaverCookie != -1)
    {
        QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.ScreenSaver"),
                                                              QLatin1String("/ScreenSaver"),
                                                              QLatin1String("org.freedesktop.ScreenSaver"),
                                                              QLatin1String("UnInhibit"));
        message << (uint)d->screenSaverCookie;
        QDBusConnection::sessionBus().send(message);
    }

#elif defined Q_OS_MACOS

    if (d->powerAssertion != 0)
    {
        IOPMAssertionRelease(d->powerAssertion);
        d->powerAssertion = 0;
    }

#endif

}

void SlideShowLoader::slotAssignRating(int rating)
{
    DItemInfo item;
    item.setRating(rating);
    d->settings->iface->setItemInfo(currentItem(), item.infoMap());

    dispatchCurrentInfoChange(currentItem());
}

void SlideShowLoader::slotAssignColorLabel(int color)
{
    DItemInfo item;
    item.setColorLabel(color);
    d->settings->iface->setItemInfo(currentItem(), item.infoMap());

    dispatchCurrentInfoChange(currentItem());
}

void SlideShowLoader::slotAssignPickLabel(int pick)
{
    DItemInfo item;
    item.setPickLabel(pick);
    d->settings->iface->setItemInfo(currentItem(), item.infoMap());

    dispatchCurrentInfoChange(currentItem());
}

void SlideShowLoader::slotToggleTag(int tag)
{
    DInfoInterface::DInfoMap info;
    info.insert(QLatin1String("tag"), tag);

    d->settings->iface->setItemInfo(currentItem(), info);

    dispatchCurrentInfoChange(currentItem());
}

void SlideShowLoader::slotHandleShortcut(const QString& shortcut, int val)
{
/*
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "SlideShowLoader::slotHandleShortcut";
*/
    if (d->shortcutPrefixes.contains(QLatin1String("rating")) &&
        shortcut.startsWith(d->shortcutPrefixes[QLatin1String("rating")]))
    {
        slotAssignRating(val);

        return;
    }

    if (d->shortcutPrefixes.contains(QLatin1String("colorlabel")) &&
        shortcut.startsWith(d->shortcutPrefixes[QLatin1String("colorlabel")]))
    {
        slotAssignColorLabel(val);

        return;
    }

    if (d->shortcutPrefixes.contains(QLatin1String("picklabel")) &&
        shortcut.startsWith(d->shortcutPrefixes[QLatin1String("picklabel")]))
    {
        slotAssignPickLabel(val);

        return;
    }

    if (d->shortcutPrefixes.contains(QLatin1String("tag")) &&
        shortcut.startsWith(d->shortcutPrefixes[QLatin1String("tag")]))
    {
        slotToggleTag(val);

        return;
    }

    qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Shortcut is not yet supported in SlideShowLoader::slotHandleShortcut():"
                                           << shortcut;
}

void SlideShowLoader::dispatchCurrentInfoChange(const QUrl& url)
{
    if (currentItem() == url)
    {
        d->osd->setCurrentUrl(currentItem());
    }
}

void SlideShowLoader::slotPause()
{

#ifdef HAVE_MEDIAPLAYER

    if (currentIndex() == VideoView)
    {
        d->videoView->pause(true);
    }
    else

#endif

    {
        d->osd->pause(true);
    }
}

void SlideShowLoader::slotPlay()
{
#ifdef HAVE_MEDIAPLAYER

    if (currentIndex() == VideoView)
    {
        d->videoView->pause(false);
    }
    else

#endif

    {
        d->osd->pause(false);
    }
}

void SlideShowLoader::slotScreenSelected(int screen)
{
    if (screen >= qApp->screens().count())
    {
        return;
    }

    QRect deskRect = qApp->screens().at(screen)->geometry();

    setWindowState(windowState() & ~Qt::WindowFullScreen);

    move(deskRect.x(), deskRect.y());
    resize(deskRect.width(), deskRect.height());

    setWindowState(windowState() | Qt::WindowFullScreen);

    // update OSD position

    if (d->fileIndex != -1)
    {
        qApp->processEvents();
        d->osd->setCurrentUrl(currentItem());
    }

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Slideshow: move to screen: " << screen
                                 << " :: " << deskRect;
}

} // namespace DigikamGenericSlideShowPlugin

#include "moc_slideshowloader.cpp"
