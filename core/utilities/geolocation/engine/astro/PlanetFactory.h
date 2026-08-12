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

// Qt includes

#include <QList>

// Local includes

#include "digikam_export.h"

class QString;

namespace Marble
{

class Planet;

/**
 * @brief The PlanetFactory class provides static methods to construct any planet known to Marble
 */
class DIGIKAM_EXPORT PlanetFactory
{
public:

    /** Provides a list of known planet IDs */
    static QList<QString> planetList();

    /** Creates the planet with the given ID, or one with default values if ID is not among planetList() */
    static Planet construct(const QString& id);

    /** Returns a localized name of the planet with the given ID */
    static QString localizedName(const QString& id);
};

} // Namespace Marble
