/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-30
 * Description : Base class for altitude lookup jobs
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lookupaltitude.h"

namespace Digikam
{

LookupAltitude::LookupAltitude(QObject* const parent)
    : QObject(parent)
{
}

} // namespace Digikam

#include "moc_lookupaltitude.cpp"
