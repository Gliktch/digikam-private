/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-08-03
 * Description : setup Metadata tab.
 *
 * SPDX-FileCopyrightText: 2003-2004 by Ralf Holzer <ralf at well dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupmetadata_p.h"

namespace Digikam
{

void SetupMetadata::appendSidecarsTab()
{
    QWidget* const sidecarsPanel      = new QWidget(d->tab);
    QVBoxLayout* const sidecarsLayout = new QVBoxLayout(sidecarsPanel);

    // --------------------------------------------------------

    QGroupBox* rwSidecarsGroup          = new QGroupBox;
    QGridLayout* const rwSidecarsLayout = new QGridLayout;

    QLabel* const rwSidecarsLabel       = new QLabel(i18nc("@label", "Reading and Writing to Sidecars"));

    d->readXMPSidecarBox  = new QCheckBox;
    d->readXMPSidecarBox->setText(i18nc("@option:check", "Read from sidecar files"));
    d->readXMPSidecarBox->setWhatsThis(i18nc("@info:whatsthis",
                                             "Turn on this option to read metadata from XMP sidecar files when reading metadata."));
    d->readXMPSidecarBox->setEnabled(MetaEngine::supportXmp());

    d->writeXMPSidecarBox = new QCheckBox;
    d->writeXMPSidecarBox->setText(i18nc("@option:check", "Write to sidecar files"));
    d->writeXMPSidecarBox->setWhatsThis(i18nc("@info:whatsthis",
                                              "Turn on this option to save, as specified, metadata to XMP sidecar files."));
    d->writeXMPSidecarBox->setEnabled(MetaEngine::supportXmp());

    d->writingModeCombo   = new QComboBox;
    d->writingModeCombo->addItem(i18nc("@item", "Write to XMP sidecar for read-only item only"), MetaEngine::WRITE_TO_SIDECAR_ONLY_FOR_READ_ONLY_FILES);
    d->writingModeCombo->addItem(i18nc("@item", "Write to XMP sidecar only"),                    MetaEngine::WRITE_TO_SIDECAR_ONLY);
    d->writingModeCombo->addItem(i18nc("@item", "Write to item and XMP Sidecar"),                MetaEngine::WRITE_TO_SIDECAR_AND_FILE);
    d->writingModeCombo->setToolTip(i18nc("@info:tooltip", "Specify the exact mode of XMP sidecar writing"));
    d->writingModeCombo->setEnabled(false);

    d->sidecarFileNameBox = new QCheckBox;
    d->sidecarFileNameBox->setText(i18nc("@option:check", "Sidecar file names are compatible with commercial programs"));
    d->sidecarFileNameBox->setWhatsThis(i18nc("@info:whatsthis",
                                              "Turn on this option to create the XMP sidecar files with a compatible "
                                              "file name (BASENAME.xmp) used by many commercial programs. "
                                              "For Darktable do not enable this option."));
    d->sidecarFileNameBox->setEnabled(false);

    rwSidecarsLayout->addWidget(rwSidecarsLabel,       0, 0, 1, 3);
    rwSidecarsLayout->addWidget(d->readXMPSidecarBox,  1, 0, 1, 3);
    rwSidecarsLayout->addWidget(d->writeXMPSidecarBox, 2, 0, 1, 3);
    rwSidecarsLayout->addWidget(d->writingModeCombo,   3, 1, 1, 2);
    rwSidecarsLayout->addWidget(d->sidecarFileNameBox, 4, 0, 1, 3);
    rwSidecarsLayout->setColumnStretch(3, 10);
    rwSidecarsGroup->setLayout(rwSidecarsLayout);

    // --------------------------------------------------------

    QGroupBox* const extensionsGroup   = new QGroupBox(sidecarsPanel);
    QGridLayout* const extensionsGrid  = new QGridLayout(extensionsGroup);

    QLabel* const extensionsGroupLabel = new QLabel(
                i18nc("@info", "Add file types to be recognised as sidecars.\n"
                      "digiKam (optionally) writes metadata to *.xmp sidecar "
                      "files. Other programs might use different types, which "
                      "can be specified below. digiKam will neither display these "
                      "nor read from or write to them. But whenever a matching album "
                      "item (e.g. \"image.dng\" for \"image.dng.pp3\") is renamed, "
                      "moved, copied or deleted, the same operation will be done "
                      "on these sidecar files.\n"
                      "Multiple extensions must be separated by a semicolon or a space."));
    extensionsGroupLabel->setWordWrap(true);

    QLabel* const extensionsLogo = new QLabel(extensionsGroup);
    extensionsLogo->setPixmap(QIcon::fromTheme(QLatin1String("text-x-texinfo")).pixmap(48));

    d->extensionsEdit            = new QLineEdit(extensionsGroup);
    d->extensionsEdit->setWhatsThis(i18nc("@info", "Here you can add extra extensions\n"
                                          "of sidecars files to be processed alongside\n"
                                          "regular items. These files will not be visible,\n"
                                          "but regarded as an extension of the main file.\n"
                                          "Just write \"xyz abc\" to support files with\n"
                                          "the *.xyz and *.abc extensions. The internally\n"
                                          "used sidecars type *.xmp is always included."));
    d->extensionsEdit->setClearButtonEnabled(true);
    d->extensionsEdit->setPlaceholderText(i18nc("@info", "Enter additional sidecars file extensions."));

    QLabel* const extensionsLabel = new QLabel(extensionsGroup);
    extensionsLabel->setText(i18nc("@info", "Additional &sidecar file extensions"));
    extensionsLabel->setBuddy(d->extensionsEdit);

    extensionsGrid->addWidget(extensionsGroupLabel, 0, 0, 1, -1);
    extensionsGrid->addWidget(extensionsLogo,       1, 0, 2, 1);
    extensionsGrid->addWidget(extensionsLabel,      1, 1, 1, -1);
    extensionsGrid->addWidget(d->extensionsEdit,    2, 1, 1, -1);
    extensionsGrid->setColumnStretch(1, 10);

    // --------------------------------------------------------

    sidecarsLayout->addWidget(rwSidecarsGroup);
    sidecarsLayout->addWidget(extensionsGroup);
    sidecarsLayout->addStretch();

    d->tab->insertTab(Sidecars, sidecarsPanel, i18nc("@title:tab", "Sidecars"));
}

} // namespace Digikam
