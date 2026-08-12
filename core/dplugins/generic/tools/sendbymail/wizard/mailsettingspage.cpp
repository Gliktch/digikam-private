/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items by email.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mailsettingspage.h"

// Qt includes

#include <QIcon>
#include <QLabel>
#include <QUrl>
#include <QWidget>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QGridLayout>
#include <QGroupBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "mailwizard.h"
#include "dfileselector.h"
#include "filesaveconflictbox.h"
#include "digikam_debug.h"

namespace DigikamGenericSendByMailPlugin
{

class Q_DECL_HIDDEN MailSettingsPage::Private
{
public:

    explicit Private(QWizard* const dialog)
    {
        wizard = dynamic_cast<MailWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
            iface    = wizard->iface();
        }
    }

    QComboBox*      mailAgentName       = nullptr;
    QComboBox*      imageFormat         = nullptr;

    QCheckBox*      changeImagesProp    = nullptr;
    QCheckBox*      addFileProperties   = nullptr;
    QCheckBox*      removeMetadata      = nullptr;

    QSpinBox*       imageCompression    = nullptr;
    QSpinBox*       attachmentlimit     = nullptr;
    QSpinBox*       imageResize         = nullptr;

    MailWizard*     wizard              = nullptr;
    DInfoInterface* iface               = nullptr;
    MailSettings*   settings            = nullptr;
};

MailSettingsPage::MailSettingsPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private(dialog))
{
    QWidget* const main = new QWidget(this);
    const int spacing   = layoutSpacing();

    // --------------------

    QLabel* const labelMailAgent = new QLabel(main);
    labelMailAgent->setWordWrap(false);
    labelMailAgent->setText(i18n("Mail program:"));

    d->mailAgentName             = new QComboBox(main);
    d->mailAgentName->setEditable(false);
    d->mailAgentName->setWhatsThis(i18n("Select your preferred external mail client program here."));

    QMap<MailSettings::MailClient, QString> map                = MailSettings::mailClientNames();
    QMap<MailSettings::MailClient, QString>::const_iterator it = map.constBegin();

    while (it != map.constEnd())
    {
        d->mailAgentName->insertItem((int)it.key(), it.value(), (int)it.key());
        ++it;
    }

    labelMailAgent->setBuddy(d->mailAgentName);

    //---------------------------------------------

    d->addFileProperties = new QCheckBox(i18n("Attach a file with items properties"), main);
    d->addFileProperties->setWhatsThis(i18n("If you enable this option, all item properties "
                                            "as Comments, Rating, or Tags, will be added as "
                                            "an attached file."));

    // --------------------------------------------

    d->attachmentlimit                 = new QSpinBox(main);
    d->attachmentlimit->setRange(1, 50);
    d->attachmentlimit->setSingleStep(1);
    d->attachmentlimit->setValue(17);
    d->attachmentlimit->setSuffix(i18n(" MB"));

    QLabel* const labelAttachmentLimit = new QLabel(i18n("Maximum email size limit:"), main);
    labelAttachmentLimit->setBuddy(d->attachmentlimit);

    //---------------------------------------------

    d->changeImagesProp       = new QCheckBox(i18n("Adjust image properties"), main);
    d->changeImagesProp->setChecked(true);
    d->changeImagesProp->setWhatsThis(i18n("If you enable this option, "
                                           "all images to be sent can be "
                                           "resized and recompressed."));

    QGroupBox* const groupBox = new QGroupBox(i18n("Image Properties"), main);
    QGridLayout* const grid2  = new QGridLayout(groupBox);

    //---------------------------------------------

    d->imageResize                 = new QSpinBox(groupBox);
    d->imageResize->setRange(300, 4000);
    d->imageResize->setSingleStep(1);
    d->imageResize->setValue(1024);
    d->imageResize->setSuffix(i18n(" px"));
    d->imageResize->setWhatsThis(i18n("Select the length of the images that are to be sent. "
                                       "The aspect ratio is preserved."));

    QLabel* const labelImageResize = new QLabel(i18n("Image Length:"), groupBox);
    labelImageResize->setBuddy(d->imageResize);

    //---------------------------------------------

    QLabel* const labelImageFormat = new QLabel(groupBox);
    labelImageFormat->setWordWrap(false);
    labelImageFormat->setText(i18n("Image Format:"));

    d->imageFormat                 = new QComboBox(groupBox);
    d->imageFormat->setEditable(false);
    d->imageFormat->setWhatsThis(i18n("Select your preferred format to convert image."));

    QMap<MailSettings::ImageFormat, QString> map2                = MailSettings::imageFormatNames();
    QMap<MailSettings::ImageFormat, QString>::const_iterator it2 = map2.constBegin();

    while (it2 != map2.constEnd())
    {
        d->imageFormat->addItem(it2.value(), (int)it2.key());
        ++it2;
    }

    labelImageFormat->setBuddy(d->imageFormat);

    // --------------------

    d->imageCompression                 = new QSpinBox(groupBox);
    d->imageCompression->setRange(1, 100);
    d->imageCompression->setSingleStep(1);
    d->imageCompression->setValue(75);
    QString whatsThis = i18n("<p>The new compression value of JPEG images to be sent:</p>");
    whatsThis         = whatsThis + i18n("<p><b>1</b>: very high compression<br>"
                                         "<b>25</b>: high compression<br>"
                                         "<b>50</b>: medium compression<br>"
                                         "<b>75</b>: low compression (default value)<br>"
                                         "<b>100</b>: no compression</p>");

    d->imageCompression->setWhatsThis(whatsThis);

    QLabel* const labelImageCompression = new QLabel(i18n("Image quality:"), this);
    labelImageCompression->setBuddy(d->imageCompression);

    // --------------------

    d->removeMetadata = new QCheckBox(i18n("Remove all metadata"), main);
    d->removeMetadata->setWhatsThis(i18n("If you enable this option, all metadata "
                                         "as Exif, Iptc, and Xmp will be removed."));

    // --------------------

    grid2->addWidget(labelImageResize,      0, 0, 1, 1);
    grid2->addWidget(d->imageResize,        0, 1, 1, 2);
    grid2->addWidget(labelImageFormat,      1, 0, 1, 1);
    grid2->addWidget(d->imageFormat,        1, 1, 1, 2);
    grid2->addWidget(labelImageCompression, 2, 0, 1, 1);
    grid2->addWidget(d->imageCompression,   2, 1, 1, 2);
    grid2->addWidget(d->removeMetadata,     3, 0, 1, 2);
    grid2->setRowStretch(4, 10);
    grid2->setColumnStretch(2, 10);
    grid2->setSpacing(spacing);
    grid2->setAlignment(Qt::AlignTop);

    // --------------------

    QGridLayout* const grid = new QGridLayout(main);
    grid->addWidget(labelMailAgent,       0, 0, 1, 1);
    grid->addWidget(d->mailAgentName,     0, 1, 1, 2);
    grid->addWidget(labelAttachmentLimit, 1, 0, 1, 1);
    grid->addWidget(d->attachmentlimit,   1, 1, 1, 4);
    grid->addWidget(d->addFileProperties, 2, 0, 1, 4);
    grid->addWidget(d->changeImagesProp,  3, 0, 1, 4);
    grid->addWidget(groupBox,             4, 0, 1, 4);
    grid->setRowStretch(5, 10);
    grid->setColumnStretch(3, 10);
    grid->setSpacing(spacing);

    setPageWidget(main);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("mail-attachment")));

    //---------------------------------------------

    connect(d->imageFormat, SIGNAL(activated(int)),
            this, SLOT(slotImageFormatChanged(int)));

    connect(d->changeImagesProp, SIGNAL(toggled(bool)),
            groupBox, SLOT(setEnabled(bool)));
}

