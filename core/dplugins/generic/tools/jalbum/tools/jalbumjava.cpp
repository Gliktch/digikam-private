/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect Java binary program
 *
 * SPDX-FileCopyrightText: 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "jalbumjava.h"

// KDE includes

#include <klocalizedstring.h>

namespace DigikamGenericJAlbumPlugin
{

JalbumJava::JalbumJava(QObject* const)
    : DBinaryIface(
                   QLatin1String("java"),
                   QLatin1String("Java"),
                   QLatin1String("https://www.java.com/"),
                   QLatin1String("jAlbum Export"),
                   QStringList(QLatin1String("-version")),
                   i18n("jAlbum Gallery Generator.")
                  )
{
    setup();
}

} // namespace DigikamGenericJAlbumPlugin

#include "moc_jalbumjava.cpp"
