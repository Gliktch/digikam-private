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

#include <QVector>
#include <QString>
#include <QtPlugin>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_export.h"

class QIcon;

namespace Marble
{

struct DIGIKAM_EXPORT PluginAuthor
{
public:

    PluginAuthor()
    {
    }

    PluginAuthor(const QString& name_, const QString& email_, const QString& task_ = i18n("Developer"))
        : name(name_),
          task(task_),
          email(email_)
    {
    }

    QString name;
    QString task;
    QString email;
};

/**
 * @short This class specifies interface of a Marble plugin.
 */

class DIGIKAM_EXPORT PluginInterface
{
public:

    virtual ~PluginInterface();

    /**
     * @brief Returns the user-visible name of the plugin.
     *
     * The user-visible name should be context free, i.e. the name should
     * provide enough information as to what the plugin is about in the context
     * of Marble.
     *
     * Example: "Starry Sky Background"
     */
    virtual QString name() const = 0;

    /**
     * @brief Returns the unique name of the plugin.
     *
     * Examples: "starrysky", "openrouteservice"
     */
    virtual QString nameId() const = 0;

    virtual QString version() const = 0;

    /**
     * @brief Returns a user description of the plugin.
     */
    virtual QString description() const = 0;

    /**
     * @brief Returns an icon for the plugin.
     */
    virtual QIcon icon() const = 0;

    virtual QString copyrightYears() const = 0;

    /**
     * @since 0.26.0
     */
    virtual QVector<PluginAuthor> pluginAuthors() const = 0;

    /**
     * @brief Returns about text (credits) for external data the plugin uses.
     *
     * The default implementation returns the empty string. Please override
     * this method to give credits for all data from 3rd-partys.
     */
    virtual QString aboutDataText() const;
};

} // namespace Marble

Q_DECLARE_TYPEINFO(Marble::PluginAuthor, Q_MOVABLE_TYPE);

Q_DECLARE_INTERFACE(Marble::PluginInterface, "org.kde.Marble.PluginInterface/1.1")
