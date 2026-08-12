/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items to web services.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "wssettingspage.h"

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

#include "wswizard.h"
#include "dfileselector.h"
#include "filesaveconflictbox.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace DigikamGenericUnifiedPlugin
{

class Q_DECL_HIDDEN WSSettingsPage::Private
{
public:

    explicit Private(QWizard* const dialog)
    {
        wizard = dynamic_cast<WSWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
            iface    = wizard->iface();
        }
    }

    QLabel*         labelImagesResize       = nullptr;
    QLabel*         labelImagesFormat       = nullptr;
    QLabel*         labelImageCompression   = nullptr;

    QComboBox*      imagesFormat            = nullptr;

    QCheckBox*      changeImagesProp        = nullptr;
    QCheckBox*      removeMetadata          = nullptr;

    QSpinBox*       imageCompression        = nullptr;
    QSpinBox*       imagesResize            = nullptr;

    WSWizard*       wizard                  = nullptr;
    DInfoInterface* iface                   = nullptr;
    WSSettings*     settings                = nullptr;
};

WSSettingsPage::WSSettingsPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private(dialog))
{
    QWidget* const main = new QWidget(this);

    //---------------------------------------------

    d->changeImagesProp  = new QCheckBox(i18n("Adjust image properties"), main);
    d->changeImagesProp->setChecked(true);
    d->changeImagesProp->setWhatsThis(i18n("If you enable this option, "
                                           "all images to be sent can be "
                                           "resized and recompressed."));

    QGroupBox* const groupBox = new QGroupBox(i18n("Image Properties"), main);
    QGridLayout* const grid2  = new QGridLayout(groupBox);

    //---------------------------------------------

    d->imagesResize = new QSpinBox(groupBox);
    d->imagesResize->setRange(300, 4000);
    d->imagesResize->setSingleStep(1);
    d->imagesResize->setValue(1024);
    d->imagesResize->setSuffix(i18n(" px"));
    d->imagesResize->setWhatsThis(i18n("Select the length of the images that are to be sent. "
                                       "The aspect ratio is preserved."));

    d->labelImagesResize = new QLabel(i18n("Image Length:"), groupBox);
    d->labelImagesResize->setBuddy(d->imagesResize);

    //---------------------------------------------

    d->labelImagesFormat = new QLabel(groupBox);
    d->labelImagesFormat->setWordWrap(false);
    d->labelImagesFormat->setText(i18n("Image Format:"));

    d->imagesFormat  = new QComboBox(groupBox);
    d->imagesFormat->setEditable(false);
    d->imagesFormat->setWhatsThis(i18n("Select your preferred format to convert image."));

    QMap<WSSettings::ImageFormat, QString> map2                = WSSettings::imageFormatNames();
    QMap<WSSettings::ImageFormat, QString>::const_iterator it2 = map2.constBegin();

    while (it2 != map2.constEnd())
    {
        d->imagesFormat->addItem(it2.value(), (int)it2.key());
        ++it2;
    }

    d->labelImagesFormat->setBuddy(d->imagesFormat);

    // --------------------

    d->imageCompression = new QSpinBox(groupBox);
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

    d->labelImageCompression = new QLabel(i18n("Image quality:"), this);
    d->labelImageCompression->setBuddy(d->imageCompression);

    // --------------------

    d->removeMetadata = new QCheckBox(i18n("Remove all metadata"), main);
    d->removeMetadata->setWhatsThis(i18n("If you enable this option, all metadata "
                                         "as Exif, Iptc, and Xmp will be removed."));

    // --------------------

    grid2->addWidget(d->labelImagesResize,     0, 0, 1, 1);
    grid2->addWidget(d->imagesResize,          0, 1, 1, 2);
    grid2->addWidget(d->labelImagesFormat,     1, 0, 1, 1);
    grid2->addWidget(d->imagesFormat,          1, 1, 1, 2);
    grid2->addWidget(d->labelImageCompression, 2, 0, 1, 1);
    grid2->addWidget(d->imageCompression,      2, 1, 1, 2);
    grid2->addWidget(d->removeMetadata,        3, 0, 1, 2);
    grid2->setRowStretch(4, 10);
    grid2->setColumnStretch(2, 10);
    grid2->setSpacing(layoutSpacing());

    grid2->setAlignment(Qt::AlignTop);

    // --------------------

    QGridLayout* const grid = new QGridLayout(main);
    grid->setSpacing(layoutSpacing());

    grid->addWidget(d->changeImagesProp,    0, 0, 1, 4);
    grid->addWidget(groupBox,               1, 0, 1, 4);
    grid->setRowStretch(2, 10);
    grid->setColumnStretch(3, 10);

    setPageWidget(main);

    /*
     * We should use another icon here, because it's not for mail attachment.
     * Therefore, I comment this line.
     *
     * setLeftBottomPix(QIcon::fromTheme(QLatin1String("mail-attachment")));
     */

    connect(d->imagesFormat, SIGNAL(activated(int)),
            this, SLOT(slotImagesFormatChanged(int)));

    connect(d->changeImagesProp, SIGNAL(toggled(bool)),
            groupBox, SLOT(setEnabled(bool)));
}

WSSettingsPage::~WSSettingsPage()
{
    delete d;
}

void WSSettingsPage::slotImagesFormatChanged(int i)
{
    if (i == WSSettings::JPEG)
    {
        d->imageCompression->setEnabled(true);
    }
    else
    {
        d->imageCompression->setEnabled(false);
    }
}

void WSSettingsPage::initializePage()
{
    d->imagesResize->setValue(d->settings->imageSize);
    d->imagesFormat->setCurrentIndex((int)d->settings->imageFormat);

    d->changeImagesProp->setChecked(d->settings->imagesChangeProp);

    d->imageCompression->setValue(d->settings->imageCompression);
    d->removeMetadata->setChecked(d->settings->removeMetadata);

    slotImagesFormatChanged(d->imagesFormat->currentIndex());
}

bool WSSettingsPage::validatePage()
{
    d->settings->imageSize         = d->imagesResize->value();
    d->settings->imageFormat       = WSSettings::ImageFormat(d->imagesFormat->currentIndex());

    d->settings->imagesChangeProp  = d->changeImagesProp->isChecked();
    d->settings->removeMetadata    = d->removeMetadata->isChecked();

    d->settings->imageCompression  = d->imageCompression->value();

    return true;
}

} // namespace DigikamGenericUnifiedPlugin

#include "moc_wssettingspage.cpp"
