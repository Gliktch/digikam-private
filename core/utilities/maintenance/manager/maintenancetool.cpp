/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : maintenance tool
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "maintenancetool.h"

// Qt includes

#include <QTime>
#include <QTimer>
#include <QElapsedTimer>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "dnotificationwrapper.h"

namespace Digikam
{

class Q_DECL_HIDDEN MaintenanceTool::Private
{
public:

    Private() = default;

public:

    bool          notification = true;
    uint          advanceSteps = 1;

    QTimer        progressTimer;
    QElapsedTimer duration;
};

MaintenanceTool::MaintenanceTool(const QString& id, ProgressItem* const parent)
    : ProgressItem(parent, id, QString(), QString(), true, true),
      d           (new Private)
{
    // --- NOTE: use dynamic binding as slotCancel() is a virtual method which can be re-implemented in derived classes.

#ifdef Q_OS_WIN

    connect(this, SIGNAL(progressItemCanceledById(QString)),
            this, SLOT(slotCancel()));

#else

    connect(this, static_cast<void (ProgressItem::*)(const QString&)>(&ProgressItem::progressItemCanceledById),
            this, &MaintenanceTool::slotCancel);

#endif

    d->progressTimer.setSingleShot(true);
}

MaintenanceTool::~MaintenanceTool()
{
    delete d;
}

void MaintenanceTool::setNotificationEnabled(bool b)
{
    d->notification = b;
}

uint MaintenanceTool::checkProgressNeeded() const
{
    if (!d->progressTimer.isActive())
    {
        uint adv        = d->advanceSteps;
        d->progressTimer.start(50);
        d->advanceSteps = 1;

        return adv;
    }

    d->advanceSteps++;

    return 0;
}

void MaintenanceTool::start()
{
    if (ProgressManager::instance()->findItembyId(id()))
    {
        QTimer::singleShot(2000, this, SLOT(start()));

        return;
    }

    ProgressManager::addProgressItem(this);

    // We delay start to be sure that eventloop
    // connect signals and slots in top level.

    QTimer::singleShot(0, this, SLOT(slotStart()));
}

void MaintenanceTool::slotStart()
{
    d->duration.start();
}

void MaintenanceTool::slotDone()
{
    QTime t = QTime::fromMSecsSinceStartOfDay(d->duration.elapsed());

    if (d->notification)
    {
        // Pop-up a message to bring user when all is done.

        DNotificationWrapper(id(),
                             i18n("Process is done.\nDuration: %1", t.toString()),
                             qApp->activeWindow(), label());
    }

    Q_EMIT signalComplete();

    setComplete();
}

void MaintenanceTool::slotCancel()
{
    setComplete();

    Q_EMIT signalCanceled();
}

} // namespace Digikam

#include "moc_maintenancetool.cpp"
