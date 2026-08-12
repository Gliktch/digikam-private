/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-14
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2007-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Parts of this code are based on
 * smoothslidesaver by Carsten Weinhold <carsten dot weinhold at gmx dot de>
 * and slideshowgl by Renchi Raju <renchi dot raju at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "presentationkb_p.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QImage>
#include <QPainter>
#include <QFont>
#include <QCursor>
#include <QPixmap>
#include <QApplication>
#include <QScreen>
#include <QWindow>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

namespace DigikamGenericPresentationPlugin
{

KBViewTrans::KBViewTrans(bool zoomIn, float relAspect)
{
    int i        = 0;

    // randomly select sizes of start and end viewport

    double s[2]  = { 0.0 };

    do
    {
        s[0]  = 0.3 * rnd() + 1.0;
        s[1]  = 0.3 * rnd() + 1.0;
    }
    while ((fabs(s[0] - s[1]) < 0.15) && (++i < 10));

    if (zoomIn ^ (s[0] > s[1]))
    {
        double tmp = s[0];
        s[0]       = s[1];
        s[1]       = tmp;
    }

    m_deltaScale = s[1] / s[0] - 1.0;
    m_baseScale  = s[0];

    // additional scale factors to ensure proper m_aspect of the displayed image

    double x[2]       = { 0.0 };
    double y[2]       = { 0.0 };
    double xMargin[2] = { 0.0 };
    double yMargin[2] = { 0.0 };
    double bestDist   = 0.0;
    double sx         = 0.0;
    double sy         = 0.0;

    if (relAspect > 1.0)
    {
        sx = 1.0;
        sy = relAspect;
    }
    else
    {
        sx = 1.0 / relAspect;
        sy = 1.0;
    }

    m_xScale   = sx;
    m_yScale   = sy;

    // calculate path

    xMargin[0] = (s[0] * sx - 1.0) / 2.0;
    yMargin[0] = (s[0] * sy - 1.0) / 2.0;
    xMargin[1] = (s[1] * sx - 1.0) / 2.0;
    yMargin[1] = (s[1] * sy - 1.0) / 2.0;

    i        = 0;
    bestDist = 0.0;

    do
    {
        double sign = rndSign();
        x[0]        = xMargin[0] * (0.2 * rnd() + 0.8) *  sign;
        y[0]        = yMargin[0] * (0.2 * rnd() + 0.8) * -sign;
        x[1]        = xMargin[1] * (0.2 * rnd() + 0.8) * -sign;
        y[1]        = yMargin[1] * (0.2 * rnd() + 0.8) *  sign;

        if (fabs(x[1] - x[0]) + fabs(y[1] - y[0]) > bestDist)
        {
            m_baseX  = x[0];
            m_baseY  = y[0];
            m_deltaX = x[1] - x[0];
            m_deltaY = y[1] - y[0];
            bestDist = fabs(m_deltaX) + fabs(m_deltaY);
        }

    }
    while ((bestDist < 0.3) && (++i < 10));
}

float KBViewTrans::transX(float pos) const
{
    return (m_baseX + m_deltaX * pos);
}

float KBViewTrans::transY(float pos) const
{
    return (m_baseY + m_deltaY * pos);
}

float KBViewTrans::scale (float pos) const
{
    return (m_baseScale * (1.0 + m_deltaScale * pos));
}

float KBViewTrans::xScaleCorrect() const
{
    return m_xScale;
}

float KBViewTrans::yScaleCorrect() const
{
    return m_yScale;
}

double KBViewTrans::rnd() const
{
    return QRandomGenerator::global()->generateDouble();
}

double KBViewTrans::rndSign() const
{
    return ((QRandomGenerator::global()->bounded(2U) == 0) ? 1.0 : -1.0);
}

// -------------------------------------------------------------------------

KBImage::KBImage(KBViewTrans* const viewTrans, float aspect)
    : m_viewTrans(viewTrans),
      m_aspect   (aspect)
{
    m_paint = (m_viewTrans) ? true : false;
}

KBImage::~KBImage()
{
    if (m_texture)
    {
        m_texture->destroy();
    }

    delete m_viewTrans;
    delete m_texture;
}

// -------------------------------------------------------------------------

PresentationKB::PresentationKB(PresentationContainer* const sharedData)
    : QOpenGLWidget(),
      d            (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setContextMenuPolicy(Qt::PreventContextMenu);

#ifdef Q_OS_WIN

    setWindowFlags(Qt::Popup               |
                   Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint);

#else

    setWindowState(windowState() | Qt::WindowFullScreen);

#endif

    QScreen* screen = qApp->primaryScreen();

    if (QWidget* const widget = qApp->activeWindow())
    {
        if (QWindow* const window = widget->windowHandle())
        {
            screen = window->screen();
        }
    }

    QRect deskRect = screen->geometry();
    d->deskX       = deskRect.x();
    d->deskY       = deskRect.y();
    d->deskWidth   = deskRect.width();
    d->deskHeight  = deskRect.height();

    move(d->deskX, d->deskY);
    resize(d->deskWidth, d->deskHeight);

    d->sharedData          = sharedData;
    d->sharedData->display = this;

    readSettings();

    unsigned frameRate;

    if (d->forceFrameRate == 0)
    {
        frameRate = qRound(screen->refreshRate() * 2);
    }
    else
    {
        frameRate = d->forceFrameRate;
    }

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Frame Rate : " << frameRate;

    d->image[0]        = new KBImage(nullptr);
    d->image[1]        = new KBImage(nullptr);
    d->step            = 1.0 / ((float)(d->delay * frameRate));
    d->enableSameSpeed = d->sharedData->kbEnableSameSpeed;
    d->imageLoadThread = new KBImageLoader(d->sharedData, width(), height());
    d->timer           = new QTimer(this);

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(moveSlot()));

