/* ============================================================
 *
 * This file is a part of the digikam project
 * https://www.digikam.org
 *
 * Date        : 2017-01-01
 * Description : button for choosing time difference photo which accepts drag & drop
 *
 * SPDX-FileCopyrightText: 2017 by Markus Leuthold <kusi at forum dot titlis dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "detbyclockphotobutton.h"

// Qt includes

#include <QMimeData>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DetByClockPhotoButton::DetByClockPhotoButton(const QString& text)
    : QPushButton(text)
{
    setAcceptDrops(true);
}

void DetByClockPhotoButton::dragEnterEvent(QDragEnterEvent* event)
{
    event->acceptProposedAction();
}

void DetByClockPhotoButton::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void DetByClockPhotoButton::dropEvent(QDropEvent* event)
{
    const QMimeData* const mimeData = event->mimeData();

    if (mimeData && mimeData->hasUrls())
    {
        // if user dropped more than one image, take the first one

        QUrl url = mimeData->urls().constFirst();
        qCDebug(DIGIKAM_GENERAL_LOG) << "Dropped clock photo:" << url;

        Q_EMIT signalClockPhotoDropped(url);
    }
}

} // namespace Digikam

#include "moc_detbyclockphotobutton.cpp"
