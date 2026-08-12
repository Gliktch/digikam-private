/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : Autodetect autooptimiser binary program and version
 *
 * SPDX-FileCopyrightText: 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "autooptimiserbinary.h"

namespace DigikamGenericPanoramaPlugin
{

AutoOptimiserBinary::AutoOptimiserBinary(QObject* const)
    : DBinaryIface(QLatin1String("autooptimiser"),
                   QLatin1String("2010.4"),
                   QLatin1String("autooptimiser version "),
                   1,
                   QLatin1String("Hugin"),
                   QLatin1String("https://hugin.sourceforge.net/download/"),
                   QLatin1String("Panorama"),
                   QStringList(QLatin1String("-h")))
{
    setup();
}

} // namespace DigikamGenericPanoramaPlugin

#include "moc_autooptimiserbinary.cpp"
