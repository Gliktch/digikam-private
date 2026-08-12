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

#include "recognitiontrainingupdatequeue.h"

#include "digikam_debug.h"
#include "sharedqueue.h"

namespace Digikam
{

SharedQueue<QString>  RecognitionTrainingUpdateQueue::queue;
QList<const QThread*> RecognitionTrainingUpdateQueue::readers;
int                   RecognitionTrainingUpdateQueue::ref = 0;

RecognitionTrainingUpdateQueue::RecognitionTrainingUpdateQueue()
{
    ++ref;
}

RecognitionTrainingUpdateQueue::~RecognitionTrainingUpdateQueue()
{
    --ref;

    if (0 == ref)
    {
        qCDebug(DIGIKAM_FACEDB_LOG) << "Remove queue destroyed";
    }
}

void RecognitionTrainingUpdateQueue::push(const QString& hash)
{
    QString val = hash;
    queue.push_back(val);
}

QString RecognitionTrainingUpdateQueue::pop_front()
{
    QString result = queue.pop_front();

    return result;
}

QString RecognitionTrainingUpdateQueue::front()
{
    return queue.front();
}

QString RecognitionTrainingUpdateQueue::endSignal()
{
    return QLatin1String("TERMINATE");
}

void RecognitionTrainingUpdateQueue::registerReaderThread(const QThread* thread)
{
    readers.append(thread);
}

void RecognitionTrainingUpdateQueue::unregisterReaderThread(const QThread* thread)
{
    const int index = readers.indexOf(thread);

    if (index >= 0)
    {
        readers.removeAt(index);
    }
}

} // namespace Digikam
