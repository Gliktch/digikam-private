/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : Thread actions container.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QMetaType>
#include <QUrl>

namespace Digikam
{

class ActionData
{

public:

    enum ActionStatus
    {
        None = 0,
        BatchStarted,
        BatchDone,
        BatchFailed,
        BatchSkipped,
        BatchCanceled,
        TaskDone,
        TaskFailed,
        TaskCanceled
    };

public:

    ActionData() = default;

    ActionStatus status     = None;

    QString      message;

    QUrl         fileUrl;
    QUrl         destUrl;

    bool         noWrite    = false;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ActionData)
