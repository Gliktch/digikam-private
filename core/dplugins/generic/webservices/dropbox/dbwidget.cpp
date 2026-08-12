/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export images to Dropbox web service
 *
 * SPDX-FileCopyrightText: 2013 by Pankaj Kumar <me at panks dot me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbwidget.h"

// Qt includes

#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>

// Local includes

#include "ditemslist.h"

namespace DigikamGenericDropBoxPlugin
{

DBWidget::DBWidget(QWidget* const parent,
                   DInfoInterface* const iface,
                   const QString& toolName)
    : WSSettingsWidget(parent, iface, toolName)
{
    getOriginalCheckBox()->show();
    getUploadBox()->hide();
    getSizeBox()->hide();
}

void DBWidget::updateLabels(const QString& name, const QString& url)
{
    QString web(QLatin1String("https://www.dropbox.com/"));

    if (!url.isEmpty())
        web = url;

    getHeaderLbl()->setText(QString::fromLatin1(
        "<b><h2><a href='%1'>"
        "<font color=\"#9ACD32\">Dropbox</font>"
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

} // namespace DigikamGenericDropBoxPlugin

#include "moc_dbwidget.cpp"
