/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect balsa binary program
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "balsabinary.h"

// KDE includes

#include <klocalizedstring.h>

namespace DigikamGenericSendByMailPlugin
{

BalsaBinary::BalsaBinary(QObject* const)
    : DBinaryIface(
                   QLatin1String("balsa"),
                   QLatin1String("Balsa"),
                   QLatin1String("https://pawsa.fedorapeople.org/balsa/"),
                   QLatin1String("SendByMail"),
                   QStringList(QLatin1String("-v")),
                   i18n("Gnome Mail Client.")
                  )
{
    setup();
}

} // namespace DigikamGenericSendByMailPlugin

#include "moc_balsabinary.cpp"
