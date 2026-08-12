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

#pragma once

// C++ includes

#ifndef Q_CC_MSVC
#   include <cstdlib>
#else
#   include <winsock2.h>
#endif

// Qt includes

#include <QList>
#include <QKeyEvent>
#include <QMap>
#include <QMouseEvent>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QOpenGLWidget>
#include <QOpenGLTexture>
#include <QRandomGenerator>

namespace DigikamGenericPresentationPlugin
{

class PresentationContainer;

class KBViewTrans
{

public:

    KBViewTrans()  = default;
    KBViewTrans(bool zoomIn, float relAspect);
    ~KBViewTrans() = default;

    float transX(float pos) const;
    float transY(float pos) const;
    float scale(float pos)  const;
    float xScaleCorrect()   const;
    float yScaleCorrect()   const;

private:

    double rnd()            const;
    double rndSign()        const;

private:

    /// delta and scale values (begin to end) and the needed offsets

    double m_deltaX     = 0.0;
    double m_deltaY     = 0.0;
    double m_deltaScale = 0.0;
    double m_baseScale  = 0.0;
    double m_baseX      = 0.0;
    double m_baseY      = 0.0;
    float  m_xScale     = 0.0F;
    float  m_yScale     = 0.0F;
};

// -------------------------------------------------------------------------

class KBImage
{

public:

    explicit KBImage(KBViewTrans* const viewTrans, float aspect = 1.0);
    ~KBImage();

public:

    KBViewTrans*    m_viewTrans = nullptr;
    float           m_aspect    = 0.0F;
    float           m_pos       = 0.0F;
    float           m_opacity   = 0.0F;
    bool            m_paint     = false;
    QOpenGLTexture* m_texture   = nullptr;

private:

    /// @note disabled
    KBImage(const KBImage&)            = delete;
    KBImage& operator=(const KBImage&) = delete;
};

// -------------------------------------------------------------------------

class PresentationKB : public QOpenGLWidget
{
    Q_OBJECT

public:

    explicit PresentationKB(PresentationContainer* const sharedData);

    ~PresentationKB() override;

    static QStringList            effectNames();
    static QMap<QString, QString> effectNamesI18N();

    bool checkOpenGL()                      const;

private:

    float    aspect()                       const;
    bool     setupNewImage(int imageIndex);
    void     startSlideShowOnce();
    void     swapImages();
    void     setNewKBEffect();
    void     endOfShow();

    void     applyTexture(KBImage* const img, const QImage& image);
    void     paintTexture(KBImage* const img);
    unsigned suggestFrameRate(unsigned forceRate);

    void     readSettings();

protected:

    void initializeGL()                override;
    void paintGL()                     override;
    void resizeGL(int w, int h)        override;

    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*)  override;
    void keyPressEvent(QKeyEvent*)     override;

private Q_SLOTS:

    void moveSlot();
    void slotMouseMoveTimeOut();
    void slotClose();

private:

    /// @note disabled
    explicit PresentationKB(QWidget*) = delete;

private:

    class Private;
    Private* const d = nullptr;

    friend class KBEffect;
};

} // namespace DigikamGenericPresentationPlugin
