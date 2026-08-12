/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Onedrive web service
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "odwidget.h"

// Qt includes

#include <QLabel>
#include <QGroupBox>

namespace DigikamGenericOneDrivePlugin
{

ODWidget::ODWidget(QWidget* const parent,
                   DInfoInterface* const iface,
                   const QString& toolName)
    : WSSettingsWidget(parent, iface, toolName)
{
    getUploadBox()->hide();
    getSizeBox()->hide();
}

void ODWidget::updateLabels(const QString& name, const QString& url)
{
    QString web(QLatin1String("https://www.onedrive.com/"));

    if (!url.isEmpty())
    {
        web = url;
    }

    getHeaderLbl()->setText(QString::fromLatin1(
        "<b><h2><a href='%1'>"
        "<font color=\"#9ACD32\">OneDrive</font>"
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

} // namespace DigikamGenericOneDrivePlugin

#include "moc_odwidget.cpp"
