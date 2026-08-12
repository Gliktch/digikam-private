/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-18
 * Description : slideshow image widget
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "slideimage.h"

// Qt includes

#include <QApplication>
#include <QPainter>
#include <QScreen>
#include <QWindow>

// Local includes

#include "digikam_debug.h"
#include "previewloadthread.h"

using namespace Digikam;

namespace DigikamGenericSlideShowPlugin
{

class Q_DECL_HIDDEN SlideImage::Private
{

public:

    Private() = default;

public:

    SlideShowSettings*  settings                = nullptr;

    QPixmap             pixmap;

    QUrl                currentImage;

    DImg                preview;
    PreviewLoadThread*  previewThread           = nullptr;
    PreviewLoadThread*  previewPreloadThread    = nullptr;
};

SlideImage::SlideImage(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setWindowFlags(Qt::FramelessWindowHint);
    setMouseTracking(true);

    d->previewThread        = new PreviewLoadThread();
    d->previewPreloadThread = new PreviewLoadThread();

    connect(d->previewThread, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
            this, SLOT(slotGotImagePreview(LoadingDescription,DImg)));
}

SlideImage::~SlideImage()
{
    d->previewPreloadThread->stopAllTasks();
    d->previewThread->stopAllTasks();

    d->previewPreloadThread->wait();
    d->previewThread->wait();

    delete d->previewPreloadThread;
    delete d->previewThread;
    delete d;
}

void SlideImage::setSlideShowSettings(SlideShowSettings* const settings)
{
    d->settings = settings;
}

void SlideImage::setLoadUrl(const QUrl& url)
{
    d->currentImage = url;

    // calculate preview size which is used for fast previews

    QScreen* screen = qApp->primaryScreen();

    if (QWidget* const widget = nativeParentWidget())
    {
        if (QWindow* const window = widget->windowHandle())
        {
            screen = window->screen();
        }
    }

    QSize desktopSize = screen->geometry().size();
    int deskSize      = qMax(640, qMax(desktopSize.height(), desktopSize.width()));
    d->previewThread->load(url.toLocalFile(), d->settings->previewSettings, deskSize);
}

void SlideImage::setPreloadUrl(const QUrl& url)
{
    // calculate preview size which is used for fast previews

    QScreen* screen = qApp->primaryScreen();

    if (QWidget* const widget = nativeParentWidget())
    {
        if (QWindow* const window = widget->windowHandle())
        {
            screen = window->screen();
        }
    }

    QSize desktopSize = screen->geometry().size();
    int deskSize      = qMax(640, qMax(desktopSize.height(), desktopSize.width()));
    d->previewPreloadThread->load(url.toLocalFile(), d->settings->previewSettings, deskSize);
}

void SlideImage::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, width(), height(), d->pixmap,
                 0, 0, d->pixmap.width(), d->pixmap.height());
    p.end();
}

void SlideImage::slotGotImagePreview(const LoadingDescription& desc, const DImg& preview)
{
    if ((desc.filePath != d->currentImage.toLocalFile()) || desc.isThumbnail())
    {
        return;
    }

    d->preview.reset();

    if (!DImg::isAnimatedImage(desc.filePath))      // Special case for animated images as GIF or NMG
    {
        d->preview = preview;
    }

    if (!d->preview.isNull())
    {
        updatePixmap();
        update();

        Q_EMIT signalImageLoaded(true);

        return;
    }

    Q_EMIT signalImageLoaded(false);
}

void SlideImage::updatePixmap()
{
    /**
     * For high resolution ("retina") displays, Mac OS X / Qt
     * report only half of the physical resolution in terms of
     * pixels, i.e. every logical pixels corresponds to 2x2
     * physical pixels. However, UI elements and fonts are
     * nevertheless rendered at full resolution, and pixmaps
     * as well, provided their resolution is high enough (that
     * is, higher than the reported, logical resolution).
     *
     * To work around this, we render the photos not a logical
     * resolution, but with the photo's full resolution, but
     * at the screen's aspect ratio. When we later draw this
     * high resolution bitmap, it is up to Qt to scale the
     * photo to the true physical resolution.  The ratio
     * computed below is the ratio between the photo and
     * screen resolutions, or equivalently the factor by which
     * we need to increase the pixel size of the rendered
     * pixmap.
     */

    double dpr     = devicePixelRatio();

    QSize fullSize = QSizeF(dpr * width(), dpr * height()).toSize();
    d->pixmap      = QPixmap(fullSize);
    d->pixmap.fill(d->settings->bgColor);

    QPainter p(&(d->pixmap));

    QPixmap pix(d->preview.smoothScale(d->pixmap.width(),
                                       d->pixmap.height(),
                                       Qt::KeepAspectRatio).convertToPixmap());

    p.drawPixmap((d->pixmap.width()  - pix.width())  / 2,
                 (d->pixmap.height() - pix.height()) / 2, pix,
                 0, 0, pix.width(), pix.height());
}

} // namespace DigikamGenericSlideShowPlugin

#include "moc_slideimage.cpp"
