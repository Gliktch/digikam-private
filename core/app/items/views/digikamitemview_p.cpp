/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-10-03
 * Description : Private Qt model-view for items
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Johannes Wienke <languitar at semipol do de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "digikamitemview_p.h"

// Local includes

#include "contextmenuhelper.h"

namespace Digikam
{

DigikamItemView::Private::Private(DigikamItemView* const qq)
    : q_ptr(qq)
{
}

void DigikamItemView::Private::updateOverlays()
{
    Q_Q(DigikamItemView);

    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (overlaysActive)
    {
        if (!settings->getIconShowOverlays())
        {
            disconnect(rotateLeftOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
                       q, SLOT(slotRotateLeft(QList<QModelIndex>)));

            disconnect(rotateRightOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
                       q, SLOT(slotRotateRight(QList<QModelIndex>)));

            q->removeOverlay(rotateLeftOverlay);
            q->removeOverlay(rotateRightOverlay);

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

    if (fullscreenActive)
    {
        if (!settings->getIconShowFullscreen())
        {
            disconnect(fullscreenOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
                       q, SLOT(slotFullscreen(QList<QModelIndex>)));

            q->removeOverlay(fullscreenOverlay);

            fullscreenActive = false;
        }
    }
    else
    {
        if (settings->getIconShowFullscreen())
        {
            fullscreenActive = true;

            q->addOverlay(fullscreenOverlay, normalDelegate);

            connect(fullscreenOverlay, SIGNAL(signalFullscreen(QList<QModelIndex>)),
                    q, SLOT(slotFullscreen(QList<QModelIndex>)));
        }
    }
}

} // namespace Digikam

#include "moc_digikamitemview_p.cpp"
