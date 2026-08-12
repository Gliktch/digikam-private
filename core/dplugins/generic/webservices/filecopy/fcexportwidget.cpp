/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-03-27
 * Description : a tool to export items to a local storage
 *
 * SPDX-FileCopyrightText: 2006-2009 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fcexportwidget.h"

// Qt includes

#include <QApplication>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGridLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dfileselector.h"
#include "wstoolutils.h"
#include "dlayoutbox.h"
#include "fctask.h"
#include "metaenginesettings.h"

namespace DigikamGenericFileCopyPlugin
{

class Q_DECL_HIDDEN FCExportWidget::Private
{
public:

    Private() = default;

public:

    DInfoInterface* iface                   = nullptr;
    DFileSelector*  selector                = nullptr;

    QCheckBox*      sidecars                = nullptr;
    QCheckBox*      writeMetadataToFile     = nullptr;
    QCheckBox*      overwrite               = nullptr;
    QCheckBox*      albumPath               = nullptr;

    QButtonGroup*   targetButtonGroup       = nullptr;
    QRadioButton*   fileCopyButton          = nullptr;
    QRadioButton*   symLinkButton           = nullptr;
    QRadioButton*   relativeButton          = nullptr;

    QUrl            targetUrl;

    QGroupBox*      imageChangeGroupBox     = nullptr;
    QCheckBox*      removeMetadataProp      = nullptr;

