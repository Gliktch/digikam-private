/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter (x.com) social network
 *
 * SPDX-FileCopyrightText: 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "twitterwidget.h"

// Qt includes

#include <QLabel>
#include <QGroupBox>

// Local includes

namespace DigikamGenericTwitterPlugin
{

TwWidget::TwWidget(QWidget* const parent,
                   DInfoInterface* const iface,
                   const QString& toolName)
    : WSSettingsWidget(parent, iface, toolName)
{
    getUploadBox()->hide();
    getSizeBox()->hide();

    /* (Feb 2019)
     * Temporarily hide album box because Twitter API has no functionality for albums
     */
    getAlbumBox()->hide();
}

void TwWidget::updateLabels(const QString& name, const QString& url)
{
    QString web(QLatin1String("https://x.com/"));

    if (!url.isEmpty())
    {
        web = url;
    }

    getHeaderLbl()->setText(QString::fromLatin1(
        "<b><h2><a href='%1'>"
        "<font color=\"#9ACD32\">x.com (aka Twitter)</font>"
        "</a></h2></b>").arg(web));

    if (name.isEmpty())
    {
        getUserNameLabel()->clear();
    }
    else
    {
        getUserNameLabel()->setText(QString::fromLatin1("<b>%1</b>").arg(name));
    }
}

} // namespace DigikamGenericTwitterPlugin

#include "moc_twitterwidget.cpp"
