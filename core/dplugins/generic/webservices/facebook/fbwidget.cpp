/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-26
 * Description : a tool to export items to Facebook web service
 *
 * SPDX-FileCopyrightText: 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fbwidget.h"

// Qt includes

#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QApplication>
#include <QStyle>

// Local includes

#include "ditemslist.h"
#include "dprogresswdg.h"

namespace DigikamGenericFaceBookPlugin
{

FbWidget::FbWidget(QWidget* const parent,
                   DInfoInterface* const iface,
                   const QString& toolName)
    : WSSettingsWidget(parent, iface, toolName)
{
    getUploadBox()->hide();
    getSizeBox()->hide();

    connect(getReloadBtn(), SIGNAL(clicked()),
            this, SLOT(slotReloadAlbumsRequest()));
}

FbWidget::~FbWidget()
{
}

void FbWidget::updateLabels(const QString& name, const QString& url)
{
    QString web(QLatin1String("https://www.facebook.com"));

    if (!url.isEmpty())
        web = url;

    getHeaderLbl()->setText(QString::fromLatin1(
        "<b><h2><a href='%1'>"
        "<font color=\"#3B5998\">Facebook</font>"
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

void FbWidget::slotReloadAlbumsRequest()
{
    Q_EMIT reloadAlbums(0);
}

} // namespace DigikamGenericFaceBookPlugin

#include "moc_fbwidget.cpp"
