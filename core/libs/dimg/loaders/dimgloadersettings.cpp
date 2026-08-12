/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-03-30
 * Description : abstract class to host DImg loader settings.
 *
 * SPDX-FileCopyrightText: 2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgloadersettings.h"

namespace Digikam
{

DImgLoaderSettings::DImgLoaderSettings(QWidget* const parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
}

DImgLoaderSettings::~DImgLoaderSettings()
{
}

QStringList DImgLoaderSettings::parameters() const
{
    return (settings().keys());
}

} // namespace Digikam

#include "moc_dimgloadersettings.cpp"
