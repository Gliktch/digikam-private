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

#pragma once

// Local includes

#include "geocoordinates.h"
#include "geoifacetypes.h"
#include "digikam_export.h"

class QDropEvent;

namespace Digikam
{

class DIGIKAM_EXPORT GeoDragDropHandler : public QObject
{
    Q_OBJECT

public:

    explicit GeoDragDropHandler(QObject* const parent = nullptr);
    ~GeoDragDropHandler() override;

    virtual Qt::DropAction accepts(const QDropEvent* e)                                 = 0;
    virtual bool dropEvent(const QDropEvent* e, const GeoCoordinates& dropCoordinates)  = 0;
    virtual QMimeData* createMimeData(const QList<QPersistentModelIndex>& modelIndices) = 0;
};

} // namespace Digikam
