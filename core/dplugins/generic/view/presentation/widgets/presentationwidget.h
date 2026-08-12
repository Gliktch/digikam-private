/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-02-16
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2006-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText:      2009 by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2021 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QMap>
#include <QPair>
#include <QPixmap>
#include <QPolygon>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <QRandomGenerator>

class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QWheelEvent;

namespace DigikamGenericPresentationPlugin
{

class PresentationContainer;

class PresentationWidget : public QWidget
{
    Q_OBJECT

public:

    typedef int (PresentationWidget::*EffectMethod)(bool);

public:

    explicit PresentationWidget(PresentationContainer* const sharedData);
    ~PresentationWidget()                 override;

    void registerEffects();

    static QStringList            effectNames();
    static QMap<QString, QString> effectNamesI18N();

protected:

    void    mousePressEvent(QMouseEvent*) override;
    void    mouseMoveEvent(QMouseEvent*)  override;
    void    wheelEvent(QWheelEvent*)      override;
    void    keyPressEvent(QKeyEvent*)     override;

    int     effectNone(bool);
    int     effectChessboard(bool doInit);
    int     effectMeltdown(bool doInit);
    int     effectSweep(bool doInit);
    int     effectMosaic(bool doInit);
    int     effectCubism(bool doInit);
    int     effectRandom(bool doInit);
    int     effectGrowing(bool doInit);
    int     effectHorizLines(bool doInit);
    int     effectVertLines(bool doInit);
    int     effectMultiCircleOut(bool doInit);
    int     effectSpiralIn(bool doInit);
    int     effectCircleOut(bool doInit);
    int     effectBlobs(bool doInit);

    void    paintEvent(QPaintEvent*)      override;
    void    startPainter();

protected:

    bool    m_simplyShow    = false;
    bool    m_startPainter  = false;
    bool    m_firstPainter  = true;
    int     m_px            = 0;
    int     m_py            = 0;
    int     m_psx           = 0;
    int     m_psy           = 0;
    bool    m_endOfShow     = false;
    QPixmap m_buffer;

private Q_SLOTS:

    void    slotTimeOut();
    void    slotMouseMoveTimeOut();

    void    slotPause();
    void    slotPlay();
    void    slotPrev();
    void    slotNext();
    void    slotClose();
    void    slotRemoveImageFromList();

    void    slotVideoLoaded(bool);
    void    slotVideoFinished();

private:

    void         loadNextImage();
    void         loadPrevImage();
    void         showCurrentImage();
    void         printFilename();
    void         printComments();
    void         printProgress();
    void         showEndOfShow();
    void         showOverlays();
    void         hideOverlays();
    void         readSettings();

    EffectMethod getRandomEffect();

private:

    /// @note disabled
    explicit PresentationWidget(QWidget*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPresentationPlugin
