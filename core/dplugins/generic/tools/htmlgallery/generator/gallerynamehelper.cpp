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

#include "gallerynamehelper.h"

// Qt includes

#include <QMutexLocker>

namespace DigikamGenericHtmlGalleryPlugin
{

static QMutex s_nameHelperMutex;

QString GalleryNameHelper::makeNameUnique(const QString& name)
{
    QString uname    = name;
    QString nameBase = name;
    int count        = 2;

    QMutexLocker locker(&s_nameHelperMutex);

    while (m_list.indexOf(uname) != -1)
    {
        uname = nameBase + QString::number(count);
        ++count;
    };

    m_list.append(uname);

    return uname;
}

} // namespace DigikamGenericHtmlGalleryPlugin
