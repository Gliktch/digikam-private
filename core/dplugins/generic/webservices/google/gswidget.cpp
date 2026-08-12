/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export items to Google web services
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2013-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gswidget.h"

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
#include <QPushButton>
#include <QApplication>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>

namespace DigikamGenericGoogleServicesPlugin
{

GSWidget::GSWidget(QWidget* const parent,
                   DInfoInterface* const iface,
                   const GoogleService& service,
                   const QString& serviceName)
    : WSSettingsWidget(parent, iface, serviceName),
      m_service       (service)
{
    QGroupBox* const leafBox = new QGroupBox(QLatin1String(""), getSettingsBox());

    if (m_service == GoogleService::GPhotoExport)
    {
        QGridLayout* leafLayout   = new QGridLayout(leafBox);
        m_tagsBGrp                = new QButtonGroup(leafBox);
        QSpacerItem* const spacer = new QSpacerItem(1, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
        QLabel* const tagsLbl     = new QLabel(i18n("Tag path behavior :"), leafBox);

        QRadioButton* const leafTagsBtn     = new QRadioButton(i18n("Leaf tags only"), leafBox);
        leafTagsBtn->setWhatsThis(i18n("Export only the leaf tags of tag hierarchies"));
        QRadioButton* const splitTagsBtn    = new QRadioButton(i18n("Split tags"), leafBox);
        splitTagsBtn->setWhatsThis(i18n("Export the leaf tag and all ancestors as single tags."));
        QRadioButton* const combinedTagsBtn = new QRadioButton(i18n("Combined String"), leafBox);
        combinedTagsBtn->setWhatsThis(i18n("Build a combined tag string."));

        m_tagsBGrp->addButton(leafTagsBtn,     GPTagLeaf);
        m_tagsBGrp->addButton(splitTagsBtn,    GPTagSplit);
        m_tagsBGrp->addButton(combinedTagsBtn, GPTagCombined);

        leafLayout->addItem(spacer,            0, 1, 1, 1);
        leafLayout->addWidget(tagsLbl,         1, 1, 1, 1);
        leafLayout->addWidget(leafTagsBtn,     2, 1, 1, 1);
        leafLayout->addWidget(splitTagsBtn,    3, 1, 1, 1);
        leafLayout->addWidget(combinedTagsBtn, 4, 1, 1, 1);

        addWidgetToSettingsBox(leafBox);
    }

    switch (m_service)
    {
        case GoogleService::GPhotoImport:
        {
            getNewAlbmBtn()->hide();
            getOptionsBox()->hide();
            imagesList()->hide();
            leafBox->hide();
            getSizeBox()->hide(); // (Trung) Hide this option temporary, until factorization
            break;
        }

        case GoogleService::GDrive:
        {
            getOriginalCheckBox()->show();
            getUploadBox()->hide();
            getSizeBox()->hide();
            leafBox->hide();
            break;
        }

        default:
        {
            getOriginalCheckBox()->show();
            getUploadBox()->hide();
            getSizeBox()->hide();
            leafBox->hide();    // Google has removed this function in the current API V3.
            getPhotoIdCheckBox()->show();
            break;
        }
    }
}

void GSWidget::updateLabels(const QString& name, const QString& url)
{
    switch (m_service)
    {
        case GoogleService::GDrive:
        {
            QString web(QLatin1String("https://drive.google.com"));
            getHeaderLbl()->setText(QString::fromLatin1(
                "<b><h2><a href='%1'>"
                "<font color=\"#9ACD32\">Google Drive</font>"
                "</a></h2></b>").arg(web));
            break;
        }

        default:
        {
            getHeaderLbl()->setText(QString::fromLatin1(
                "<b><h2><a href='https://photos.google.com/%1'>"
                "<font color=\"#9ACD32\">Google Photos</font>"
                "</a></h2></b>").arg(url));
            break;
        }
    }

    if (name.isEmpty())
    {
        getUserNameLabel()->clear();
    }
    else
    {
        getUserNameLabel()->setText(QString::fromLatin1("<b>%1</b>").arg(name));
    }
}

} // namespace DigikamGenericGoogleServicesPlugin

#include "moc_gswidget.cpp"
