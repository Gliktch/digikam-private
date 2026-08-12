/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2026-06-13
 * Description : Filename matching helper for timelapse grouping
 *
 * SPDX-FileCopyrightText: 2026 by François Martin <kde at fmartin dot ch>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT TimelapseFilenameMatch
{
public:

    TimelapseFilenameMatch() = default;

    explicit TimelapseFilenameMatch(const QString& filename);

    bool directlyPreceeds(TimelapseFilenameMatch const& other) const;

public:

    qulonglong value            = 0;
    qsizetype  valueLength      = 0;
    QString   prefix;
    QString   suffix;
    bool      containsValue    = false;
};

DIGIKAM_GUI_EXPORT QList<qsizetype> timelapseFilenameSequenceOrder(const QList<TimelapseFilenameMatch>& nameSortedMatches);

} // namespace Digikam
