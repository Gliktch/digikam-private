/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-10-16
 * Description : history updater thread for importui
 *
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDateTime>
#include <QMultiMap>
#include <QPair>
#include <QThread>

// Local includes

#include "camiteminfo.h"

class QWidget;

namespace Digikam
{

typedef QMultiMap<QDateTime, CamItemInfo>  CHUpdateItemMap;
typedef QPair<QByteArray, CHUpdateItemMap> CHUpdateItem;

class CameraHistoryUpdater : public QThread
{
    Q_OBJECT

public:

    explicit CameraHistoryUpdater(QWidget* const parent);
    ~CameraHistoryUpdater();

    void addItems(const QByteArray& id, CHUpdateItemMap& map);

Q_SIGNALS:

    void signalBusy(bool val);
    void signalHistoryMap(const CHUpdateItemMap&);

public Q_SLOTS:

    void slotCancel();

protected:

    void run();

private:

    void proccessMap(const QByteArray& id, CHUpdateItemMap& map);
    void sendBusy(bool val);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
