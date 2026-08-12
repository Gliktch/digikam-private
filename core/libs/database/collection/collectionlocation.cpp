/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-09
 * Description : Collection location abstraction
 *
 * SPDX-FileCopyrightText: 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "collectionlocation.h"

namespace Digikam
{

int CollectionLocation::id() const
{
    return m_id;
}

Qt::CaseSensitivity CollectionLocation::asQtCaseSensitivity() const
{
    if (m_caseSensitivity == CaseInsensitive)
    {
        return Qt::CaseInsensitive;
    }

    return Qt::CaseSensitive;
}

CollectionLocation::CaseSensitivity CollectionLocation::caseSensitivity() const
{
    return m_caseSensitivity;
}

CollectionLocation::Status CollectionLocation::status() const
{
    return m_status;
}

CollectionLocation::Type CollectionLocation::type() const
{
    return m_type;
}

QString CollectionLocation::albumRootPath() const
{
    return m_path;
}

QString CollectionLocation::label() const
{
    return m_label;
}

} // namespace Digikam
