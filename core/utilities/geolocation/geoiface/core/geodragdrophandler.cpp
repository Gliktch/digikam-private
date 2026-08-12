/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-18
 * Description : Drag-and-drop handler for geolocation interface
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Local includes

#include "geodragdrophandler.h"

namespace Digikam
{

GeoDragDropHandler::GeoDragDropHandler(QObject* const parent)
    : QObject(parent)
{
}

GeoDragDropHandler::~GeoDragDropHandler()
{
}

} // namespace Digikam

#include "moc_geodragdrophandler.cpp"
