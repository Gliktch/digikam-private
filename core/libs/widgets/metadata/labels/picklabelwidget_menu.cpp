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

#include "picklabelwidget_p.h"

namespace Digikam
{

PickLabelMenuAction::PickLabelMenuAction(QMenu* const parent)
    : QMenu(parent)
{
    setTitle(i18nc("@title: pick label menu", "Pick"));
    QWidgetAction* const wa    = new QWidgetAction(this);
    PickLabelWidget* const plw = new PickLabelWidget(parent);
    wa->setDefaultWidget(plw);
    addAction(wa);

    connect(plw, SIGNAL(signalPickLabelChanged(int)),
            this, SIGNAL(signalPickLabelChanged(int)));

    connect(plw, SIGNAL(signalPickLabelChanged(int)),
            parent, SLOT(close()));
}

} // namespace Digikam
