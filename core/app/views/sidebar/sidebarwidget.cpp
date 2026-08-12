/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : base class for sidebar widgets
 *
 * SPDX-FileCopyrightText: 2009      by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "sidebarwidget.h"

namespace Digikam
{

SidebarWidget::SidebarWidget(QWidget* const parent)
    : QWidget          (parent),
      StateSavingObject(this)
{
}

} // namespace Digikam

#include "moc_sidebarwidget.cpp"
