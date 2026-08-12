/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-03
 * Description : A wrapper send desktop notifications
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QPixmap>

// Local includes

#include "digikam_export.h"

class QWidget;

namespace Digikam
{

/**
 * @brief Show a notification using KNotify, or KPassivePopup if KNotify is unavailable
 * @param eventId     Event id for this notification, KNotification::Notification
 *                    is used if this is empty. Events have to be configured in
 *                    digikam.notifyrc
 * @param message     Message to display
 * @param parent      Widget which owns the notification
 * @param windowTitle Title of the notification window (only used for KPassivePopup)
 * @param pixmap      Pixmap to show in the notification, in addition to the digikam logo.
 */
void DIGIKAM_EXPORT DNotificationWrapper(const QString& eventId, const QString& message,
                                         QWidget* const parent, const QString& windowTitle,
                                         const QPixmap& pixmap = QPixmap());
} // namespace Digikam
