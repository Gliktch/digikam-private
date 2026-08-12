/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : Autodetects cpclean binary program and version
 *
 * SPDX-FileCopyrightText: 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "cpcleanbinary.h"

namespace DigikamGenericPanoramaPlugin
{

CPCleanBinary::CPCleanBinary(QObject* const)
    : DBinaryIface(QLatin1String("cpclean"),
                   QLatin1String("2010.4"),
                   QLatin1String("cpclean version "),
                   1,
                   QLatin1String("Hugin"),
                   QLatin1String("https://hugin.sourceforge.net/download/"),
                   QLatin1String("Panorama"),
                   QStringList(QLatin1String("-h")))
{
    setup();
}

} // namespace DigikamGenericPanoramaPlugin

#include "moc_cpcleanbinary.cpp"
