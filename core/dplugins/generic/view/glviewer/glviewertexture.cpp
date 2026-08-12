/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-11
 * Description : a tool to show image using an OpenGL interface.
 *
 * SPDX-FileCopyrightText: 2007-2008 by Markus Leuthold <kusi at forum dot titlis dot org>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "glviewertexture.h"

// Qt includes

#include <QUrl>
#include <QScopedPointer>

// Local includes

#include "dimg.h"
#include "iccmanager.h"
#include "iccsettings.h"
#include "iccsettingscontainer.h"
#include "previewloadthread.h"
#include "dmetadata.h"
#include "digikam_debug.h"
#include "glviewertimer.h"

namespace DigikamGenericGLViewerPlugin
{

class Q_DECL_HIDDEN GLViewerTexture::Private
{
public:

    Private()
    {
        rotate_list[0] = DMetadata::ORIENTATION_ROT_90;
        rotate_list[1] = DMetadata::ORIENTATION_ROT_180;
        rotate_list[2] = DMetadata::ORIENTATION_ROT_270;
        rotate_list[3] = DMetadata::ORIENTATION_ROT_180;
    }

public:

    float                       rdx                 = 0.0F;
    float                       rdy                 = 0.0F;
    float                       z                   = 0.0F;
    float                       ux                  = 0.0F;
    float                       uy                  = 0.0F;
    float                       rtx                 = 0.0F;
    float                       rty                 = 0.0F;
    float                       vtop                = 0.0F;
    float                       vbottom             = 0.0F;
    float                       vleft               = 0.0F;
    float                       vright              = 0.0F;
    int                         display_x           = 0;
    int                         display_y           = 0;
    QString                     filename;
    QImage                      qimage;
    QImage                      fimage;
    DMetadata::ImageOrientation rotate_list[4];
    int                         rotate_idx          = 0;
    IccProfile                  iccProfile;
    DInfoInterface*             iface               = nullptr;
    QWidget*                    displayWidget       = nullptr;

private:

    /// No copy constructor
    Private(const Private&);
};

GLViewerTexture::GLViewerTexture(DInfoInterface* const iface, QWidget* const display)
    : QOpenGLTexture(QOpenGLTexture::TargetRectangle),
      d             (new Private)
{
    d->iface                      = iface;
    d->displayWidget              = display;
    ICCSettingsContainer settings = IccSettings::instance()->settings();

    if (settings.enableCM && settings.useManagedPreviews)
    {
        d->iccProfile = IccProfile(IccManager::displayProfile(display));
    }

    reset();
}

GLViewerTexture::~GLViewerTexture()
{
    destroy();

    delete d;
}

/*!
    \fn GLViewerTexture::load(QString fn, QSize size)
    @brief load file from disc and save it in texture
    \param fn filename to load
    \param size the size of image which is downloaded to texture mem
    if "size" is set to image size, scaling is only performed by the GPU but not
    by the CPU, however the AGP usage to texture memory is increased (20MB for a 5mp image)
 */
bool GLViewerTexture::load(const QString& fn, const QSize& size)
{
    d->filename   = fn;
    d->qimage     = PreviewLoadThread::loadFastSynchronously(d->filename,
                                                             qMax(size.width()  * 1.2,
                                                                  size.height() * 1.2),
                                                             d->iccProfile).copyQImage();

    if (d->qimage.isNull())
    {
        return false;
    }

    loadInternal();
    reset();

    d->rotate_idx = 0;

    return true;
}

/*!
    \fn GLViewerTexture::load(QImage im, QSize size)
    @brief copy file from QImage to texture
    \param im Qimage to be copied from
 */
bool GLViewerTexture::load(const QImage& im)
{
    d->qimage = im;

    loadInternal();
    reset();

    d->rotate_idx   = 0;

    return true;
}

/*!
    @brief load full size image from disc and save it in texture
 */
bool GLViewerTexture::loadFullSize()
{
    if (!d->fimage.isNull())
    {
        return false;
    }

    d->fimage     = PreviewLoadThread::loadHighQualitySynchronously(d->filename,
                                                                    PreviewSettings::RawPreviewAutomatic,
                                                                    d->iccProfile).copyQImage();

    if (d->fimage.isNull())
    {
        return false;
    }

    loadInternal();
    reset();

    d->rotate_idx = 0;

    return true;
}

/*!
    \fn GLViewerTexture::load()
    internal load function
    rt[xy] <= 1
 */
bool GLViewerTexture::loadInternal()
{
    destroy();
    create();

    QImage texImg = d->fimage.isNull() ? d->qimage
                                       : d->fimage;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 9, 0))

    setData(texImg.flipped(Qt::Vertical),
            QOpenGLTexture::DontGenerateMipMaps);

#else

    setData(texImg.mirrored(),
            QOpenGLTexture::DontGenerateMipMaps);

#endif

    setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    setMagnificationFilter(QOpenGLTexture::Linear);

    int w = width();
    int h = height();

    if (h < w)
    {
        d->rtx = 1;
        d->rty = float(h) / float(w);
    }
    else
    {
        d->rtx = float(w) / float(h);
        d->rty = 1;
    }

    return true;
}

