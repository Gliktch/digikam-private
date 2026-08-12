/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Foundation for all machine learning classifiers
 *
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// C++ includes

#include <limits>

// Qt includes

#include <QMutex>
#include <QWaitCondition>
#include <QQueue>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

template <typename T>
class DIGIKAM_EXPORT SharedQueue
{
public:

    SharedQueue()  = default;
    ~SharedQueue() = default;

public:

    T& front()
    {
        QMutexLocker locker(&mutex_);

        while (queue_.isEmpty())
        {
            front_.wait(&mutex_);
        }

        return queue_.head();
    }

    T pop_front()
    {
        QMutexLocker locker(&mutex_);

        while (queue_.isEmpty())
        {
            front_.wait(&mutex_);
        }

        T result = queue_.dequeue();
        back_.wakeOne();

        return result;
    }

    void push_back(T const& item)
    {
        QMutexLocker locker(&mutex_);

        while (maxDepth_ <= queue_.size())
        {
            back_.wait(&mutex_);
        }

        queue_.enqueue(item);
        front_.wakeOne();     ///< Notify one waiting thread.
    }

    void push_back(T&& item)
    {
        QMutexLocker locker(&mutex_);

        while (maxDepth_ <= queue_.size())
        {
            back_.wait(&mutex_);
        }

        queue_.enqueue(std::move(item));
        front_.wakeOne();     ///< Notify one waiting thread.
    }

    int size()
    {
        QMutexLocker locker(&mutex_);

        return queue_.size();
    }

    bool isEmpty()
    {
        QMutexLocker locker(&mutex_);

        return queue_.isEmpty();
    }

    void clear()
    {
        QMutexLocker locker(&mutex_);
        queue_.clear();
    }

    int maxDepth() const
    {
        return maxDepth_;
    }

    void setMaxDepth(int depth)
    {
        maxDepth_ = depth;
    }

    int maxDepthLimit() const
    {
        return std::numeric_limits<int>::max();
    }

    void cancel(T const& queueEndSignal)
    {
        QMutexLocker locker(&mutex_);
        maxDepth_ = std::numeric_limits<int>::max();
        queue_.push_front(queueEndSignal);
        front_.wakeOne();
        back_.wakeAll();
    }

private:

    QQueue<T>               queue_;
    QMutex                  mutex_;
    QWaitCondition          front_;
    QWaitCondition          back_;
    qsizetype               maxDepth_ = std::numeric_limits<int>::max();
};

} // namespace Digikam