    // -- playback widget -------------------------------

#ifdef HAVE_MEDIAPLAYER

    d->playbackWidget = new PresentationAudioWidget(this, d->sharedData->soundtrackUrls, d->sharedData);
    d->playbackWidget->hide();
    d->playbackWidget->move(0, 0);

#endif

    // -- hide cursor when not moved --------------------

    d->mouseMoveTimer = new QTimer(this);
    d->mouseMoveTimer->setSingleShot(true);

    connect(d->mouseMoveTimer, SIGNAL(timeout()),
            this, SLOT(slotMouseMoveTimeOut()));

    setMouseTracking(true);
    slotMouseMoveTimeOut();

    // -- load image and let's start

    d->imageLoadThread->start();
    d->timer->start(1000 / frameRate);

#ifdef HAVE_MEDIAPLAYER

    if (d->sharedData->soundtrackPlay)
    {
        d->playbackWidget->slotPlay();
    }

#endif

}

PresentationKB::~PresentationKB()
{

#ifdef HAVE_MEDIAPLAYER

    d->playbackWidget->slotStop();

#endif

    d->timer->stop();
    d->mouseMoveTimer->stop();

    delete d->effect;
    delete d->image[0];
    delete d->image[1];

    if (d->endTexture)
    {
        d->endTexture->destroy();
    }

    delete d->endTexture;

    d->imageLoadThread->quit();
    bool terminated = d->imageLoadThread->wait(10000);

    if (!terminated)
    {
        d->imageLoadThread->terminate();
        d->imageLoadThread->wait(3000);
    }

    delete d->imageLoadThread;
    delete d;
}

float PresentationKB::aspect() const
{
    return ((float)width() / (float)height());
}

void PresentationKB::setNewKBEffect()
{
    KBEffect::Type type;
    bool needFadeIn = ((d->effect == nullptr) || (d->effect->type() == KBEffect::Fade));

    // we currently only have two effects

    if      (d->disableFadeInOut)
    {
        type = KBEffect::Blend;
    }
    else if (d->disableCrossFade)
    {
        type = KBEffect::Fade;
    }
    else
    {
        type = KBEffect::chooseKBEffect((d->effect) ? d->effect->type() : KBEffect::Fade);
    }

    delete d->effect;

    switch (type)
    {
        case KBEffect::Fade:
        {
            d->effect = new FadeKBEffect(this, needFadeIn);
            break;
        }

        case KBEffect::Blend:
        {
            d->effect = new BlendKBEffect(this, needFadeIn);
            break;
        }

        default:
        {
            qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Unknown transition effect, falling back to crossfade";
            d->effect = new BlendKBEffect(this, needFadeIn);
            break;
        }
    }
}

