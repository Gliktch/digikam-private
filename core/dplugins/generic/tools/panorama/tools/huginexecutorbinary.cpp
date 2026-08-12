/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-11-02
 * Description : Autodetect hugin_executor binary program and version
 *
 * SPDX-FileCopyrightText: 2015-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "huginexecutorbinary.h"

namespace DigikamGenericPanoramaPlugin
{

HuginExecutorBinary::HuginExecutorBinary(QObject* const)
    : DBinaryIface(QLatin1String("hugin_executor"),
                   QLatin1String("Hugin"),
                   QLatin1String("https://hugin.sourceforge.net/download/"),
                   QLatin1String("Panorama"),
                   QStringList(QLatin1String("-h")))
{
    setup();
}

} // namespace DigikamGenericPanoramaPlugin

#include "moc_huginexecutorbinary.cpp"
