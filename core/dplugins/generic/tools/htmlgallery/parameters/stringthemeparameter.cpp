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

#include "stringthemeparameter.h"

// Qt includes

#include <QLineEdit>

namespace DigikamGenericHtmlGalleryPlugin
{

QWidget* StringThemeParameter::createWidget(QWidget* parent, const QString& value) const
{
    QLineEdit* const edit = new QLineEdit(parent);
    edit->setText(value);

    return edit;
}

QString StringThemeParameter::valueFromWidget(QWidget* widget) const
{
    Q_ASSERT(widget);

    QLineEdit* const edit = static_cast<QLineEdit*>(widget);

    return edit->text();
}

} // namespace DigikamGenericHtmlGalleryPlugin
