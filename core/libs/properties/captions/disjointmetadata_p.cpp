/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-17
 * Description : Helper class for Image Description Editor Tab
 *
 * SPDX-FileCopyrightText: 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "disjointmetadata_p.h"

namespace Digikam
{

DisjointMetadata::Private::Private()
{
}

DisjointMetadata::Private::Private(const DisjointMetadataDataFields& other)
    : DisjointMetadataDataFields(other)
{
}

DisjointMetadata::Private::Private(const Private& other)
    : DisjointMetadataDataFields(other)
{
}

void DisjointMetadata::Private::makeConnections(DisjointMetadata* const q)
{
    QObject::connect(TagsCache::instance(), SIGNAL(tagDeleted(int)),
                     q, SLOT(slotTagDeleted(int)),
                     Qt::DirectConnection);

    QObject::connect(CoreDbAccess::databaseWatch(), SIGNAL(databaseChanged()),
                     q, SLOT(slotInvalidate()));
}

} // namespace Digikam
