/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View for DImg preview
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "graphicsdimgview.h"

// Qt includes

#include <QApplication>
#include <QGraphicsScene>
#include <QScrollBar>
#include <QToolButton>
#include <QStyle>
#include <QTimer>
#include <QCursor>
#include <QGesture>
#include <QPinchGesture>
#include <QPanGesture>
#include <QSwipeGesture>

// Local includes

#include "digikam_debug.h"
#include "dimgpreviewitem.h"
#include "magnifieritem.h"
#include "imagezoomsettings.h"
#include "paniconwidget.h"
#include "singlephotopreviewlayout.h"
#include "dimgchilditem.h"

namespace Digikam
{

class Q_DECL_HIDDEN GraphicsDImgView::Private
{
public:

    Private() = default;

public:

    QGraphicsScene*           scene             = nullptr;
    GraphicsDImgItem*         item              = nullptr;
    SinglePhotoPreviewLayout* layout            = nullptr;

    PanIconWidget*            pan               = nullptr;

    QPoint                    mousePressPos;
    QPoint                    panningScrollPos;
    bool                      movingInProgress  = false;
    bool                      showText          = true;

    MagnifierItem*            magnifier         = nullptr;
    bool                      magnifierEnabled  = false;
};

} // namespace Digikam
