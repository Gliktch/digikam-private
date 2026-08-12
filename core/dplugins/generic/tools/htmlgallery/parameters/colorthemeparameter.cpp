/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "colorthemeparameter.h"

// Local includes

#include "dcolorselector.h"

using namespace Digikam;

namespace DigikamGenericHtmlGalleryPlugin
{

QWidget* ColorThemeParameter::createWidget(QWidget* parent, const QString& value) const
{
    DColorSelector* const button = new DColorSelector(parent);
    QColor color(value);
    button->setColor(color);

    return button;
}

QString ColorThemeParameter::valueFromWidget(QWidget* widget) const
{
    const DColorSelector* const button = static_cast<DColorSelector*>(widget);

    return button->color().name();
}

} // namespace DigikamGenericHtmlGalleryPlugin
