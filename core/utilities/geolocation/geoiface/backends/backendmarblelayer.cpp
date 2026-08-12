/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-08
 * Description : Internal part of the Marble-backend for geolocation interface
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "backendmarblelayer.h"

// Marble includes

#include "GeoPainter.h"

// Local includes

#include "backendmarble.h"

namespace Digikam
{

BackendMarbleLayer::BackendMarbleLayer(BackendMarble* const pMarbleBackend)
    : marbleBackend(pMarbleBackend)
{
}

bool BackendMarbleLayer::render(Marble::GeoPainter* painter,
                                Marble::ViewportParams* /*viewport*/,
                                const QString& renderPos,
                                Marble::GeoSceneLayer* /*layer*/)
{
    if (marbleBackend && (renderPos == QLatin1String("HOVERS_ABOVE_SURFACE")))
    {
        marbleBackend->marbleCustomPaint(painter);

        return true;
    }

    return false;
}

QStringList BackendMarbleLayer::renderPosition () const
{
    QStringList layerNames;
    layerNames << QLatin1String("HOVERS_ABOVE_SURFACE" );

    return layerNames;
}

void BackendMarbleLayer::setBackend(BackendMarble* const pMarbleBackend)
{
    marbleBackend = pMarbleBackend;
}

} // namespace Digikam
