/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-13
 * Description : printer thread.
 *
 * SPDX-FileCopyrightText: 2008      by Orgad Shaneh <orgads at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMap>
#include <QThread>
#include <QUrl>

class QPrinter;

namespace DigikamGenericCalendarPlugin
{

class CalPainter;

class CalPrinter : public QThread
{
    Q_OBJECT

public:

    explicit CalPrinter(QPrinter* const printer,
                        const QMap<int, QUrl>& months,
                        QObject* const parent);

    ~CalPrinter() override;

protected:

    void run()    override;

Q_SIGNALS:

    void pageChanged(int page);
    void totalBlocks(int total);
    void blocksFinished(int finished);

public Q_SLOTS:

    void cancel();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
