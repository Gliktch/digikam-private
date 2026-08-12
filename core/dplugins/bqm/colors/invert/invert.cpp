/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-16
 * Description : Invert colors batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "invert.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "invertfilter.h"

namespace DigikamBqmInvertPlugin
{

Invert::Invert(QObject* const parent)
    : BatchTool(QLatin1String("Invert"), ColorTool, parent)
{
}

BatchToolSettings Invert::defaultSettings()
{
    return BatchToolSettings();
}

BatchTool* Invert::clone(QObject* const parent) const
{
    return new Invert(parent);
}

bool Invert::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    InvertFilter inv(&image(), nullptr);
    applyFilter(&inv);

    return (savefromDImg());
}

} // namespace DigikamBqmInvertPlugin

#include "moc_invert.cpp"