/*!
    \fn GLViewerTexture::zoom(float delta, QPoint mousepos)
    @brief calculate new tex coords on zooming
    \param delta the delta between previous zoom and current zoom
    \param mousepos mouse position returned by QT
    \TODO rename mousepos to something more generic
*/
void GLViewerTexture::zoom(float delta, const QPoint& mousepos)
// u: start in texture, u = [0..1], u = 0 is begin, u = 1 is end of texture
// z = [0..1], z = 1 -> no zoom
// l: length of tex in glFrustum coordinate system
// rt: ratio of tex, rt <= 1, see loadInternal() for definition
// rd: ratio of display, rd >= 1
// m: mouse pos normalized, cd = [0..rd]
// c: mouse pos normalized to zoom * l, c = [0..1]
{
    d->z    *= delta;
    delta    =  d->z * (1.0 / delta - 1.0); //convert to real delta = z_old - z_new

    float mx = mousepos.x() / (float)d->display_x * d->rdx;
    float cx = (mx-d->rdx / 2.0 + d->rtx / 2.0) / d->rtx;
    float vx = d->ux + cx * d->z;
    d->ux    = d->ux + (vx - d->ux) * delta / d->z;

    float my = mousepos.y() / (float)d->display_y * d->rdy;
    float cy = (my-d->rdy / 2.0 + d->rty / 2.0) / d->rty;
    cy       = 1 - cy;
    float vy = d->uy + cy * d->z;
    d->uy    = d->uy + (vy - d->uy) * delta / d->z;

    calcVertex();
}

/*!
    \fn GLViewerTexture::calcVertex()
    Calculate vertices according internal state variables
    z, ux, uy are calculated in GLViewerTexture::zoom()
 */
void GLViewerTexture::calcVertex()
// rt: ratio of tex, rt <= 1, see loadInternal() for definition
// u: start in texture, u = [0..1], u=0 is begin, u=1 is end of texture
// l: length of tex in glFrustum coordinate system
// halftexel: the color of a texel is determined by a corner of the texel and not its center point
//            this seems to introduce a visible jump on changing the tex-size.
//
// the glFrustum coord-sys is visible in [-rdx..rdx] ([-1..1] for square screen) for z = 1 (no zoom)
// the tex coord-sys goes from [-rtx..rtx] ([-1..1] for square texture)
{
    // x part
    float lx          = 2 * d->rtx / d->z;   //length of tex
    float tsx         = lx / (float)width(); //texelsize in glFrustum coordinates
    float halftexel_x = tsx / 2.0;
    float wx          = lx * (1 - d->ux - d->z);
    d->vleft          = -d->rtx - d->ux * lx - halftexel_x; //left
    d->vright         = d->rtx + wx - halftexel_x;          //right

    // y part
    float ly          = 2 * d->rty / d->z;
    float tsy         = ly / (float)height(); //texelsize in glFrustum coordinates
    float halftexel_y = tsy / 2.0;
    float wy          = ly * (1 - d->uy - d->z);
    d->vbottom        = -d->rty - d->uy * ly + halftexel_y; //bottom
    d->vtop           = d->rty + wy + halftexel_y;          //top
}

/*!
    \fn GLViewerTexture::vertex_bottom() const
 */
GLfloat GLViewerTexture::vertex_bottom() const
{
    return (GLfloat) d->vbottom;
}

/*!
    \fn GLViewerTexture::vertex_top() const
 */
GLfloat GLViewerTexture::vertex_top() const
{
    return (GLfloat) d->vtop;
}

/*!
    \fn GLViewerTexture::vertex_left() const
 */
GLfloat GLViewerTexture::vertex_left() const
{
    return (GLfloat) d->vleft;
}

/*!
    \fn GLViewerTexture::vertex_right() const
 */
GLfloat GLViewerTexture::vertex_right() const
{
    return (GLfloat) d->vright;
}

/*!
    \fn GLViewerTexture::setViewport(int w, int h)
    \param w width of window
    \param h height of window
    Set widget's viewport. Ensures that rdx & rdy are always > 1
 */
void GLViewerTexture::setViewport(int w, int h)
{
    if (h > w)
    {
        d->rdx = 1.0;
        d->rdy = h / float(w);
    }
    else
    {
        d->rdx = w / float(h);
        d->rdy = 1.0;
    }

    d->display_x = w;
    d->display_y = h;
}

/*!
    \fn GLViewerTexture::move(QPoint diff)
    new tex coordinates have to be calculated if the view is panned
 */
void GLViewerTexture::move(const QPoint& diff)
{
    d->ux = d->ux - diff.x() / float(d->display_x) * d->z * d->rdx / d->rtx;
    d->uy = d->uy + diff.y() / float(d->display_y) * d->z * d->rdy / d->rty;
    calcVertex();
}

/*!
    \fn GLViewerTexture::reset()
 */