    QSpinBox*       imageCompression        = nullptr;
    QSpinBox*       imageResize             = nullptr;
    QComboBox*      imageFormat             = nullptr;
};

FCExportWidget::FCExportWidget(DInfoInterface* const iface, QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    d->iface                    = iface;

    // setup local target selection

    const int spacing           = layoutSpacing();

    QLabel* const locationLabel = new QLabel(this);
    locationLabel->setText(i18n("Target location: "));
    d->selector                 = new DFileSelector(this);
    d->selector->setFileDlgMode(QFileDialog::Directory);
    d->selector->setFileDlgOptions(QFileDialog::ShowDirsOnly);
    d->selector->setFileDlgTitle(i18nc("@title:window", "Target Folder"));
    d->selector->setWhatsThis(i18n("Sets the target address to copy the items to."));

    QLabel* const targetLabel   = new QLabel(i18n("Target file behavior:"), this);
    d->targetButtonGroup        = new QButtonGroup(this);
    d->fileCopyButton           = new QRadioButton(i18n("Copy files"), this);
    d->symLinkButton            = new QRadioButton(i18n("Create symlinks"), this);
    d->relativeButton           = new QRadioButton(i18n("Create relative symlinks"), this);

    d->sidecars                 = new QCheckBox(i18n("Include the sidecar of the items"), this);
    d->overwrite                = new QCheckBox(i18n("Overwrite existing items in the target"), this);
    d->writeMetadataToFile      = new QCheckBox(i18n("Write sidecar metadata to the items"), this);
    d->albumPath                = new QCheckBox(i18n("Use the album path of the items in the target"), this);

    d->albumPath->setDisabled(!d->iface->supportAlbums());

    d->writeMetadataToFile->setDisabled
    (
        MetaEngineSettings::instance()->settings().metadataWritingMode != MetaEngine::WRITE_TO_SIDECAR_ONLY
    );

    d->targetButtonGroup->addButton(d->fileCopyButton, FCContainer::CopyFile);
    d->targetButtonGroup->addButton(d->symLinkButton,  FCContainer::FullSymLink);
    d->targetButtonGroup->addButton(d->relativeButton, FCContainer::RelativeSymLink);
    d->targetButtonGroup->setExclusive(true);
    d->fileCopyButton->setChecked(true);

    //---------------------------------------------

    d->imageChangeGroupBox          = new QGroupBox(i18n("Adjust Image Properties"), this);
    d->imageChangeGroupBox->setCheckable(true);
    d->imageChangeGroupBox->setChecked(false);
    d->imageChangeGroupBox->setWhatsThis(i18n("If you enable this option, "
                                              "all images to be sent can be "
                                              "resized and recompressed."));

    d->imageResize                  = new QSpinBox(d->imageChangeGroupBox);
    d->imageResize->setRange(300, 6000);
    d->imageResize->setSingleStep(1);
    d->imageResize->setValue(1024);
    d->imageResize->setSuffix(i18n(" px"));
    d->imageResize->setWhatsThis(i18n("Select the length of the images that are to be sent. "
                                       "The aspect ratio is preserved."));
    d->imageChangeGroupBox->setEnabled(false);

    QLabel* const  labelImageResize = new QLabel(i18n("Image Length:"), d->imageChangeGroupBox);
    labelImageResize->setBuddy(d->imageResize);

    //---------------------------------------------

    QLabel* const labelImageFormat = new QLabel(d->imageChangeGroupBox);
    labelImageFormat->setWordWrap(false);
    labelImageFormat->setText(i18n("Image Format:"));

    d->imageFormat                 = new QComboBox(d->imageChangeGroupBox);
    d->imageFormat->setEditable(false);
    d->imageFormat->setWhatsThis(i18n("Select your preferred format to convert image."));
    d->imageFormat->addItem(i18nc("Image format: JPEG", "Jpeg"), FCContainer::JPEG);
    d->imageFormat->addItem(i18nc("Image format: PNG",  "Png"),  FCContainer::PNG);
    labelImageFormat->setBuddy(d->imageFormat);

    //---------------------------------------------

    d->imageCompression                 = new QSpinBox(d->imageChangeGroupBox);
    d->imageCompression->setRange(1, 100);
    d->imageCompression->setSingleStep(1);
    d->imageCompression->setValue(75);
    d->imageCompression->setWhatsThis(i18n("<p>The new compression value of JPEG images to be sent:</p>"
                                           "<p><b>1</b>: very high compression<br>"
                                           "<b>25</b>: high compression<br>"
                                           "<b>50</b>: medium compression<br>"
                                           "<b>75</b>: low compression (default value)<br>"
                                           "<b>100</b>: no compression</p>"));

    QLabel* const labelImageCompression = new QLabel(i18n("Image quality:"), d->imageChangeGroupBox);
    labelImageCompression->setBuddy(d->imageCompression);

    //---------------------------------------------

    d->removeMetadataProp = new QCheckBox(i18n("Remove all metadata"), d->imageChangeGroupBox);
    d->removeMetadataProp->setWhatsThis(i18n("If you enable this option, all metadata "
                                             "as Exif, Iptc, and Xmp will be removed."));

    //---------------------------------------------

    QGridLayout* const grid2 = new QGridLayout(d->imageChangeGroupBox);
    grid2->addWidget(labelImageResize,      0, 0, 1, 1);
    grid2->addWidget(d->imageResize,        0, 1, 1, 2);
    grid2->addWidget(labelImageFormat,      1, 0, 1, 1);
    grid2->addWidget(d->imageFormat,        1, 1, 1, 2);
    grid2->addWidget(labelImageCompression, 2, 0, 1, 1);
    grid2->addWidget(d->imageCompression,   2, 1, 1, 2);
    grid2->addWidget(d->removeMetadataProp, 3, 0, 1, 2);
    grid2->setColumnStretch(2, 10);
    grid2->setSpacing(spacing);
    grid2->setAlignment(Qt::AlignTop);

    //---------------------------------------------

    // Layout Widget

    QGridLayout* const grid = new QGridLayout(this);

    grid->addWidget(locationLabel,           0, 0, 1, 1);
    grid->addWidget(d->selector,             1, 0, 1, 1);
    grid->addWidget(targetLabel,             2, 0, 1, 1);
    grid->addWidget(d->fileCopyButton,       3, 0, 1, 1);
    grid->addWidget(d->symLinkButton,        4, 0, 1, 1);
    grid->addWidget(d->relativeButton,       5, 0, 1, 1);
    grid->addWidget(d->sidecars,             6, 0, 1, 1);
    grid->addWidget(d->overwrite,            7, 0, 1, 1);
    grid->addWidget(d->writeMetadataToFile,  8, 0, 1, 1);
    grid->addWidget(d->albumPath,            9, 0, 1, 1);
    grid->addWidget(d->imageChangeGroupBox, 10, 0, 1, 1);
    grid->setRowStretch(11, 10);
    grid->setSpacing(spacing);
    grid->setContentsMargins(QMargins());

    // ------------------------------------------------------------------------

    connect(d->selector->lineEdit(), SIGNAL(textEdited(QString)),
            this, SLOT(slotLabelUrlChanged()));

    connect(d->selector, SIGNAL(signalUrlSelected(QUrl)),
            this, SLOT(slotLabelUrlChanged()));

    connect(d->fileCopyButton, SIGNAL(toggled(bool)),
            this, SLOT(slotFileCopyButtonChanged(bool)));
}

FCExportWidget::~FCExportWidget()
{
    delete d;
}

QUrl FCExportWidget::targetUrl() const
{
    return d->targetUrl;
}

FCContainer FCExportWidget::getSettings() const
{
    FCContainer settings;

    settings.iface                 = d->iface;
    settings.destUrl               = d->targetUrl;
    settings.behavior              = d->targetButtonGroup->checkedId();
    settings.imageFormat           = d->imageFormat->currentIndex();
    settings.imageResize           = d->imageResize->value();
    settings.imageCompression      = d->imageCompression->value();
    settings.sidecars              = d->sidecars->isChecked();
    settings.writeMetadataToFile   = d->writeMetadataToFile->isChecked();
    settings.overwrite             = d->overwrite->isChecked();
    settings.albumPath             = d->albumPath->isChecked();
    settings.removeMetadata        = d->removeMetadataProp->isChecked();
    settings.changeImageProperties = d->imageChangeGroupBox->isChecked();

    return settings;
}

void FCExportWidget::setSettings(const FCContainer& settings)
{
    d->targetUrl                  = settings.destUrl;
    d->selector->setFileDlgPath(d->targetUrl.toLocalFile());
    QAbstractButton* const button = d->targetButtonGroup->button(settings.behavior);

    if (button)
    {
        button->setChecked(true);
    }

    d->imageFormat->setCurrentIndex(settings.imageFormat);
    d->imageResize->setValue(settings.imageResize);
    d->imageCompression->setValue(settings.imageCompression);
    d->sidecars->setChecked(settings.sidecars);
    d->writeMetadataToFile->setChecked(settings.writeMetadataToFile);
    d->overwrite->setChecked(settings.overwrite);
    d->albumPath->setChecked(settings.albumPath);
    d->removeMetadataProp->setChecked(settings.removeMetadata);
    d->imageChangeGroupBox->setChecked(settings.changeImageProperties);

    slotFileCopyButtonChanged(d->fileCopyButton->isChecked());
}

void FCExportWidget::slotLabelUrlChanged()
{
    d->targetUrl = QUrl::fromLocalFile(d->selector->fileDlgPath());

    Q_EMIT signalTargetUrlChanged(d->targetUrl);
}

void FCExportWidget::slotFileCopyButtonChanged(bool enabled)
{
    if (!enabled)
    {
        d->imageChangeGroupBox->setChecked(false);
    }

    d->imageChangeGroupBox->setEnabled(enabled);
}

} // namespace DigikamGenericFileCopyPlugin

#include "moc_fcexportwidget.cpp"
