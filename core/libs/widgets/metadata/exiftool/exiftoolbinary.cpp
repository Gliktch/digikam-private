/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-04-18
 * Description : auto-detect ExifTool program and version.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "exiftoolbinary.h"

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

ExifToolBinary::ExifToolBinary(QObject* const)
    : DBinaryIface(QLatin1String("exiftool"),
                   QLatin1String("10.00"),          // First release for production uses.
                   QLatin1String(),
                   0,
                   QLatin1String("ExifTool"),
                   QLatin1String("https://exiftool.org/"),
                   QString(),
                   QStringList(QLatin1String("-ver")),
                   i18n("ExifTool is a platform-independent command-line application for reading, "
                        "writing and editing meta information in a wide variety of files.")
                  )
{
    setup();
}

} // namespace Digikam

#include "moc_exiftoolbinary.cpp"
