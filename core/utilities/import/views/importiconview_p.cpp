/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-25-07
 * Description : Private Qt item view for images
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importiconview_p.h"

namespace Digikam
{

ImportIconView::Private::Private(ImportIconView* const qq)
    : q_ptr(qq)
{
}

void ImportIconView::Private::updateOverlays()
{
    Q_Q(ImportIconView);

    ImportSettings* const settings = ImportSettings::instance();

    if (overlaysActive)
    {
        if (!settings->getIconShowOverlays())
        {
            disconnect(rotateLeftOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
                       q, SLOT(slotRotateLeft(QList<QModelIndex>)));

            disconnect(rotateRightOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
                       q, SLOT(slotRotateRight(QList<QModelIndex>)));

            q->removeOverlay(rotateRightOverlay);
            q->removeOverlay(rotateLeftOverlay);

            overlaysActive = false;
        }
    }
    else
    {
        if (settings->getIconShowOverlays())
        {
            q->addOverlay(rotateLeftOverlay, normalDelegate);
            q->addOverlay(rotateRightOverlay, normalDelegate);

            connect(rotateLeftOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
                    q, SLOT(slotRotateLeft(QList<QModelIndex>)));

            connect(rotateRightOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
                    q, SLOT(slotRotateRight(QList<QModelIndex>)));

            overlaysActive = true;
        }
    }
}

} // namespace Digikam

#include "moc_importiconview_p.cpp"
