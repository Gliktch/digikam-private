/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-18
 * Description : item info task splitter
 *
 * SPDX-FileCopyrightText: 2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Local includes

#include "iteminfotasksplitter.h"

// Qt includes

#include <QThread>

namespace Digikam
{

ItemInfoTaskSplitter::ItemInfoTaskSplitter(const FileActionItemInfoList& list)
    : FileActionItemInfoList(list)
{
    int parts = qMax(1, QThread::idealThreadCount());
    m_n       = qMax(1, list.size() / parts);
}

FileActionItemInfoList ItemInfoTaskSplitter::next()
{
    QList<ItemInfo> list;

    if (size() <= m_n)
    {
        list = *this;
        clear();
    }
    else
    {
        list.reserve(m_n);

        // qCopy does not work with QList

        for (int i = 0 ;  i < m_n ; ++i)
        {
            list << at(i);
        }

        erase(begin(), begin() + m_n);
    }

    return FileActionItemInfoList::continueTask(list, progress());
}

bool ItemInfoTaskSplitter::hasNext() const
{
    return !isEmpty();
}

} // namespace Digikam
