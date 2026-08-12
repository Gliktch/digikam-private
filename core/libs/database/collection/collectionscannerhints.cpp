/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-09
 * Description : Hint data containers for the collection scanner
 *
 * SPDX-FileCopyrightText: 2008      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "collectionscannerhints.h"

namespace Digikam
{

ItemMetadataAdjustmentHint::ItemMetadataAdjustmentHint(qlonglong id, AdjustmentStatus status,
                                                       const QDateTime& modificationDateOnDisk,
                                                       qlonglong fileSize)
    : m_id              (id),
      m_status          (status),
      m_modificationDate(modificationDateOnDisk),
      m_fileSize        (fileSize)
{
}

qlonglong ItemMetadataAdjustmentHint::id()  const
{
    return m_id;
}

ItemMetadataAdjustmentHint::AdjustmentStatus ItemMetadataAdjustmentHint::adjustmentStatus() const
{
    return m_status;
}

QDateTime ItemMetadataAdjustmentHint::modificationDate() const
{
    return m_modificationDate;
}

qlonglong ItemMetadataAdjustmentHint::fileSize() const
{
    return m_fileSize;
}

#ifdef HAVE_DBUS

ItemMetadataAdjustmentHint& ItemMetadataAdjustmentHint::operator<<(const QDBusArgument& argument)
{
    argument.beginStructure();
    int status;
    argument >> m_id
             >> status
             >> m_modificationDate
             >> m_fileSize;
    argument.endStructure();

    return *this;
}

const ItemMetadataAdjustmentHint& ItemMetadataAdjustmentHint::operator>>(QDBusArgument& argument) const
{
    argument.beginStructure();
    argument << m_id
             << (int)m_status
             << m_modificationDate
             << m_fileSize;
    argument.endStructure();

    return *this;
}

#endif

} // namespace Digikam
