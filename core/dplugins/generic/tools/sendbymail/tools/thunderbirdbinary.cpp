/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect thunderbird binary program
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "thunderbirdbinary.h"

// KDE includes

#include <klocalizedstring.h>

namespace DigikamGenericSendByMailPlugin
{

ThunderbirdBinary::ThunderbirdBinary(QObject* const)
   : DBinaryIface(
                  QLatin1String("thunderbird"),
                  QLatin1String("Thunderbird"),
                  QLatin1String("https://www.mozilla.org/en-US/thunderbird/"),
                  QLatin1String("SendByMail"),
                  QStringList(QLatin1String("-v")),
                  i18n("Mozilla Foundation Mail Client.")
                 )
{
    setup();
}

} // namespace DigikamGenericSendByMailPlugin

#include "moc_thunderbirdbinary.cpp"
