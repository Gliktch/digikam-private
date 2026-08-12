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

#include "dpluginauthor.h"

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

DPluginAuthor::DPluginAuthor(const QString& _name,
                             const QString& _email,
                             const QString& _year)
    : name (_name),
      email(_email),
      years(_year),
      roles(i18n("Developer"))
{
}

DPluginAuthor::DPluginAuthor(const QString& _name,
                             const QString& _email,
                             const QString& _year,
                             const QString& _role)
    : name (_name),
      email(_email),
      years(_year),
      roles(_role)
{
}

QString DPluginAuthor::toString() const
{
    return (
            QString::fromLatin1("%1 <%2> %3 [%4]")
            .arg(name)
            .arg(email)
            .arg(years)
            .arg(roles)
           );
}

} // namespace Digikam