MailSettingsPage::~MailSettingsPage()
{
    delete d;
}

void MailSettingsPage::slotImageFormatChanged(int i)
{
    if (i == MailSettings::JPEG)
    {
        d->imageCompression->setEnabled(true);
    }
    else
    {
        d->imageCompression->setEnabled(false);
    }
}

void MailSettingsPage::initializePage()
{
    QMap<MailSettings::MailClient, QString> map                = d->settings->binPaths;
    QMap<MailSettings::MailClient, QString>::const_iterator it = map.constBegin();

    while (it != map.constEnd())
    {
        if (d->settings->binPaths[it.key()].isEmpty())
        {
            d->mailAgentName->setItemData((int)it.key(), false, Qt::UserRole-1);
        }
        else if (it.key() == d->settings->mailProgram)
        {
            d->mailAgentName->setCurrentIndex((int)d->settings->mailProgram);
        }

        ++it;
    }

    d->imageResize->setValue(d->settings->imageSize);
    d->imageFormat->setCurrentIndex((int)d->settings->imageFormat);

    d->changeImagesProp->setChecked(d->settings->imagesChangeProp);

    d->addFileProperties->setChecked(d->iface ? d->settings->addFileProperties : false);
    d->addFileProperties->setEnabled(d->iface);

    d->imageCompression->setValue(d->settings->imageCompression);
    d->attachmentlimit->setValue(d->settings->attLimitInMbytes);
    d->removeMetadata->setChecked(d->settings->removeMetadata);

    slotImageFormatChanged(d->imageFormat->currentIndex());
}

bool MailSettingsPage::validatePage()
{
    d->settings->mailProgram       = MailSettings::MailClient(d->mailAgentName->currentIndex());
    d->settings->imageSize         = d->imageResize->value();
    d->settings->imageFormat       = MailSettings::ImageFormat(d->imageFormat->currentIndex());

    d->settings->imagesChangeProp  = d->changeImagesProp->isChecked();
    d->settings->addFileProperties = d->addFileProperties->isChecked();
    d->settings->removeMetadata    = d->removeMetadata->isChecked();

    d->settings->imageCompression  = d->imageCompression->value();
    d->settings->attLimitInMbytes  = d->attachmentlimit->value();

    return true;
}

} // namespace DigikamGenericSendByMailPlugin

#include "moc_mailsettingspage.cpp"
