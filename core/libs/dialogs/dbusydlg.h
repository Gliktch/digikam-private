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

#pragma once

// Qt includes

#include <QThread>
#include <QString>
#include <QProgressDialog>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DBusyThread : public QThread
{
    Q_OBJECT

public:

    explicit DBusyThread(QObject* const parent);
    ~DBusyThread() override;

Q_SIGNALS:

    void signalComplete();

protected:

    /// Reimplement this method with your code to run in a separate thread.
    void run() override {};
};

// ----------------------------------------------------------------------------------

class DIGIKAM_EXPORT DBusyDlg : public QProgressDialog
{
    Q_OBJECT

public:

    explicit DBusyDlg(const QString& txt, QWidget* const parent = nullptr);
    ~DBusyDlg() override;

    void setBusyThread(DBusyThread* const thread);

public Q_SLOTS:

    void slotComplete();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
