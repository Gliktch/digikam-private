/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect claws-mail binary program
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "clawsmailbinary.h"

// KDE includes

#include <klocalizedstring.h>

namespace DigikamGenericSendByMailPlugin
{

ClawsMailBinary::ClawsMailBinary(QObject* const)
    : DBinaryIface(
                   QLatin1String("claws-mail"),
                   QLatin1String("Claws Mail"),
                   QLatin1String("https://www.claws-mail.org/"),
                   QLatin1String("SendByMail"),
                   QStringList(QLatin1String("-v")),
                   i18n("GTK based Mail Client.")
                  )
{
    setup();
}

} // namespace DigikamGenericSendByMailPlugin

#include "moc_clawsmailbinary.cpp"
