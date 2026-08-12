/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-17
 * Description : low level files management interface - Finder classes.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2018      by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QUrl>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class ItemInfo;

class DIGIKAM_GUI_EXPORT SidecarFinder
{

public:

    explicit SidecarFinder(const QList<QUrl>& files);

public:

    QList<QUrl>    localFiles;
    QList<bool>    localFileModes;
    QList<QString> localFileSuffixes;
};

// -----------------------------------------------------------------------------------------

class DIGIKAM_GUI_EXPORT GroupedImagesFinder
{

public:

    explicit GroupedImagesFinder(const QList<ItemInfo>& source);

public:

    QList<ItemInfo> infos;
};

} // namespace Digikam
