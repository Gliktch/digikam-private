/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-01
 * Description : Qt model view for Showfoto item - the delegate
 *
 * SPDX-FileCopyrightText: 2013 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "showfotoitemviewdelegate.h"

// Qt includes

#include <QPainter>
#include <QApplication>
#include <QFont>
#include <QModelIndex>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "thememanager.h"
#include "itempropertiestab.h"
#include "showfotoiteminfo.h"
#include "colorlabelwidget.h"
#include "thumbnailsize.h"
#include "itemdelegateoverlay.h"

using namespace Digikam;

namespace ShowFoto
{

class ShowfotoItemViewDelegate;

class Q_DECL_HIDDEN ShowfotoItemViewDelegatePrivate
{
public:

    ShowfotoItemViewDelegatePrivate()          = default;
    virtual ~ShowfotoItemViewDelegatePrivate() = default;

    void init(ShowfotoItemViewDelegate* const _q, QWidget* const _widget);

    /// Resets cached rects. Remember to reimplement in subclass for added rects.
    virtual void clearRects();

public:

    int                       spacing           = 0;
    QSize                     gridSize;

    QRect                     rect;

    QPixmap                   regPixmap;
    QPixmap                   selPixmap;
    QVector<QPixmap>          ratingPixmaps;

    QFont                     font;
    QFont                     fontReg;
    QFont                     fontCom;
    QFont                     fontXtra;

    ThumbnailSize             thumbSize         = ThumbnailSize(0);

    ShowfotoItemViewDelegate* q                 = nullptr;

    QWidget*                  displayWidget     = nullptr;

    QRect                     oneRowRegRect;
    QRect                     oneRowComRect;
    QRect                     oneRowXtraRect;

    /// constant values for drawing
    int                       radius            = 3;
    int                       margin            = 5;

private:

    Q_DISABLE_COPY(ShowfotoItemViewDelegatePrivate)
};

} // namespace ShowFoto
