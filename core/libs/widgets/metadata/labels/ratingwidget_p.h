/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-08-15
 * Description : a widget to draw stars rating
 *
 * SPDX-FileCopyrightText: 2005      by Owen Hirst <n8rider@sbcglobal.net>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "ratingwidget.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QTimeLine>
#include <QFont>
#include <QAction>
#include <QWidgetAction>

// KDE includes

#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <klocalizedstring.h>
#include <kactioncollection.h>

// Restore warnings
#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "thememanager.h"
#include "dxmlguiwindow.h"
#include "dexpanderbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN RatingWidget::Private
{
public:

    Private() = default;

public:

    bool       tracking         = true;
    bool       isHovered        = false;
    bool       fading           = false;

    int        rating           = 0;
    int        fadingValue      = 0;
    int        duration         = 600;       ///< in milliseconds
    int        offset           = 0;
    int        width            = 15;

    QTimeLine* fadingTimeLine   = nullptr;

    QPixmap    selPixmap;      ///< Selected star.
    QPixmap    regPixmap;      ///< Regular star.
    QPixmap    disPixmap;      ///< Disable star.
};

class Q_DECL_HIDDEN RatingBox::Private
{

public:

    Private() = default;

public:

    DAdjustableLabel* shortcut      = nullptr;

    RatingWidget*     ratingWidget  = nullptr;
};

} // namespace Digikam
