/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-12-28
 * Description : Low level threads management for batch processing on multi-core
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2014 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2012 by Janardhan Reddy <annapareddyjanardhanreddy at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "actionthreadbase.h"

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

ActionJob::ActionJob(QObject* const parent)
    : QObject  (parent),
      QRunnable()
{
    setAutoDelete(false);
}

ActionJob::~ActionJob()
{
    cancel();
}

void ActionJob::cancel()
{
    m_cancel = true;
}

void ActionJob::run()
{
    QString name = objectName();

    if (name.isEmpty())
    {
        // No explicit name set with QObject::setObjectName(): fall back
        // to the class name of the derived job implementation.

        name = QString::fromLatin1(metaObject()->className());
    }

    ActionThreadBase::setCurrentThreadName(name);
}

} // namespace Digikam

#include "moc_actionjob.cpp"
