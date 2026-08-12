/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-05-01
 * Description : ItemInfo common data
 *
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iteminfodata.h"

// Qt includes

#include <QHash>

namespace Digikam
{

ItemInfoStatic* ItemInfoStatic::m_instance = nullptr;

void ItemInfoStatic::create()
{
    if (!m_instance)
    {
        m_instance = new ItemInfoStatic;
    }
}

void ItemInfoStatic::destroy()
{
    delete m_instance;
    m_instance = nullptr;
}

ItemInfoCache* ItemInfoStatic::cache()
{
    return &m_instance->m_cache;
}

} // namespace Digikam
