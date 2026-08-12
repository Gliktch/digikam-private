/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-11-24
 * Description : Color management setup tab.
 *
 * SPDX-FileCopyrightText: 2005-2007 by F.J. Cruz <fj dot cruz at supercable dot es>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupicc_p.h"

namespace Digikam
{

SetupICC::SetupICC(QDialogButtonBox* const dlgBtnBox, QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    const int spacing              = layoutSpacing();

    d->dlgBtnBox                   = dlgBtnBox;
    d->tab                         = new QTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    d->behaviorPanel               = new QWidget;
    QVBoxLayout* const mainLayout  = new QVBoxLayout(d->behaviorPanel);

    // --------------------------------------------------------

    QWidget* const colorPolicy     = new QWidget;
    QGridLayout* const gridHeader  = new QGridLayout(colorPolicy);

    d->enableColorManagement       = new QCheckBox(colorPolicy);
    d->enableColorManagement->setText(i18n("Enable Color Management"));
    d->enableColorManagement->setWhatsThis(i18n("<ul><li>Checked: Color Management is enabled</li>"
                                                "<li>Unchecked: Color Management is "
                                                "disabled</li></ul>"));

    DActiveLabel* const lcmsLogoLabel = new DActiveLabel(QUrl(QLatin1String("https://www.littlecms.com")),
                                                         QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/logo-lcms.webp")),
                                                         colorPolicy);
    lcmsLogoLabel->setToolTip(i18n("Visit Little CMS project website"));

    gridHeader->addWidget(d->enableColorManagement, 0, 0, 1, 1);
    gridHeader->addWidget(lcmsLogoLabel,            0, 2, 1, 1);
    gridHeader->setColumnStretch(1, 10);
    gridHeader->setContentsMargins(spacing, spacing, spacing, spacing);
    gridHeader->setSpacing(0);

    // --------------------------------------------------------

    d->workspaceGB             = new QGroupBox(i18n("Working Color Space"));
    QHBoxLayout* const hboxWS  = new QHBoxLayout(d->workspaceGB);

    QLabel* const workIcon     = new QLabel;
    workIcon->setPixmap(QIcon::fromTheme(QLatin1String("input-tablet")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));
    d->workProfilesKC          = new IccProfilesComboBox;
    d->workProfilesKC->setWhatsThis(i18n("<p>This is the color space all the images will be converted to when opened "
                                         "(if you choose to convert) and the profile that will be embedded when saving. "
                                         "Good and safe choices are <b>Adobe RGB (1998)</b> and <b>sRGB IEC61966-2.1</b></p>"));

    d->infoWorkProfiles        = new QPushButton;
    d->infoWorkProfiles->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
    d->infoWorkProfiles->setWhatsThis(i18n("<p>You can use this button to get more detailed "
                                           "information about the selected workspace profile.</p>"));

    hboxWS->addWidget(workIcon);
    hboxWS->addWidget(d->workProfilesKC, 10);
    hboxWS->addWidget(d->infoWorkProfiles);

    // --------------------------------------------------------

    d->mismatchGB                   = new QGroupBox;  // NOTE: Behavior on Profile Mismatch
    QVBoxLayout* const vlayMismatch = new QVBoxLayout(d->mismatchGB);

    QLabel* const behaviorIcon      = new QLabel;
    behaviorIcon->setPixmap(QIcon::fromTheme(QLatin1String("view-preview")).pixmap(32));
    QLabel* const behaviorLabel     = new QLabel(i18n("When the profile of an image does not match the working color space"));
    behaviorLabel->setWordWrap(true);

    QHBoxLayout* const hboxBL       = new QHBoxLayout;
    hboxBL->addWidget(behaviorIcon);
    hboxBL->addWidget(behaviorLabel, 10);

    d->defaultAskMismatch           = new QRadioButton(d->mismatchGB);
    d->defaultAskMismatch->setText(i18n("Ask when opening the image"));
    d->defaultAskMismatch->setWhatsThis(i18n("<p>If an image has an embedded color profile not matching the working "
                                             "space profile, digiKam will ask if you want to convert to the working space, "
                                             "keep the embedded profile or discard the embedded profile and assign "
                                             "a different one.</p>"));

    d->defaultConvertMismatch       = new QRadioButton(d->mismatchGB);
    d->defaultConvertMismatch->setText(i18n("Convert the image to the working color space"));
    d->defaultConvertMismatch->setWhatsThis(i18n("<p>If an image has an embedded color profile not matching the working "
                                                 "space profile, digiKam will convert the image's color information to "
                                                 "the working color space. This changes the pixel data, but not the "
                                                 "appearance of the image.</p>"));

    vlayMismatch->addLayout(hboxBL);
    vlayMismatch->addWidget(d->defaultAskMismatch);
    vlayMismatch->addWidget(d->defaultConvertMismatch);

    // --------------------------------------------------------

    d->missingGB                    = new QGroupBox;        // Missing Profile Behavior
    QVBoxLayout* const vlayMissing  = new QVBoxLayout(d->missingGB);

    QLabel* const  missingIcon      = new QLabel;
    missingIcon->setPixmap(QIcon::fromTheme(QLatin1String("paint-unknown")).pixmap(32));
    QLabel* const missingLabel      = new QLabel(i18n("When an image has no color profile information"));
    missingLabel->setWordWrap(true);

    QHBoxLayout* const hboxMP       = new QHBoxLayout;
    hboxMP->addWidget(missingIcon);
    hboxMP->addWidget(missingLabel, 10);

    d->defaultAskMissing            = new QRadioButton(i18n("Ask when opening the image"));
    d->defaultAskMissing->setWhatsThis(i18n("<p>If an image has no embedded color profile, "
                                            "digiKam will ask which color space shall be used to interpret the image "
                                            "and to which color space it shall be transformed for editing.</p>"));

    d->defaultSRGBMissing = new QRadioButton(i18n("Assume it is using the sRGB color space (Internet standard)"));

    /**
     * @todo d->defaultSRGBMissing->setWhatsThis(i18n("<p></p>"));
     */

    d->defaultSRGBConvert = new QCheckBox(i18n("and convert it to the working color space"));

    /**
     * @todo d->defaultSRGBConvert->setWhatsThis(i18n("<p></p>"));
     */

    d->defaultSRGBConvert->setChecked(true);

    QGridLayout* const gridRgb  = new QGridLayout;
    gridRgb->addWidget(d->defaultSRGBMissing, 0, 0, 1, 2);
    gridRgb->addWidget(d->defaultSRGBConvert, 1, 1);
    gridRgb->setColumnMinimumWidth(0, 10);

    d->defaultWSMissing         = new QRadioButton(i18n("Assume it is using the working color space"));

    /**
     * @todo d->defaultWSMissing->setWhatsThis(i18n("<p></p>"));
     */

    d->defaultInputMissing      = new QRadioButton(i18n("Convert it from default input color space to working space"));

    /**
     * @todo d->defaultInputMissing->setWhatsThis(i18n("<p></p>"));
     */

    vlayMissing->addLayout(hboxMP);
    vlayMissing->addWidget(d->defaultAskMissing);
    vlayMissing->addLayout(gridRgb);
    vlayMissing->addWidget(d->defaultWSMissing);
    vlayMissing->addWidget(d->defaultInputMissing);

    // --------------------------------------------------------

    d->rawGB                       = new QGroupBox;         // Raw File Behavior
    QVBoxLayout* const vlayRaw     = new QVBoxLayout(d->rawGB);

    QLabel* const rawBehaviorIcon  = new QLabel;
    rawBehaviorIcon->setPixmap(QIcon::fromTheme(QLatin1String("image-x-adobe-dng")).pixmap(32));
    QLabel* const rawBehaviorLabel = new QLabel(i18n("When loading a RAW file with uncalibrated colors"));
    rawBehaviorLabel->setWordWrap(true);

    QHBoxLayout* const hboxRF      = new QHBoxLayout;
    hboxRF->addWidget(rawBehaviorIcon);
    hboxRF->addWidget(rawBehaviorLabel, 10);

    d->defaultAskRaw   = new QRadioButton(i18n("Ask for the input profile"));

    /**
     * @todo d->defaultAskRaw->setWhatsThis(i18n("<p></p>"));
     */

    d->defaultGuessRaw = new QRadioButton(i18n("Automatic color correction"));

    /**
     * @todo d->defaultGuessRaw->setWhatsThis(i18n("<p></p>"));
     */

    d->defaultInputRaw = new QRadioButton(i18n("Convert it from the default input profile"));

    /**
     * @todo d->defaultSRGBMissing->setWhatsThis(i18n("<p></p>"));
     */

    d->defaultGuessRaw->setChecked(true);

    vlayRaw->addLayout(hboxRF);
    vlayRaw->addWidget(d->defaultAskRaw);
    vlayRaw->addWidget(d->defaultGuessRaw);
    vlayRaw->addWidget(d->defaultInputRaw);

    mainLayout->addWidget(colorPolicy);
    mainLayout->addWidget(d->workspaceGB);
    mainLayout->addWidget(d->mismatchGB);
    mainLayout->addWidget(d->missingGB);
    mainLayout->addWidget(d->rawGB);
    mainLayout->addStretch();

    // --------------------------------------------------------

    d->profilesPanel               = new QWidget;
    QVBoxLayout* const vboxDisplay = new QVBoxLayout(d->profilesPanel);
    d->viewGB                      = new QGroupBox(i18n("Color Managed View"));
    QGridLayout* const gridView    = new QGridLayout(d->viewGB);

    QLabel* const monitorIcon      = new QLabel;
    monitorIcon->setPixmap(QIcon::fromTheme(QLatin1String("video-display")).pixmap(32));
    QLabel* const monitorProfiles  = new QLabel(i18n("Monitor profile:"));

    d->monitorProfilesKC           = new IccProfilesComboBox;
    monitorProfiles->setBuddy(d->monitorProfilesKC);
    d->monitorProfilesKC->setWhatsThis(i18n("<p>If your desktop do not provide a color profile for your screen, "
                                            "select here the default one. If no profile is set, "
                                            "sRGB will be used as fail-back. In all cases, the color "
                                            "management workflow will try to get in first the profile set "
                                            "for your monitor in your the desktop</p>"));

    d->infoMonitorProfiles         = new QPushButton;
    d->infoMonitorProfiles->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
    d->infoMonitorProfiles->setWhatsThis(i18n("<p>You can use this button to get more detailed "
                                              "information about the selected monitor profile.</p>"));

    d->managedView                 = new QCheckBox;
    d->managedView->setText(i18n("Use color managed view in editor"));
    d->managedView->setWhatsThis(i18n("<p>Turn on this option if "
                                      "you want to use your <b>Monitor Color Profile</b> to show your pictures in "
                                      "the Image Editor window with a color correction adapted to your monitor. "
                                      "You can at any time toggle this option from the Editor window. "
                                      "<i>Warning</i>: This can slow down rendering of the image, depending on the speed of your computer.</p>"));

    d->managedPreviews             = new QCheckBox;
    d->managedPreviews->setText(i18n("Use color managed view for previews and thumbnails"));

    /**
     * @todo d->managedPreview->setWhatsThis(i18n(""));
     */

    gridView->addWidget(monitorIcon,            0, 0);
    gridView->addWidget(monitorProfiles,        0, 1, 1, 2);
    gridView->addWidget(d->monitorProfilesKC,   1, 0, 1, 2);
    gridView->addWidget(d->infoMonitorProfiles, 1, 2);
    gridView->addWidget(d->managedView,         2, 0, 1, 3);
    gridView->addWidget(d->managedPreviews,     3, 0, 1, 3);
    gridView->setColumnStretch(1, 10);

    // --------------------------------------------------------

    d->inputGB                 = new QGroupBox(i18n("Camera and Scanner"));
    QGridLayout* const gridIP  = new QGridLayout(d->inputGB);

    QLabel* const inputIcon    = new QLabel;
    inputIcon->setPixmap(QIcon::fromTheme(QLatin1String("input-tablet")).pixmap(32));
    QLabel* const inputLabel   = new QLabel(i18n("Default input color profile:"));
    d->inProfilesKC            = new IccProfilesComboBox;
    d->inProfilesKC->setWhatsThis(i18n("<p>This is the default color profile for your input device "
                                       "(that is your camera - or your scanner). A camera input profile "
                                       "is recommended for correct conversion of RAW images in 16bit. "
                                       "Some of the options about loading behavior above refer to this profile.</p>"));

    d->infoInProfiles          = new QPushButton;
    d->infoInProfiles->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
    d->infoInProfiles->setWhatsThis(i18n("<p>You can use this button to get more detailed "
                                         "information about the selected input profile.</p>"));

    gridIP->addWidget(inputIcon,         0, 0);
    gridIP->addWidget(inputLabel,        0, 1, 1, 2);
    gridIP->addWidget(d->inProfilesKC,   1, 0, 1, 2);
    gridIP->addWidget(d->infoInProfiles, 1, 2);
    gridIP->setColumnStretch(1, 10);

    // --------------------------------------------------------

    d->proofGB                   = new QGroupBox(i18n("Printing and Proofing"));
    QGridLayout* const gridProof = new QGridLayout(d->proofGB);

    QLabel* const proofIcon      = new QLabel;
    proofIcon->setPixmap(QIcon::fromTheme(QLatin1String("document-print")).pixmap(32));
    QLabel* const proofLabel     = new QLabel(i18n("Output device profile:"));
    d->proofProfilesKC           = new IccProfilesComboBox;
    proofLabel->setBuddy(d->proofProfilesKC);
    d->proofProfilesKC->setWhatsThis(i18n("<p>Select the profile for your output device "
                                          "(usually, your printer). This profile will be used to do a soft proof, so you will "
                                          "be able to preview how an image will be rendered via an output device.</p>"));

    d->infoProofProfiles         = new QPushButton;
    d->infoProofProfiles->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
    d->infoProofProfiles->setWhatsThis(i18n("<p>You can use this button to get more detailed "
                                            "information about the selected proofing profile.</p>"));

    gridProof->addWidget(proofIcon,            0, 0);
    gridProof->addWidget(proofLabel,           0, 1, 1, 2);
    gridProof->addWidget(d->proofProfilesKC,   1, 0, 1, 2);
    gridProof->addWidget(d->infoProofProfiles, 1, 2);
    gridProof->setColumnStretch(1, 10);

    // --------------------------------------------------------

    d->iccFolderGB                   = new QGroupBox(i18n("Color Profiles Repository"));
    QGridLayout* const gridIccFolder = new QGridLayout(d->iccFolderGB);

    QLabel* const iccFolderIcon = new QLabel;
    iccFolderIcon->setPixmap(QIcon::fromTheme(QLatin1String("folder-downloads")).pixmap(32));
    d->iccFolderLabel           = new QLabel(i18n("digiKam looks for ICC profiles in a number of <a href='default'>default locations</a>. "
                                                 "You can specify an additional folder:"));
    d->iccFolderLabel->setWordWrap(true);

    d->defaultPathKU            = new DFileSelector;
    d->iccFolderLabel->setBuddy(d->defaultPathKU);
    d->defaultPathKU->setFileDlgMode(QFileDialog::Directory);
    d->defaultPathKU->setFileDlgOptions(QFileDialog::ShowDirsOnly);
    d->defaultPathKU->setWhatsThis(i18n("<p>digiKam searches ICC profiles in default system folders "
                                        "and ships itself a few selected profiles. "
                                        "Store all your additional color profiles in the directory set here.</p>"));

    gridIccFolder->addWidget(iccFolderIcon,     0, 0);
    gridIccFolder->addWidget(d->iccFolderLabel, 0, 1);
    gridIccFolder->addWidget(d->defaultPathKU,  1, 0, 1, 2);
    gridIccFolder->setColumnStretch(1, 10);

    vboxDisplay->addWidget(d->viewGB);
    vboxDisplay->addWidget(d->inputGB);
    vboxDisplay->addWidget(d->proofGB);
    vboxDisplay->addWidget(d->iccFolderGB);
    vboxDisplay->addStretch(1);

    // --------------------------------------------------------

    d->advancedPanel                = new QWidget;
    QVBoxLayout* const vboxAdvanced = new QVBoxLayout(d->advancedPanel);
    d->advancedSettingsGB           = new QGroupBox(i18n("Advanced Settings"));
    QGridLayout* const gridAdvanced = new QGridLayout(d->advancedSettingsGB);

    d->bpcAlgorithm                 = new QCheckBox(d->advancedSettingsGB);
    d->bpcAlgorithm->setText(i18n("Use black point compensation"));
    d->bpcAlgorithm->setWhatsThis(i18n("<p><b>Black Point Compensation</b> is a way to make "
                                       "adjustments between the maximum "
                                       "black levels of digital files and the black capabilities of various "
                                       "digital devices.</p>"));

    QLabel* const label            = new QLabel(d->advancedSettingsGB);
    label->setText(i18n("Rendering Intents:"));

    d->renderingIntentKC            = new IccRenderingIntentComboBox(d->advancedSettingsGB);

    gridAdvanced->addWidget(d->bpcAlgorithm,      0, 0, 1, 2);
    gridAdvanced->addWidget(label,                1, 0, 1, 1);
    gridAdvanced->addWidget(d->renderingIntentKC, 1, 1, 1, 1);
    gridAdvanced->setContentsMargins(spacing, spacing, spacing, spacing);
    gridAdvanced->setSpacing(0);

    vboxAdvanced->addWidget(d->advancedSettingsGB);
    vboxAdvanced->addStretch(1);

    // --------------------------------------------------------

    d->tab->insertTab(Behavior, d->behaviorPanel, i18nc("@title: icc panel tab", "Behavior"));
    d->tab->insertTab(Profiles, d->profilesPanel, i18nc("@title: icc panel tab", "Profiles"));
    d->tab->insertTab(Advanced, d->advancedPanel, i18nc("@title: icc panel tab", "Advanced"));

    // --------------------------------------------------------

    connect(d->enableColorManagement, SIGNAL(toggled(bool)),
            this, SLOT(slotToggledEnabled()));

    connect(d->infoProofProfiles, SIGNAL(clicked()),
            this, SLOT(slotClickedProof()));

    connect(d->infoInProfiles, SIGNAL(clicked()),
            this, SLOT(slotClickedIn()));

    connect(d->infoMonitorProfiles, SIGNAL(clicked()),
            this, SLOT(slotClickedMonitor()));

    connect(d->infoWorkProfiles, SIGNAL(clicked()),
            this, SLOT(slotClickedWork()));

    connect(d->defaultPathKU, SIGNAL(signalUrlSelected(QUrl)),
            this, SLOT(slotUrlChanged()));

    connect(d->defaultPathKU->lineEdit(), SIGNAL(textChanged(QString)),
            this, SLOT(slotUrlTextChanged()));

    connect(d->iccFolderLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(slotShowDefaultSearchPaths()));

    connect(d->defaultAskMissing, SIGNAL(toggled(bool)),
            this, SLOT(slotMissingToggled(bool)));

    connect(d->defaultSRGBMissing, SIGNAL(toggled(bool)),
            this, SLOT(slotMissingToggled(bool)));

    connect(d->defaultWSMissing, SIGNAL(toggled(bool)),
            this, SLOT(slotMissingToggled(bool)));

    connect(d->defaultInputMissing, SIGNAL(toggled(bool)),
            this, SLOT(slotMissingToggled(bool)));

    // --------------------------------------------------------

    adjustSize();

    readSettings();
    slotToggledEnabled();
}

SetupICC::~SetupICC()
{
    delete d;
}

} // namespace Digikam

#include "moc_setupicc.cpp"