void PresentationKB::moveSlot()
{
    if (d->initialized)
    {
        if (d->effect->done())
        {
            setNewKBEffect();
            d->imageLoadThread->requestNewImage();
            d->endOfShow = !d->haveImages;
        }

        if (d->enableSameSpeed)
        {
            d->effect->advanceTime(d->stepSameSpeed);
        }
        else
        {
            d->effect->advanceTime(d->step);
        }
    }

    update();
}

bool PresentationKB::setupNewImage(int idx)
{
    Q_ASSERT(idx >= 0 && idx < 2);

    if (!d->haveImages)
    {
        return false;
    }

    bool ok   = false;
    d->zoomIn = !d->zoomIn;

    if (d->imageLoadThread->grabImage())
    {
        delete d->image[idx];

        // we have the image lock and there is an image

        float imageAspect            = d->imageLoadThread->imageAspect();
        KBViewTrans* const viewTrans = new KBViewTrans(d->zoomIn, aspect() / imageAspect);
        d->image[idx]                = new KBImage(viewTrans, imageAspect);

        applyTexture(d->image[idx], d->imageLoadThread->image());
        ok                           = true;

    }
    else
    {
        d->haveImages = false;
    }

    // don't forget to release the lock on the copy of the image
    // owned by the image loader thread

    d->imageLoadThread->ungrabImage();

    return ok;
}

void PresentationKB::startSlideShowOnce()
{
    // when the image loader thread is ready, it will already have loaded
    // the first image

    if ((d->initialized == false) && d->imageLoadThread->ready())
    {
        setupNewImage(0);                      // setup the first image and
        d->imageLoadThread->requestNewImage(); // load the next one in background
        setNewKBEffect();                      // set the initial effect
        if (d->enableSameSpeed)
        {
            d->stepSameSpeed = d->step / d->imageLoadThread->imageAspect();
        }

        d->initialized = true;
    }
}

void PresentationKB::swapImages()
{
    KBImage* const tmp = d->image[0];
    d->image[0]        = d->image[1];
    d->image[1]        = tmp;
}

void PresentationKB::initializeGL()
{
    // Enable Texture Mapping

    glEnable(GL_TEXTURE_2D);

    // Clear The Background Color

    glClearColor(0.0, 0.0, 0.0, 1.0f);

    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);

    // Turn Blending On

    glEnable(GL_BLEND);

    // Blending Function For Translucency Based On Source Alpha Value

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable perspective vision

    glClearDepth(1.0f);
}

void PresentationKB::paintGL()
{
    startSlideShowOnce();

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // only clear the color buffer, if none of the active images is fully opaque

    if (!((d->image[0]->m_paint && (d->image[0]->m_opacity == 1.0)) ||
        (d->image[1]->m_paint && (d->image[1]->m_opacity == 1.0))))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (d->endOfShow)
    {
        endOfShow();
        d->timer->stop();
    }
    else
    {
        if (d->image[1]->m_paint)
        {
            paintTexture(d->image[1]);
        }

        if (d->image[0]->m_paint)
        {
            paintTexture(d->image[0]);
        }
    }

    glFlush();
}

void PresentationKB::resizeGL(int w, int h)
{
    glViewport(0, 0, (GLint)w, (GLint)h);
}

void PresentationKB::applyTexture(KBImage* const img, const QImage &texture)
{
    /* create the texture */

    img->m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 9, 0))

    img->m_texture->setData(texture.flipped());

#else

    img->m_texture->setData(texture.mirrored());

#endif

    img->m_texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    img->m_texture->setMagnificationFilter(QOpenGLTexture::Linear);
    img->m_texture->bind();
}

void PresentationKB::paintTexture(KBImage* const img)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float sx = img->m_viewTrans->xScaleCorrect();
    float sy = img->m_viewTrans->yScaleCorrect();

    glTranslatef(img->m_viewTrans->transX(img->m_pos) * 2.0, img->m_viewTrans->transY(img->m_pos) * 2.0, 0.0);
    glScalef(img->m_viewTrans->scale(img->m_pos), img->m_viewTrans->scale(img->m_pos), 0.0);

    img->m_texture->bind();

    glBegin(GL_QUADS);
    {
        glColor4f(1.0, 1.0, 1.0, img->m_opacity);
        glTexCoord2f(0, 0);
        glVertex3f(-sx, -sy, 0);

        glTexCoord2f(1, 0);
        glVertex3f(sx, -sy, 0);

        glTexCoord2f(1, 1);
        glVertex3f(sx, sy, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-sx, sy, 0);
    }
    glEnd();
}