void GLViewerTexture::reset(bool resetFullImage)
{
    d->ux           = 0;
    d->uy           = 0;
    d->z            = 1.0;
    float zoomdelta = 0;

    if ((d->rtx < d->rty) && (d->rdx < d->rdy) && ((d->rtx / d->rty) < (d->rdx / d->rdy)))
    {
        zoomdelta = d->z - d->rdx / d->rdy;
    }

    if ((d->rtx < d->rty) && ((d->rtx / d->rty) > (d->rdx / d->rdy)))
    {
        zoomdelta = d->z - d->rtx;
    }

    if ((d->rtx >= d->rty) && (d->rdy < d->rdx) && ((d->rty / d->rtx) < (d->rdy / d->rdx)))
    {
        zoomdelta = d->z - d->rdy / d->rdx;
    }

    if ((d->rtx >= d->rty) && ((d->rty / d->rtx) > (d->rdy / d->rdx)))
    {
        zoomdelta = d->z - d->rty;
    }

    QPoint p  = QPoint(d->display_x / 2, d->display_y / 2);
    zoom(1.0 - zoomdelta, p);

    if (resetFullImage)
    {
        d->fimage = QImage();
    }

    calcVertex();
}

/*!
    \fn GLViewerTexture::setNewSize(QSize size)
    \param size desired texture size. QSize(0,0) will take the full image
    \return true if size has changed, false otherwise
    set new texture size in order to reduce AGP bandwidth
 */
bool GLViewerTexture::setNewSize(QSize size)
{
    if (d->qimage.isNull())
    {
        return false;
    }

    // don't allow larger textures than the original image. the image will be upsampled by
    // OpenGL if necessary and not by QImage::scale

    QImage texImg = d->fimage.isNull() ? d->qimage
                                       : d->fimage;
    size          = size.boundedTo(texImg.size());

    if (width() == size.width())
    {
        return false;
    }

    int w = size.width();
    int h = size.height();

    destroy();
    create();

    if (w == 0)

#if (QT_VERSION >= QT_VERSION_CHECK(6, 9, 0))

    {
        setData(texImg.flipped(Qt::Vertical),
                QOpenGLTexture::DontGenerateMipMaps);
    }
    else
    {
        setData(texImg.scaled(w, h, Qt::KeepAspectRatio,
                              Qt::SmoothTransformation).flipped(Qt::Vertical),
                              QOpenGLTexture::DontGenerateMipMaps);
    }

#else

    {
        setData(texImg.mirrored(),
                QOpenGLTexture::DontGenerateMipMaps);
    }
    else
    {
        setData(texImg.scaled(w, h, Qt::KeepAspectRatio,
                              Qt::SmoothTransformation).mirrored(),
                              QOpenGLTexture::DontGenerateMipMaps);
    }

#endif

    setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    setMagnificationFilter(QOpenGLTexture::Linear);

    // recalculate half-texel offset

    calcVertex();

    return true;
}

/*!
    \fn GLViewerTexture::rotate()
    @brief smart image rotation
    since the two most frequent usecases are a CW or CCW rotation of 90,
    perform these rotation with one (+90) or two (-90) calls of rotation()
 */
void GLViewerTexture::rotate()
{
    QScopedPointer<DMetadata> meta(new DMetadata);

    if (!d->fimage.isNull())
    {
        meta->rotateExifQImage(d->fimage, (DMetadata::ImageOrientation)d->rotate_list[d->rotate_idx % 4]);
    }

    meta->rotateExifQImage(d->qimage, (DMetadata::ImageOrientation)d->rotate_list[d->rotate_idx % 4]);

    loadInternal();

    // save new rotation in host application

    DInfoInterface::DInfoMap info;
    DItemInfo item(info);
    item.setOrientation(d->rotate_list[d->rotate_idx % 4]);
    d->iface->setItemInfo(QUrl::fromLocalFile(d->filename), info);

    reset();
    d->rotate_idx++;
}

/*!
    \fn GLViewerTexture::setToOriginalSize()
    zoom image such that each pixel of the screen corresponds to a pixel in the jpg
    remember that OpenGL is not a pixel exact specification, and the image will still be filtered by OpenGL
 */
void GLViewerTexture::zoomToOriginal()
{
    QSize imgSize = d->fimage.isNull() ? d->qimage.size() : d->fimage.size();
    float zoomfactorToOriginal;
    reset();

    if (float(imgSize.width()) / float(imgSize.height()) > float(d->display_x) / float(d->display_y))
    {
        // Image touches right and left edge of window

        zoomfactorToOriginal = float(d->display_x) / imgSize.width();
    }
    else
    {
        // Image touches upper and lower edge of window

        zoomfactorToOriginal = float(d->display_y) / imgSize.height();
    }

    zoomfactorToOriginal *= d->displayWidget->devicePixelRatio();

    zoom(zoomfactorToOriginal, QPoint(d->display_x / 2, d->display_y / 2));
}

} // namespace DigikamGenericGLViewerPlugin
