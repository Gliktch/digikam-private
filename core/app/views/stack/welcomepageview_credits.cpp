/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-12-20
 * Description : a widget to display a welcome page
 *               on root album.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "welcomepageview_p.h"

namespace Digikam
{

QString WelcomePageView::creditsTabContent()  const
{
    QString creditsTab =
        QString::fromUtf8(
                "<table>"
                  "<tr>"
                    "<td width=\"10%\">%1</td>"
                    "<td><h4>%2</h4></td>"
                  "</tr>"
                  "<tr>"
                    "<td width=\"10%\">%3</td>"
                    "<td><h4>%4</h4></td>"
                  "</tr>"
                  "<tr>"
                    "<td width=\"10%\">%5</td>"
                    "<td><h4>%6</h4></td>"
                  "</tr>"
                  "<tr>"
                    "<td width=\"10%\">%7</td>"
                    "<td><h4>%8</h4></td>"
                  "</tr>"
                  "<tr>"
                    "<td width=\"10%\">%9</td>"
                    "<td><h4>%10</h4></td>"
                  "</tr>"
                "</table>")

        .arg(i18n("Author:"))
        .arg(QString::fromUtf8("Jack Gold"))

        .arg(i18n("Location:"))
        .arg(QString::fromUtf8("<a href=\""
                                    "https://en.wikipedia.org/wiki/Victoria_Bridge,_Brisbane"
                               "\">"
                                    "Brisbane CBD Victoria bridge, Queensland, Australia."
                               "</a>"))

        .arg(i18n("Date:"))
        .arg(QString::fromUtf8("July 2019"))

        .arg(i18n("Camera:"))
        .arg(QString::fromUtf8("Nikon D3100"))

        .arg(i18n("Lens:"))
        .arg(QString::fromUtf8("Sigma 17-50mm F2.8"))
    ;

    return creditsTab;
}

} // namespace Digikam
