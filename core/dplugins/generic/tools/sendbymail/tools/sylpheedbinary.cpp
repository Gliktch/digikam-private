/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect sylpheed binary program
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "sylpheedbinary.h"

// KDE includes

#include <klocalizedstring.h>

namespace DigikamGenericSendByMailPlugin
{

SylpheedBinary::SylpheedBinary(QObject* const)
    : DBinaryIface(
                   QLatin1String("sylpheed"),
                   QLatin1String("Sylpheed"),
                   QLatin1String("https://sylpheed.sraoss.jp/en/"),
                   QLatin1String("SendByMail"),
                   QStringList(QLatin1String("--version")),
                   i18n("GTK based Mail Client.")
                  )
{
    setup();
}

} // namespace DigikamGenericSendByMailPlugin

#include "moc_sylpheedbinary.cpp"
