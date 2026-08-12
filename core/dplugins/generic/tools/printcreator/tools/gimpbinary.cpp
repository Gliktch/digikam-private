/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect gimp binary program
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gimpbinary.h"

// KDE includes

#include <klocalizedstring.h>

namespace DigikamGenericPrintCreatorPlugin
{

GimpBinary::GimpBinary(QObject* const)
    : DBinaryIface(
                   QLatin1String("gimp"),
                   QLatin1String("The Gimp"),
                   QLatin1String("https://www.gimp.org/downloads/"),
                   QLatin1String("PrintCreator"),
                   QStringList(QLatin1String("-v")),
                   i18n("The GNU Image Manipulation Program.")
                  )
{
    setup();
}

} // namespace DigikamGenericPrintCreatorPlugin

#include "moc_gimpbinary.cpp"
