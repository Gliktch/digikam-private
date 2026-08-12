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

#include "colorlabelwidget_p.h"

namespace Digikam
{

ColorLabelMenuAction::ColorLabelMenuAction(QMenu* const parent)
    : QMenu(parent)
{
    setTitle(i18nc("@title: color label menu", "Color"));
    QWidgetAction* const wa     = new QWidgetAction(this);
    ColorLabelWidget* const clw = new ColorLabelWidget(parent);
    wa->setDefaultWidget(clw);
    addAction(wa);

    connect(clw, SIGNAL(signalColorLabelChanged(int)),
            this, SIGNAL(signalColorLabelChanged(int)));

    connect(clw, SIGNAL(signalColorLabelChanged(int)),
            parent, SLOT(close()));
}

} // namespace Digikam
