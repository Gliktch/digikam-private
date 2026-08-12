/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-10-17
 * Description : test for implementation of DItemsList api
 *
 * SPDX-FileCopyrightText: 2011-2012 by A Janardhan Reddy <annapareddyjanardhanreddy at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QDialog>

// Local includes

#include "actionthreadbase.h"

using namespace Digikam;

class ActionThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit ActionThread(QObject* const parent);
    ~ActionThread() override;

    void rotate(const QList<QUrl>& list);

Q_SIGNALS:

    void signalStarting(const QUrl& url);
    void signalFinished(const QUrl& url);
    void signalFailed(const QUrl& url, const QString& err);
    void signalProgress(const QUrl& url, int percent);

private Q_SLOTS:

    void slotJobDone();
    void slotJobProgress(int);
    void slotJobStarted();
};

// -----------------------------------------------------

class DItemsListTest : public QDialog
{
    Q_OBJECT

public:

    explicit DItemsListTest(QWidget* const parent = nullptr);
    ~DItemsListTest() override;

private Q_SLOTS:

    void slotStart();
    void slotStarting(const QUrl&);
    void slotFinished(const QUrl&);
    void slotFailed(const QUrl&, const QString&);

private:

    class Private;
    Private* const d = nullptr;
};
