/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface - Xmp helpers.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011      by Leif Huhn <leif at dkstat dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmetadata.h"

// Qt includes

#include <QLocale>

// Local includes

#include "metaenginesettings.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

QVariant DMetadata::fromXmpList(const char* const xmpTagName) const
{
    QVariant var = getXmpTagVariant(xmpTagName);

    if (var.isNull())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        return QVariant(QMetaType(QMetaType::QStringList));

#else

        return QVariant(QVariant::StringList);

#endif

    }

    return var;
}

QVariant DMetadata::fromXmpLangAlt(const char* const xmpTagName) const
{
    QVariant var = getXmpTagVariant(xmpTagName);

    if (var.isNull())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        return QVariant(QMetaType(QMetaType::QVariantMap));

#else

        return QVariant(QVariant::Map);

#endif

    }

    return var;
}

bool DMetadata::removeXmpTags(const QStringList& tagFilters)
{
    MetaDataMap m = getXmpTagsDataList(tagFilters);

    if (m.isEmpty())
    {
        return false;
    }

    for (MetaDataMap::iterator it = m.begin() ; it != m.end() ; ++it)
    {
        removeXmpTag(it.key().toLatin1().constData());
    }

    return true;
}

} // namespace Digikam
