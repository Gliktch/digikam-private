/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-16
 * Description : 8 to 16 bits color depth converter batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "convert8to16.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "dimgbuiltinfilter.h"

namespace DigikamBqmConvert8To16Plugin
{

Convert8to16::Convert8to16(QObject* const parent)
    : BatchTool(QLatin1String("Convert8to16"), ColorTool, parent)
{
}

BatchToolSettings Convert8to16::defaultSettings()
{
    return BatchToolSettings();
}

BatchTool* Convert8to16::clone(QObject* const parent) const
{
    return new Convert8to16(parent);
}

bool Convert8to16::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    DImgBuiltinFilter filter(DImgBuiltinFilter::ConvertTo16Bit);
    applyFilter(&filter);

    return (savefromDImg());
}

} // namespace DigikamBqmConvert8To16Plugin

#include "moc_convert8to16.cpp"
