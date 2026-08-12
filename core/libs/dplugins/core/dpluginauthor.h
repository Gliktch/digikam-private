/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : author data container for external plugin
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPluginAuthor
{
public:

    DPluginAuthor(const QString& _nane,
                  const QString& _email,
                  const QString& _year);

    DPluginAuthor(const QString& _name,
                  const QString& _email,
                  const QString& _year,
                  const QString& _role);

    ~DPluginAuthor() = default;

    /**
     * Return author details as string.
     * For debug purpose only.
     */
    QString toString() const;

public:

    QString name;    ///< Author name and surname
    QString email;   ///< Email anti-spammed
    QString years;   ///< Copyrights years
    QString roles;   ///< Author roles, as "Developer", "Designer", "Translator", etc.
};

} // namespace Digikam
