/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-09
 * Description : Widget to choose options for face scanning
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "facescanwidget_p.h"

// Local includes

#include "peoplesidebarwidget.h"

namespace Digikam
{

FaceScanWidget::FaceScanWidget(QWidget* const parent)
    : QTabWidget       (parent),
      StateSavingObject(this),
      d                (new Private)
{
    setObjectName(d->configName);
    setupUi();
}

FaceScanWidget::~FaceScanWidget()
{
    delete d;
}

void FaceScanWidget::doLoadState()
{
    KConfigGroup group = getConfigGroup();

    FaceScanSettings set;
    set.readFromConfig(group);

    /**
     * ClearAll isn't a valid value anymore so set it Rescan.
     * ClearAll is only used by ResetFacesDb in maintenance.
     */

    if (FaceScanSettings::AlreadyScannedHandling::ClearAll == set.alreadyScannedHandling)
    {
        set.alreadyScannedHandling = FaceScanSettings::AlreadyScannedHandling::Rescan;
    }

    d->alreadyScannedBox->setCurrentIndex(d->alreadyScannedBox->findData(set.alreadyScannedHandling));
    d->detectAccuracyInput->setValue(set.detectAccuracy);
    d->detectSizeBox->setCurrentIndex(d->detectSizeBox->findData(set.detectSize));
    d->recognizeAccuracyInput->setValue(set.recognizeAccuracy);
    d->useFullCpuButton->setChecked(set.useFullCpu);

    d->albumSelectors->loadState();

    setupConnections();
}

void FaceScanWidget::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    FaceScanSettings set;

    set.alreadyScannedHandling = static_cast<FaceScanSettings::AlreadyScannedHandling>
                                 (d->alreadyScannedBox->itemData(d->alreadyScannedBox->currentIndex()).toInt());
    set.detectAccuracy         = d->detectAccuracyInput->value();
    set.detectSize             = static_cast<FaceScanSettings::FaceDetectionSize>
                                 (d->detectSizeBox->currentData().toInt());
    set.recognizeAccuracy      = d->recognizeAccuracyInput->value();
    set.useFullCpu             = d->useFullCpuButton->isChecked();

    set.writeToConfig(group);

    d->albumSelectors->saveState();
}

