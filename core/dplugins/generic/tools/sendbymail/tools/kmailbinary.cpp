/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect kmail binary program
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "kmailbinary.h"

// KDE includes

#include <klocalizedstring.h>

using namespace Digikam;

namespace DigikamGenericSendByMailPlugin
{

KmailBinary::KmailBinary(QObject* const)
    : DBinaryIface(
                   QLatin1String("kmail"),
                   QLatin1String("KMail"),
                   QLatin1String("https://kde.org/applications/office/org.kde.kmail2"),
                   QLatin1String("SendByMail"),
                   QStringList(QLatin1String("-v")),
                   i18n("KDE Mail Client.")
                  )
{
    setup();
}

} // namespace DigikamGenericSendByMailPlugin

#include "moc_kmailbinary.cpp"
