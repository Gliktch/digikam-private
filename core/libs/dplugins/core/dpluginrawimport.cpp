/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-08
 * Description : RAW Import digiKam plugin definition.
 *
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dpluginrawimport.h"

// Qt includes

#include <QApplication>

// Local includes

#include "digikam_version.h"
#include "digikam_debug.h"

namespace Digikam
{

DPluginRawImport::DPluginRawImport(QObject* const parent)
    : DPlugin(parent)
{
}

QString DPluginRawImport::getRawProgram() const
{
    return QString();
}

} // namespace Digikam

#include "moc_dpluginrawimport.cpp"