void FaceScanWidget::setupUi()
{
    // --- Album tab --------------------------------------------------------------------------------------

    d->albumSelectors                   = new AlbumSelectors(QString(), d->configName,
                                                             this, AlbumSelectors::AlbumType::All, true);
    addTab(d->albumSelectors, i18nc("@title:tab", "Search in"));

    // --- Settings tab -----------------------------------------------------------------------------------

    d->settingsTab                      = new QWidget(this);
    QVBoxLayout* const settingsLayout   = new QVBoxLayout(d->settingsTab);

    // --- Workflow settings ---

    d->workflowWidget                   = new DHBox(d->settingsTab);

    QLabel* const workflowLabel         = new QLabel(i18nc("@label short translated text about the Face Workflow.",
                                                           "Workflow:"), d->workflowWidget);
    workflowLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->alreadyScannedBox                = new SqueezedComboBox(d->workflowWidget);
    d->alreadyScannedBox->addSqueezedItem(i18nc("@label:listbox", "Scan new images"),           FaceScanSettings::Skip);
    d->alreadyScannedBox->addSqueezedItem(i18nc("@label:listbox", "Scan all images"),           FaceScanSettings::Rescan);
    d->alreadyScannedBox->addSqueezedItem(i18nc("@label:listbox", "Recognize faces only"),      FaceScanSettings::RecognizeOnly);
    d->alreadyScannedBox->setToolTip(i18nc("@info:tooltip",
                                           "digiKam can search for faces in your photos.<br>"
                                           "When you have identified your friends on a number of photos,<br>"
                                           "it can also recognize the people shown on your photos."
                                           "<p><b>Scan new images</b> will scan for faces and attempt recognition for new images found in the albums selected in the \"Search in\" tab.</p>"
                                           "<p><b>Scan all images</b> will scan for faces and attempt recognition for all images found in the albums selected in the \"Search in\" tab.</p>"
                                           "<p><b>Recognize faces only</b> will try to match unknown faces with confirmed faces without scanning images new faces.</p>"));

    d->workflowWidget->setStretchFactor(d->alreadyScannedBox, 10);
    d->alreadyScannedBox->setCurrentIndex(FaceScanSettings::Skip);

    // --- Detection settings ---

    DExpanderBox* const expBox          = new DExpanderBox(d->settingsTab);

    QWidget* const detectWidget         = new QWidget(expBox);
    QGridLayout* const detectGrid       = new QGridLayout(detectWidget);

    QLabel* const detectAccuracyLabel   = new QLabel(i18nc("@label short translated text about the Face Detection Accuracy",
                                                           "Accuracy:"), d->settingsTab);
    detectAccuracyLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->detectAccuracyInput              = new DIntNumInput(d->settingsTab);
    d->detectAccuracyInput->setDefaultValue(7);
    d->detectAccuracyInput->setRange(1, 10, 1);
    d->detectAccuracyInput->setToolTip(i18nc("@info:tooltip",
                                             "Adjust sensitivity versus specificity: the higher the value, "
                                             "the more accurately faces will<br>"
                                             "be detected, but less faces will be detected."));

    QLabel* const detectSizeLabel       = new QLabel(i18nc("@label with a short translated text about the face size for detection",
                                                           "Face size:"), d->settingsTab);
    detectSizeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->detectSizeBox                    = new SqueezedComboBox(d->settingsTab);
    d->detectSizeBox->addSqueezedItem(i18nc("@label: listbox face size", "Extra Small"), FaceScanSettings::FaceDetectionSize::ExtraSmall);
    d->detectSizeBox->addSqueezedItem(i18nc("@label: listbox face size", "Small"),       FaceScanSettings::FaceDetectionSize::Small);
    d->detectSizeBox->addSqueezedItem(i18nc("@label: listbox face size", "Medium"),      FaceScanSettings::FaceDetectionSize::Medium);
    d->detectSizeBox->addSqueezedItem(i18nc("@label: listbox face size", "Large"),       FaceScanSettings::FaceDetectionSize::Large);
    d->detectSizeBox->addSqueezedItem(i18nc("@label: listbox face size", "Extra Large"), FaceScanSettings::FaceDetectionSize::ExtraLarge);
    d->detectSizeBox->setEditable(false);
    d->detectSizeBox->setToolTip(i18nc("@info:tooltip",
                                       "<p>Selecting <b>Extra Small</b> means the model will detect small background faces in addition "
                                       "to medium and larger faces. Selecting <b>Small Faces</b> increases the sensitivity of the model "
                                       "by tuning the model parameters. It will also increase the probability of false-positives, "
                                       "and it will increase detection time.</p>"
                                       "<p>Selecting <b>Large</b> or <b>Extra Large</b> will eliminate small background faces from being detected. "
                                       "It is much faster and will reduce the number of false-positives, but will not detect small faces "
                                       "in the background or faces in large group photos.</p>"
                                       "<p>If you want to detect all faces in one pass, select <b>Extra Small</b> faces and set the detection "
                                       "accuracy to 2. Unfortunately, this will create a significant number of false-positives. "
                                       "It is recommended to set a detection accuracy of 6-8 and face size of <b>Medium</b> or <b>Large</b> "
                                       "for normal use.</p><p>This setting applies only to YuNet.</p>"));

    detectGrid->addWidget(detectAccuracyLabel,      0, 0, 1, 1);
    detectGrid->addWidget(d->detectAccuracyInput,   0, 2, 1, 1);
    detectGrid->addWidget(detectSizeLabel,          1, 0, 1, 1);
    detectGrid->addWidget(d->detectSizeBox,         1, 2, 1, 1);

    expBox->addItem(detectWidget, i18nc("@title short translated text", "Detection"),
                    QLatin1String("FaceDetectionSettings"), true);

    // --- Recognition settings ---

    QWidget* const recognizeWidget       = new QWidget(expBox);
    QGridLayout* const recognizeGrid     = new QGridLayout(recognizeWidget);

    QLabel* const recognizeAccuracyLabel = new QLabel(i18nc("@label short translated text about the Face Recognition Accuracy",
                                                            "Accuracy:"), d->settingsTab);
    recognizeAccuracyLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->recognizeAccuracyInput            = new DIntNumInput(d->settingsTab);
    d->recognizeAccuracyInput->setDefaultValue(7);
    d->recognizeAccuracyInput->setRange(1, 10, 1);
    d->recognizeAccuracyInput->setToolTip(i18nc("@info:tooltip",
                                                "Adjust sensitivity versus specificity: the higher the value, the more accurately faces will<br>"
                                                "be recognized, but less faces will be recognized.<br>"
                                                "Note: only faces that are very similar to pre-tagged faces are recognized."));

    recognizeGrid->addWidget(recognizeAccuracyLabel,    0, 0, 1, 1);
    recognizeGrid->addWidget(d->recognizeAccuracyInput, 0, 2, 1, 1);

    // --- Full cpu settings ---

    d->useFullCpuButton                 = new QCheckBox(d->settingsTab);
    d->useFullCpuButton->setText(i18nc("@option:check with a short translated text",
                                       "Uses all processors"));
    d->useFullCpuButton->setToolTip(i18nc("@info:tooltip",
                                          "Face detection and recognition are time-consuming tasks.<br>"
                                          "You can choose if you wish to employ all processor cores<br>"
                                          "on your system, or work in the background only on one core."));

    expBox->addItem(recognizeWidget, i18nc("@title short translated text", "Recognition"),
                    QLatin1String("FaceRecognitionSettings"), false);

    expBox->addStretch();

    // ---

    settingsLayout->addWidget(d->workflowWidget);
    settingsLayout->addWidget(expBox);
    settingsLayout->addWidget(d->useFullCpuButton);

    addTab(d->settingsTab, i18nc("@title:tab", "Settings"));
}

