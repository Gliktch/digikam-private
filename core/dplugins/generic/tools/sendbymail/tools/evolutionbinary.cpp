/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect evolution binary program
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "evolutionbinary.h"

// KDE includes

#include <klocalizedstring.h>

namespace DigikamGenericSendByMailPlugin
{

EvolutionBinary::EvolutionBinary(QObject* const)
    : DBinaryIface(
                   QLatin1String("evolution"),
                   QLatin1String("Evolution"),
                   QLatin1String("https://wiki.gnome.org/Apps/Evolution"),
                   QLatin1String("SendByMail"),
                   QStringList(QLatin1String("-v")),
                   i18n("Gnome Mail Client.")
                  )
{
    setup();
}

} // namespace DigikamGenericSendByMailPlugin

#include "moc_evolutionbinary.cpp"
