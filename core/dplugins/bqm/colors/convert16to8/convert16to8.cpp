/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-16
 * Description : 16 to 8 bits color depth converter batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "convert16to8.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "dimgbuiltinfilter.h"

namespace DigikamBqmConvert16To8Plugin
{

Convert16to8::Convert16to8(QObject* const parent)
    : BatchTool(QLatin1String("Convert16to8"), ColorTool, parent)
{
    setToolTitle(i18n("Convert to 8 bits"));
    setToolDescription(i18n("Convert color depth from 16 to 8 bits."));
    setToolIconName(QLatin1String("depth16to8"));
}

BatchToolSettings Convert16to8::defaultSettings()
{
    return BatchToolSettings();
}

BatchTool* Convert16to8::clone(QObject* const parent) const
{
    return new Convert16to8(parent);
}

bool Convert16to8::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    DImgBuiltinFilter filter(DImgBuiltinFilter::ConvertTo8Bit);
    applyFilter(&filter);

    return (savefromDImg());
}

} // namespace DigikamBqmConvert16To8Plugin

#include "moc_convert16to8.cpp"
