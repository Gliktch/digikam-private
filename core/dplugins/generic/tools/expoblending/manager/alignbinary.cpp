/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-23
 * Description : Autodetect align_image_stack binary program and version
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "alignbinary.h"

namespace DigikamGenericExpoBlendingPlugin
{

AlignBinary::AlignBinary()
    : DBinaryIface(QLatin1String("align_image_stack"),
                   QLatin1String("0.8"),
                   QLatin1String("align_image_stack version "),
                   1,
                   QLatin1String("Hugin"),
                   QLatin1String("http://hugin.sourceforge.net/download/"),     // krazy:exclude=insecurenet
                   QLatin1String("ExpoBlending"),
                   QStringList(QLatin1String("-h"))
                  )
{
    setup();
}

} // namespace DigikamGenericExpoBlendingPlugin

#include "moc_alignbinary.cpp"
