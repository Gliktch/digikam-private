/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-17
 * Description : Face tag region item
 *
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "assignnamewidgetstates.h"

// Local includes

#include "digikam_debug.h"
#include "faceitem.h"
#include "assignnamewidget.h"

namespace Digikam
{

AssignNameWidgetStates::AssignNameWidgetStates(FaceItem* const item)
    : HidingStateChanger(item->widget(), "mode", item)
{
    // The WidgetProxyItem.

    addItem(item->hudWidget());

    connect(this, SIGNAL(stateChanged()),
            this, SLOT(slotStateChanged()));
}

void AssignNameWidgetStates::slotStateChanged()
{
    FaceItem* const item = static_cast<FaceItem*>(parent());

    // Show resize handles etc. only in edit modes.

    item->setEditable(
        !(
            (item->widget()->mode() == AssignNameWidget::IgnoredMode)  ||
            (item->widget()->mode() == AssignNameWidget::ConfirmedMode)
        )
    );
}

} // namespace Digikam

#include "moc_assignnamewidgetstates.cpp"
