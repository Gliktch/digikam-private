/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-07-06
 * Description : Autodetect outlook binary program
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "outlookbinary.h"

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

OutlookBinary::OutlookBinary(QObject* const)
    : DBinaryIface(
                   QLatin1String("outlook"),
                   QLatin1String("Outlook"),
                   QLatin1String("https://www.microsoft.com/"),
                   QLatin1String("SendByMail"),
                   QStringList(),
                   i18n("Outlook Mail Client.")
                  )
{
    setup();
}

} // namespace Digikam

#include "moc_outlookbinary.cpp"
