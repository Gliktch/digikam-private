/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

/**
 * This file contains the headers for ViewParameters.
 */

#include "MarbleGlobal.h"

namespace Marble
{

class ViewParamsPrivate;

/**
 * @short A public class that controls the painting of a MarbleWidget
 */

class ViewParams
{
public:

    ViewParams();
    ~ViewParams();

    MapQuality mapQuality(ViewContext viewContext) const;
    MapQuality mapQuality() const;
    void setMapQualityForViewContext(MapQuality quality, ViewContext viewContext);

    ViewContext viewContext() const;
    void setViewContext(ViewContext viewContext);

    bool showAtmosphere() const;
    void setShowAtmosphere(bool);

    bool showClouds() const;
    void setShowClouds(bool const);

private:

    Q_DISABLE_COPY(ViewParams)

private:

    ViewParamsPrivate* const d = nullptr;
};

} // Namespace Marble
