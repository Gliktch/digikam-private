/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : image file IO threaded interface.
 *
 * SPDX-FileCopyrightText: 2005-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "sharedloadsavethread.h"

// Local includes

#include "loadingcache.h"
#include "loadsavetask.h"

namespace Digikam
{

SharedLoadSaveThread::SharedLoadSaveThread(QObject* const parent)
    : ManagedLoadSaveThread(parent)
{
}

void SharedLoadSaveThread::loadShared(const LoadingDescription& description,
                                      AccessMode mode,
                                      LoadingPolicy policy)
{
    ManagedLoadSaveThread::load(description, LoadingModeShared, policy, mode);
}

} // namespace Digikam

#include "moc_sharedloadsavethread.cpp"