void PresentationKB::readSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("Presentation Settings"));

    d->delay                = group.readEntry("Delay", 8000) / 1000;
    d->disableFadeInOut     = group.readEntry("KB Disable FadeInOut", false);
    d->disableCrossFade     = group.readEntry("KB Disable Crossfade", false);
    d->forceFrameRate       = group.readEntry("KB Force Framerate", 0);

    if (d->delay < 5)
    {
        d->delay = 5;
    }

    if (d->forceFrameRate > 120)
    {
        d->forceFrameRate = 120;
    }
}

void PresentationKB::endOfShow()
{
    QPixmap pix(512, 512);
    pix.fill(Qt::black);

    QFont fn(font());
    fn.setPointSize(fn.pointSize() + 10);
    fn.setBold(true);

    QPainter p(&pix);
    p.setPen(Qt::white);
    p.setFont(fn);
    p.drawText(20, 50,  i18n("SlideShow Completed"));
    p.drawText(20, 100, i18n("Click to Exit..."));
    p.end();

    /* create the texture */

    d->endTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 9, 0))

    d->endTexture->setData(pix.toImage().flipped());

#else

    d->endTexture->setData(pix.toImage().mirrored());

#endif

    d->endTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    d->endTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    d->endTexture->bind();

    /* paint the texture */

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glTexCoord2f(0, 0);
        glVertex3f(-1.0, -1.0, 0);

        glTexCoord2f(1, 0);
        glVertex3f(1.0, -1.0, 0);

        glTexCoord2f(1, 1);
        glVertex3f(1.0, 1.0, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-1.0, 1.0, 0);
    }
    glEnd();

    d->showingEnd = true;
}

QStringList PresentationKB::effectNames()
{
    QStringList effects;
    effects.append(QLatin1String("Ken Burns"));

    return effects;
}

QMap<QString, QString> PresentationKB::effectNamesI18N()
{
    QMap<QString, QString> effects;
    effects[QLatin1String("Ken Burns")] = i18n("Ken Burns");

    return effects;
}

void PresentationKB::keyPressEvent(QKeyEvent* event)
{
    if (!event)
    {
        return;
    }

#ifdef HAVE_MEDIAPLAYER

    d->playbackWidget->keyPressEvent(event);

#endif

    if (event->key() == Qt::Key_Escape)
    {
        close();
    }
}

void PresentationKB::mousePressEvent(QMouseEvent* e)
{
    if (!e)
    {
        return;
    }

    if (d->endOfShow && d->showingEnd)
    {
        slotClose();
    }
}

void PresentationKB::mouseMoveEvent(QMouseEvent* e)
{
    setCursor(QCursor(Qt::ArrowCursor));
    d->mouseMoveTimer->start(1000);

#ifdef HAVE_MEDIAPLAYER

    if (!d->playbackWidget->canHide())
    {
        return;
    }

    QPoint pos(e->pos());

    if (
        (pos.y() > 20)                     &&
        (pos.y() < (d->deskHeight - 20 - 1))
       )
    {
        if (d->playbackWidget->isHidden())
        {
            return;
        }
        else
        {
            d->playbackWidget->hide();
            setFocus();
        }

        return;
    }

    d->playbackWidget->show();

#else

    Q_UNUSED(e);

#endif
}

void PresentationKB::slotMouseMoveTimeOut()
{
    QPoint pos(QCursor::pos());

    if (
        (pos.y() < 20)                    ||
        (pos.y() > (d->deskHeight - 20 - 1)
       )

#ifdef HAVE_MEDIAPLAYER

        || d->playbackWidget->underMouse()

#endif

       )
    {
        return;
    }

    setCursor(QCursor(Qt::BlankCursor));
}

bool PresentationKB::checkOpenGL() const
{
    // No OpenGL context is found. Are the drivers ok?

    if (!isValid())
    {
        return false;
    }

    // GL_EXT_texture3D is not supported

    QString s = QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));

    if (!s.contains(QString::fromLatin1("GL_EXT_texture3D"), Qt::CaseInsensitive))
    {
        return false;
    }

    // Everything is ok!

    return true;
}

void PresentationKB::slotClose()
{
    close();
}

} // namespace DigikamGenericPresentationPlugin

#include "moc_presentationkb.cpp"
