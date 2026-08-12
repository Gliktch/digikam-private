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

#pragma once

// Qt includes

#include <QPointer>

// Marble includes

#include "LayerInterface.h"

// Local includes

#include "digikam_export.h"

/// @cond false
namespace Marble
{
    class GeoPainter;
    class ViewportParams;
    class GeoSceneLayer;
}
/// @endcond

namespace Digikam
{

class BackendMarble;

class DIGIKAM_EXPORT BackendMarbleLayer : public Marble::LayerInterface
{
public:

    explicit BackendMarbleLayer(BackendMarble* const pMarbleBackend);
    ~BackendMarbleLayer()                                       override = default;

    bool render(Marble::GeoPainter* painter,
                        Marble::ViewportParams* viewport,
                        const QString& renderPos = QLatin1String("NONE"),
                        Marble::GeoSceneLayer* layer = nullptr) override;

    QStringList renderPosition ()                         const override;

    void setBackend(BackendMarble* const pMarbleBackend);

private:

    QPointer<BackendMarble> marbleBackend;

private:

    Q_DISABLE_COPY(BackendMarbleLayer)
};

} // namespace Digikam
