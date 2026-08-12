/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-14
 * Description : pick label widget
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "picklabelwidget.h"

// Qt includes

#include <QPainter>
#include <QIcon>
#include <QLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QWidgetAction>
#include <QFontMetrics>
#include <QFont>
#include <QToolButton>
#include <QApplication>

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

#include "dxmlguiwindow.h"
#include "dexpanderbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN PickLabelWidget::Private
{

public:

    Private() = default;

public:

    QButtonGroup*     pickBtns  = nullptr;

    QLabel*           desc      = nullptr;

    QToolButton*      btnNone   = nullptr;
    QToolButton*      btnRej    = nullptr;
    QToolButton*      btnPndg   = nullptr;
    QToolButton*      btnAccpt  = nullptr;

    DHBox*            descBox   = nullptr;

    DAdjustableLabel* shortcut  = nullptr;
};

class Q_DECL_HIDDEN PickLabelSelector::Private
{
public:

    Private() = default;

public:

    PickLabelWidget* plw = nullptr;
};

} // namespace Digikam
