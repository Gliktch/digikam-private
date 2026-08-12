/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-01-19
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText:      2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2009 by Valerio Fuoglio <valerio.fuoglio@gmail.com>
 * SPDX-FileCopyrightText:      2009 by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2021 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#ifdef Q_CC_MSVC
#   include <winsock2.h>
#endif

// Qt includes

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QList>
#include <QMap>
#include <QMouseEvent>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QWheelEvent>
#include <QRandomGenerator>

namespace DigikamGenericPresentationPlugin
{

class PresentationContainer;

class PresentationGL : public QOpenGLWidget
{
    Q_OBJECT

public:

    explicit PresentationGL(PresentationContainer* const sharedData);
    ~PresentationGL()                  override;

    void registerEffects();

    static QStringList effectNames();
    static QMap<QString, QString> effectNamesI18N();

    bool checkOpenGL() const;

protected:

    void initializeGL()                override;
    void paintGL()                     override;
    void resizeGL(int w, int h)        override;

    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*)  override;
    void wheelEvent(QWheelEvent*)      override;
    void keyPressEvent(QKeyEvent*)     override;

private:

    typedef void (PresentationGL::*EffectMethod)();

    QPixmap       generateOutlinedTextPixmap(const QString& text);
    QPixmap       generateOutlinedTextPixmap(const QString& text, QFont& fn);
    QPixmap       generateCustomOutlinedTextPixmap(const QString& text,
                                                   QFont& fn, QColor& fgColor, const QColor& bgColor,
                                                   int opacity, bool transBg = true);

    void          paintTexture();
    void          advanceFrame();
    void          previousFrame();
    void          loadImage();
    void          montage(QImage& top, QImage& bot);
    EffectMethod  getRandomEffect();
    void          showEndOfShow();
    void          showOverlays();
    void          hideOverlays();
    void          printFilename(QImage& layer);
    void          printProgress(QImage& layer);
    void          printComments(QImage& layer);

    void          effectNone();
    void          effectBlend();
    void          effectFade();
    void          effectRotate();
    void          effectBend();
    void          effectInOut();
    void          effectSlide();
    void          effectFlutter();
    void          effectCube();

private Q_SLOTS:

    void slotTimeOut();
    void slotMouseMoveTimeOut();

    void slotPause();
    void slotPlay();
    void slotPrev();
    void slotNext();
    void slotClose();

private:

    /// @note disabled
    explicit PresentationGL(QWidget*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPresentationPlugin
