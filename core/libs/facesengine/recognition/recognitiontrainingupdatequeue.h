/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2024-10-28
 * Description : Threadsafe queue for submitting face training removal requests.
 *
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <micahel underscore miller at msn dot com>
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QThread>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

template <typename T>
class SharedQueue;

class DIGIKAM_EXPORT RecognitionTrainingUpdateQueue
{
public:

    RecognitionTrainingUpdateQueue();
    ~RecognitionTrainingUpdateQueue();

public:

    void push(const QString& hash);
    QString pop_front();
    QString front();

    QString endSignal();

    void registerReaderThread(const QThread* thread);
    void unregisterReaderThread(const QThread* thread);

private:

    static SharedQueue<QString>  queue;
    static QList<const QThread*> readers;
    static int                   ref;

private:

    Q_DISABLE_COPY(RecognitionTrainingUpdateQueue)
};

} // namespace Digikam
