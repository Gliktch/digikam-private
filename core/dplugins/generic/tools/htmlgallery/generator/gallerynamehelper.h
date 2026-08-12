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

// Qt includes

#include <QStringList>

namespace DigikamGenericHtmlGalleryPlugin
{

/**
 * This helper class is used to make sure we use unique filenames
 */
class GalleryNameHelper
{
public:

    GalleryNameHelper()  = default;
    ~GalleryNameHelper() = default;

    QString makeNameUnique(const QString& name);

private:

    QStringList m_list;
};

} // namespace DigikamGenericHtmlGalleryPlugin