void FaceScanWidget::setupConnections()
{
    connect(d->useFullCpuButton, &QCheckBox::toggled,
            this, &FaceScanWidget::slotSettingsChanged);

    connect(d->detectAccuracyInput, &DIntNumInput::valueChanged,
            this, &FaceScanWidget::slotSettingsChanged);

    connect(d->recognizeAccuracyInput, &DIntNumInput::valueChanged,
            this, &FaceScanWidget::slotSettingsChanged);

    connect(d->detectSizeBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FaceScanWidget::slotSettingsChanged);
}

void FaceScanWidget::slotPrepareForDetect(bool status)
{
    d->alreadyScannedBox->setEnabled(status);
}

void FaceScanWidget::slotPrepareForRecognize(bool /*status*/)
{
    d->alreadyScannedBox->setEnabled(false);
}

void FaceScanWidget::slotSettingsChanged()
{
    KConfigGroup group = getConfigGroup();

    FaceScanSettings set;
    set.readFromConfig(group);

    set.detectSize        = static_cast<FaceScanSettings::FaceDetectionSize>
                            (d->detectSizeBox->currentData().toInt());
    set.useFullCpu        = d->useFullCpuButton->isChecked();
    set.detectAccuracy    = d->detectAccuracyInput->value();
    set.recognizeAccuracy = d->recognizeAccuracyInput->value();

    set.writeToConfig(group);
}

bool FaceScanWidget::settingsConflicted() const
{
    return d->settingsConflicted;
}

FaceScanSettings FaceScanWidget::settings() const
{
    FaceScanSettings settings;

    d->settingsConflicted = false;

    if (FaceScanSettings::RecognizeOnly == d->alreadyScannedBox->itemData(d->alreadyScannedBox->currentIndex()).toInt())
    {
        settings.task                   = FaceScanSettings::RecognizeMarkedFaces;
        settings.alreadyScannedHandling = FaceScanSettings::AlreadyScannedHandling::Rescan;
    }
    else
    {
        settings.task                   = FaceScanSettings::DetectAndRecognize;
        settings.alreadyScannedHandling = (FaceScanSettings::AlreadyScannedHandling)
                                          d->alreadyScannedBox->itemData(d->alreadyScannedBox->currentIndex()).toInt();
    }

    settings.albums                 = d->albumSelectors->selectedAlbumsAndTags();
    settings.wholeAlbums            = d->albumSelectors->wholeAlbumsChecked();

    if (settings.task == FaceScanSettings::RecognizeMarkedFaces)
    {
        int numberOfIdentities      = FaceDbAccess().db()->getNumberOfIdentities();
        d->settingsConflicted       = (numberOfIdentities == 0);
    }

    settings.detectAccuracy         = d->detectAccuracyInput->value();
    settings.detectModel            = FaceScanSettings::FaceDetectionModel::YuNet;
    settings.detectSize             = static_cast<FaceScanSettings::FaceDetectionSize>
                                      (d->detectSizeBox->currentData().toInt());
    settings.recognizeAccuracy      = d->recognizeAccuracyInput->value();
    settings.recognizeModel         = FaceScanSettings::FaceRecognitionModel::SFace;

    settings.useFullCpu             = d->useFullCpuButton->isChecked();

    return settings;
}

} // namespace Digikam

#include "moc_facescanwidget.cpp"
