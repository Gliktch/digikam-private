/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Data set for item lister
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QDataStream>
#include <QDateTime>
#include <QSize>
#include <QList>
#include <QVariant>

// Local includes

#include "digikam_export.h"
#include "coredbconstants.h"
#include "coredbalbuminfo.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ItemListerRecord
{

public:

    ItemListerRecord() = default;

    bool operator==(const ItemListerRecord& record) const;

public:

    int                    albumID               = -1;
    int                    albumRootID           = -1;
    int                    rating                = -1;

    qlonglong              fileSize              = -1;
    qlonglong              imageID               = -1;
    qlonglong              currentReferenceImage = -1;

    double                 currentSimilarity     = 0.0;

    QString                format;
    QString                name;

    QDateTime              creationDate;
    QDateTime              modificationDate;

    QSize                  imageSize;

    DatabaseItem::Category category              = DatabaseItem::UndefinedCategory;

    QList<QVariant>        extraValues;
};
/*
DIGIKAM_DATABASE_EXPORT QDataStream& operator<<(QDataStream& os, const ItemListerRecord& record);
DIGIKAM_DATABASE_EXPORT QDataStream& operator>>(QDataStream& ds, ItemListerRecord& record);
*/
} // namespace Digikam
