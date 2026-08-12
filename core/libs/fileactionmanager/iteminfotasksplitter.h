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

#pragma once

// Local includes

#include "fileactionimageinfolist.h"

namespace Digikam
{

class ItemInfoTaskSplitter : public FileActionItemInfoList
{
public:

    explicit ItemInfoTaskSplitter(const FileActionItemInfoList& list);
    ~ItemInfoTaskSplitter() = default;

    FileActionItemInfoList next();
    bool hasNext() const;

protected:

    int m_n = 1;
};

} // namespace Digikam
