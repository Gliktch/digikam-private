/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-11
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "captionthemeparameter.h"

// Local includes

#include "dtextedit.h"

using namespace Digikam;

namespace DigikamGenericHtmlGalleryPlugin
{

QWidget* CaptionThemeParameter::createWidget(QWidget* parent, const QString& value) const
{
    DTextEdit* const edit = new DTextEdit(parent);
    edit->setLinesVisible(4);
    edit->setText(value);

    return edit;
}

QString CaptionThemeParameter::valueFromWidget(QWidget* widget) const
{
    Q_ASSERT(widget);

    const DTextEdit* const edit = static_cast<DTextEdit*>(widget);

    return edit->text();
}

} // namespace DigikamGenericHtmlGalleryPlugin
