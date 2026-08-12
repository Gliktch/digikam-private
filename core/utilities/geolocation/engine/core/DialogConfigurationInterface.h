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

#include <QObject>

// Local includes

#include "digikam_export.h"

class QDialog;

namespace Marble
{

/**
 * @brief This interface allows a plugin to provide a QWidget-based configuration
 * dialog which is accessible within Marble's own configuration dialog.
 *
 * @note In order for your plugin to provide a configuration dialog, derive your
 * plugin from this interface in addition to any other interfaces and classes.
 *
 * @note Make sure to deploy the @code Q_INTERFACES @endcode macro, which tells
 * Marble that you actually implemented the interface.
 */
class DIGIKAM_EXPORT DialogConfigurationInterface
{
public:

    virtual ~DialogConfigurationInterface();

    /**
     * @brief Returns a pointer to the configuration dialog of the plugin.
     *
     * @return: Pointer to the configuration dialog, which must be non-zero.
     */
    virtual QDialog* configDialog() = 0;
};

} // Namespace Marble

Q_DECLARE_INTERFACE(Marble::DialogConfigurationInterface, "org.kde.Marble.DialogConfigurationInterface/1.0")
