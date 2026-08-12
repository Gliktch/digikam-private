/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-12-28
 * Description : test for implementation of thread manager api
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2014 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QUrl>
#include <QDialog>

class QProgressBar;

class ProcessorDlg : public QDialog
{
    Q_OBJECT

public:

    explicit ProcessorDlg(const QList<QUrl>& list, QWidget* const parent = nullptr);
    ~ProcessorDlg() override;

private :

    QProgressBar* findProgressBar(const QUrl& url) const;
    void updateCount();

private Q_SLOTS:

    void slotStart();
    void slotStop();
    void slotStarting(const QUrl&);
    void slotProgress(const QUrl&, int);
    void slotFinished(const QUrl&);
    void slotFailed(const QUrl&, const QString&);

private:

    class Private;
    Private* const d = nullptr;
};
