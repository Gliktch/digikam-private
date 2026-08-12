/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Shared data container.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metaengine_data_p.h"

// Local includes

#include "metaengine_p.h"

namespace Digikam
{

MetaEngineData::MetaEngineData()
    : d(nullptr)
{
}

MetaEngineData::MetaEngineData(const MetaEngineData& other)
    : d(other.d)
{
}

MetaEngineData::~MetaEngineData()
{
}

int MetaEngineData::size() const
{
    if (d)
    {
        return d->size();
    }

    return 0;
}

MetaEngineData& MetaEngineData::operator=(const MetaEngineData& other)
{
    d = other.d;

    return *this;
}

} // namespace Digikam
