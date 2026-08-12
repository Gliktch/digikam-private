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

#include <QStringList>

// Local includes

#include "PluginInterface.h"
#include "LayerInterface.h"

namespace Marble
{

class RenderPlugin;

/**
 * @short The class that specifies the Marble layer interface of a plugin.
 *
 */

class RenderPluginInterface: public PluginInterface, public LayerInterface
{
public:

    ~RenderPluginInterface() override;

    virtual void initialize() = 0;

    virtual bool isInitialized() const = 0;

    /**
     * @brief Returns the name(s) of the backend that the plugin can render
     *
     * This method should return the name of the backend that the plugin
     * can render. The string has to be the same one that is given
     * for the attribute in the layer element of the DGML file that
     * backend is able to process.
     * Examples to replace available default backends would be "vector" or
     * "texture". To provide a completely new kind of functionality please
     * choose your own unique string.
     */
    virtual QStringList backendTypes() const = 0;

    /**
     * @brief Return how the plugin settings should be used.
     *
     * FIXME: Document this
     *
     * Possible Values:
     * "ALWAYS"            -- the plugin renders at the preferred position
     *                        no matter what got specified in the DGML file.
     * "SPECIFIED"         -- renders only in case it got specified in the
     *                        DGML file.
     * "SPECIFIED_ALWAYS"  -- In case it got specified in the DGML file
     *                        render according to the specification
     *                        otherwise choose the preferred position
     */
    virtual QString renderPolicy() const = 0;
};

} // namespace Marble

Q_DECLARE_INTERFACE(Marble::RenderPluginInterface, "org.kde.Marble.RenderPluginInterface/1.09")
