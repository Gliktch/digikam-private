/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-01-28
 * Description : color label widget
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "colorlabelwidget.h"

// Qt includes

#include <QApplication>
#include <QPainter>
#include <QPixmap>
#include <QIcon>
#include <QLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QWidgetAction>
#include <QFontMetrics>
#include <QFont>
#include <QToolButton>

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
#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Restore warnings
#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "dxmlguiwindow.h"
#include "dexpanderbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN ColorLabelWidget::Private
{
public:

    Private() = default;

public:

    QButtonGroup*       colorBtns   = nullptr;

    QToolButton*        btnNone     = nullptr;
    QToolButton*        btnRed      = nullptr;
    QToolButton*        btnOrange   = nullptr;
    QToolButton*        btnYellow   = nullptr;
    QToolButton*        btnGreen    = nullptr;
    QToolButton*        btnBlue     = nullptr;
    QToolButton*        btnMagenta  = nullptr;
    QToolButton*        btnGray     = nullptr;
    QToolButton*        btnBlack    = nullptr;
    QToolButton*        btnWhite    = nullptr;

    DHBox*              descBox     = nullptr;

    DAdjustableLabel*   desc        = nullptr;
    DAdjustableLabel*   shortcut    = nullptr;
};

class Q_DECL_HIDDEN ColorLabelSelector::Private
{

public:

    Private() = default;

public:

    ColorLabelWidget* clw = nullptr;
};

} // namespace Digikam
