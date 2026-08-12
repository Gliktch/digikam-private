/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-23
 * Description : Core database DBus interface description
 *
 * SPDX-FileCopyrightText: 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "coredbwatchadaptor.h"

CoreDbWatchAdaptor::CoreDbWatchAdaptor(Digikam::CoreDbWatch* const watch)
    : QDBusAbstractAdaptor(watch)
{
    setAutoRelaySignals(true);
}

#include "moc_coredbwatchadaptor.cpp"
