/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : a busy dialog for digiKam
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbusydlg.h"

// Qt includes

#include <QPushButton>
#include <QProgressBar>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DBusyThread::DBusyThread(QObject* const parent)
    : QThread(parent)
{
}

DBusyThread::~DBusyThread()
{
    wait();
}

// ----------------------------------------------------------------------------------

class Q_DECL_HIDDEN DBusyDlg::Private
{
public:

    Private() = default;

public:

    DBusyThread* thread = nullptr;
};

DBusyDlg::DBusyDlg(const QString& txt, QWidget* const parent)
    : QProgressDialog(parent, Qt::FramelessWindowHint),
      d              (new Private)
{
    setLabelText(txt);
    setCancelButton(nullptr);
    setMinimumDuration(0);
    setModal(true);
    setAutoClose(false);

    setMaximum(0);
    setMinimum(0);
    setValue(0);
}

DBusyDlg::~DBusyDlg()
{
    delete d;
}

void DBusyDlg::setBusyThread(DBusyThread* const thread)
{
    d->thread = thread;

    if (d->thread)
    {
        connect(d->thread, SIGNAL(signalComplete()),
                this, SLOT(slotComplete()));

        qCDebug(DIGIKAM_GENERAL_LOG) << "Thread is started";
        d->thread->start();
    }
}

void DBusyDlg::slotComplete()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Thread is complete";
    accept();
}

} // namespace Digikam

#include "moc_dbusydlg.cpp"
