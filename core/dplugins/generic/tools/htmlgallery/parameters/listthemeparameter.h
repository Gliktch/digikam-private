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

#pragma once

// Local includes

#include "abstractthemeparameter.h"

namespace DigikamGenericHtmlGalleryPlugin
{

/**
 * A theme parameter to select a value in a list
 */
class ListThemeParameter : public AbstractThemeParameter
{
public:

    ListThemeParameter();
    ~ListThemeParameter()                                                          override;

    void     init(const QByteArray& internalName, const KConfigGroup* configGroup) override;
    QWidget* createWidget(QWidget* parent, const QString& value)             const override;
    QString  valueFromWidget(QWidget*)                                       const override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericHtmlGalleryPlugin
