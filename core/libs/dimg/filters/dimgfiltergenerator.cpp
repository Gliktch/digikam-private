/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-24
 * Description : manager for filters (registering, creating etc)
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgfiltergenerator.h"

namespace Digikam
{

bool DImgFilterGenerator::isSupported(const QString& filterIdentifier)
{
    return supportedFilters().contains(filterIdentifier);
}

bool DImgFilterGenerator::isSupported(const QString& filterIdentifier, int version)
{
    if (isSupported(filterIdentifier))
    {
        return supportedVersions(filterIdentifier).contains(version);
    }

    return false;
}

} // namespace Digikam
