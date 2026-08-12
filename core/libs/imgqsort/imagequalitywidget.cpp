/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : Image Quality settings widget
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imagequalitywidget_p.h"

namespace Digikam
{

ImageQualityWidget::ImageQualityWidget(SettingsDisplayMode displayMode, QWidget* const parent)
    : QTabWidget       (parent),
      StateSavingObject(this),
      d                (new Private)
{
    d->displayMode = displayMode;
    setObjectName(d->configName);
    setupUi();
}

ImageQualityWidget::~ImageQualityWidget()
{
    delete d;
}

void ImageQualityWidget::doLoadState()
{
    KConfigGroup group = getConfigGroup();
    d->albumSelectors->loadState();

    readSettings(group);
}

void ImageQualityWidget::doSaveState()
{
    KConfigGroup group = getConfigGroup();
    d->albumSelectors->saveState();

    applySettings(group);
}

void ImageQualityWidget::setupUi()
{
    const int spacing         = layoutSpacing();

    // --- Album tab ----------------------------------------------------------------

    d->albumSelectors         = new AlbumSelectors(QString(), d->configName,
                                                   this, AlbumSelectors::AlbumType::All, true);
    addTab(d->albumSelectors, i18nc("@title:tab", "Search in"));

    // ---- Settings tab ------------------------------------------------------------

    d->settingsView           = new QWidget(this);
    d->settingsView->setWhatsThis(i18nc("@info", "These settings determine the quality of an image and convert it into a score "
                                                 "stored in database as Pick Label property. This information can be evaluated two ways: "
                                                 "using four basic factors to rate the image (blur, noise, exposure, and compression), "
                                                 "or using a deep learning neural network engine. The first one helps to determine whether "
                                                 "images are distorted by the basic factors, however it requires fine-tuning "
                                                 "by the user and it cannot work along the aesthetic image processing. "
                                                 "The second one uses an AI approach based on %1 model to predict the score. "
                                                 "As deep learning is an end-to-end solution, it doesn’t require adjusting the settings, "
                                                 "and makes this feature easier to use.",
                                        QString::fromUtf8("<a href='https://expertphotography.com/aesthetic-photography/'>%1</a>")
                                            .arg(i18nc("@label", "aesthetic image quality"))));

    DHBox* const hlay0        = new DHBox(d->settingsView);
    new QLabel(i18nc("@label: short translated text about the image quality scan mode", "Scan Mode:"), hlay0);
    QWidget* const space8     = new QWidget(hlay0);
    hlay0->setStretchFactor(space8, 10);

    d->scanMode               = new SqueezedComboBox(hlay0);
    d->scanMode->addSqueezedItem(i18nc("@item: all items to scan with image quality scan",  "All"),                ImageQualitySettings::ScanMode::AllItems);
    d->scanMode->addSqueezedItem(i18nc("@item: image quality scan only non assigned items", "Non-Assigned Only"),  ImageQualitySettings::ScanMode::NonAssignedItems);
    d->scanMode->setToolTip(i18nc("@info:tooltip",
                                  "<p><b>All</b>: re-scan all items for quality.</p>"
                                  "<p><b>Non-Assigned Only</b>: scan only the items with no assigned auto-pick labels.</p>"));

    // ------------------------------------------------------------------------------

    DHBox* const hlay1        = new DHBox(d->settingsView);

    d->setRejected            = new QCheckBox(i18nc("@option:check with a short translated text",
                                                    "Low as Rejected"), hlay1);
    d->setRejected->setToolTip(i18nc("@info:tooltip", "Low quality images detected by the quality engine<br>"
                                     "will be assigned to the rejected pick label (red flag)."));

    QWidget* const hspace1    = new QWidget(hlay1);
    hlay1->setStretchFactor(hspace1, 10);

    QLabel* const workIcon1   = new QLabel(hlay1);
    workIcon1->setPixmap(QIcon::fromTheme(QLatin1String("flag-red")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));

    // ------------------------------------------------------------------------------

    DHBox* const hlay2        = new DHBox(d->settingsView);

    d->setPending             = new QCheckBox(i18nc("@option:check with a short translated text",
                                                    "Medium as Pending"), hlay2);
    d->setPending->setToolTip(i18nc("@info:tooltip", "Medium quality images detected by the quality engine<br>"
                                    "will be assigned to the pending pick label (yellow flag)."));

    QWidget* const hspace2    = new QWidget(hlay2);
    hlay2->setStretchFactor(hspace2, 10);

    QLabel* const workIcon2   = new QLabel(hlay2);
    workIcon2->setPixmap(QIcon::fromTheme(QLatin1String("flag-yellow")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));

    // ------------------------------------------------------------------------------

    DHBox* const hlay3        = new DHBox(d->settingsView);

    d->setAccepted            = new QCheckBox(i18nc("@option:check with a short translated text",
                                                    "High as Accepted"), hlay3);
    d->setAccepted->setToolTip(i18nc("@info:tooltip", "High quality images detected by the quality engine<br>"
                                     "will be assigned to the accepted pick label (green flag)."));

    QWidget* const hspace3    = new QWidget(hlay3);
    hlay3->setStretchFactor(hspace3, 10);

    QLabel* const workIcon3   = new QLabel(hlay3);
    workIcon3->setPixmap(QIcon::fromTheme(QLatin1String("flag-green")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));

    // ------------------------------------------------------------------------------

    QLabel* const engineLbl   = new QLabel(i18nc("@label with a short translated text", "Quality Engine:"), d->settingsView);

    d->detectButtonGroup      = new QButtonGroup(d->settingsView);
    d->detectButtonGroup->setExclusive(true);

    d->detectAesthetic        = new QRadioButton(i18nc("@option:radio with a short translated text",
                                                       "Aesthetic Contents"), this);
    d->detectAesthetic->setToolTip(i18nc("@info:tooltip", "Detect if the image has aesthetic contents.<br>"
                                         "The aesthetic detection engine is based on a deep learning model to classify images."));

    d->detectButtonGroup->addButton(d->detectAesthetic, Private::AESTHETIC);

    d->detectBasicFactors     = new QRadioButton(i18nc("@option:radio with a short translated text",
                                                       "Basic Factors"), this);
    d->detectBasicFactors->setToolTip(i18nc("@info:tooltip", "Detect if the image is sabotaging using four basic factors<br>"
                                            "eg. blur, noise, exposure (under / over), and compression levels.<br>"
                                            "See the relevant settings from the next <b>Basic Factors</b> tab."));

    d->detectButtonGroup->addButton(d->detectBasicFactors, Private::BASICFACTORS);
    d->detectAesthetic->setChecked(true);

    // ------------------------------------------------------------------------------

    DHBox* const hbox16         = new DHBox(d->settingsView);
    d->useFullCpuButton         = new QCheckBox(hbox16);
    d->useFullCpuButton->setText(i18nc("@option:check with a short translated text",
                                       "Uses all processors"));
    d->useFullCpuButton->setToolTip(i18nc("@info:tooltip",
                                          "Quality detection is time-consuming tasks.<br>"
                                          "You can choose if you wish to employ all processor cores<br>"
                                          "on your system, or work in the background only on one core."));

    // ------------------------------------------------------------------------------

    QGridLayout* const glay = new QGridLayout(d->settingsView);
    glay->addWidget(hlay0,                 0, 0, 1, 2);
    glay->addWidget(hlay1,                 1, 0, 1, 2);
    glay->addWidget(hlay2,                 2, 0, 1, 2);
    glay->addWidget(hlay3,                 3, 0, 1, 2);
    glay->addWidget(engineLbl,             4, 0, 1, 2);
    glay->addWidget(d->detectAesthetic,    5, 1, 1, 1);
    glay->addWidget(d->detectBasicFactors, 6, 1, 1, 1);
    glay->addWidget(hbox16,                7, 0, 1, 2);
    glay->setColumnStretch(1, 10);
    glay->setRowStretch(8, 10);
    glay->setContentsMargins(spacing, spacing, spacing, spacing);

    addTab(d->settingsView, i18nc("@title:tab", "Settings"));

    // --- Basic Factors tab --------------------------------------------------------

    d->basicView              = new QWidget(this);
    QGridLayout* const grid1  = new QGridLayout(d->basicView);

    d->lbl2                   = new QLabel(i18nc("@label with a short translated text",
                                                 "Rejected:"), d->basicView);
    d->lbl2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->setRejectedThreshold   = new DIntNumInput(d->basicView);
    d->setRejectedThreshold->setDefaultValue(10);
    d->setRejectedThreshold->setRange(1, 100, 1);
    d->setRejectedThreshold->setToolTip(i18nc("@info:tooltip",
                                              "Threshold below which all pictures are assigned Rejected Label"));

    d->lbl3                   = new QLabel(i18nc("@label with a short translated text",
                                                 "Pending:"), d->basicView);
    d->lbl3->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->setPendingThreshold    = new DIntNumInput(d->basicView);
    d->setPendingThreshold->setDefaultValue(40);
    d->setPendingThreshold->setRange(1, 100, 1);
    d->setPendingThreshold->setToolTip(i18nc("@info:tooltip",
                                             "Threshold below which all pictures are assigned Pending Label"));

    d->lbl4                   = new QLabel(i18nc("@label with a short translated text",
                                                 "Accepted:"), d->basicView);
    d->lbl4->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->setAcceptedThreshold   = new DIntNumInput(d->basicView);
    d->setAcceptedThreshold->setDefaultValue(60);
    d->setAcceptedThreshold->setRange(1, 100, 1);
    d->setAcceptedThreshold->setToolTip(i18nc("@info:tooltip",
                                              "Threshold above which all pictures are assigned Accepted Label"));

    d->detectBlur             = new QCheckBox(i18nc("@option:check with a short translated text",
                                                    "Blur"), d->basicView);
    d->detectBlur->setToolTip(i18nc("@info:tooltip", "Detect the amount of blur in the images passed to it"));

    d->lbl5                   = new QLabel(i18nc("@label with a short translated text",
                                                 "Weight:"), d->basicView);
    d->lbl5->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->setBlurWeight          = new DIntNumInput(d->basicView);
    d->setBlurWeight->setDefaultValue(100);
    d->setBlurWeight->setRange(1, 100, 1);
    d->setBlurWeight->setToolTip(i18nc("@info:tooltip", "Weight to assign to Blur Algorithm"));

    d->detectNoise            = new QCheckBox(i18nc("@option:check with a short translated text", "Noise"), d->basicView);
    d->detectNoise->setToolTip(i18nc("@info:tooltip", "Detect the amount of noise in the images passed to it"));

    d->lbl6                   = new QLabel(i18nc("@label with a short translated text",
                                                 "Weight:"), d->basicView);
    d->lbl6->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->setNoiseWeight         = new DIntNumInput(d->basicView);
    d->setNoiseWeight->setDefaultValue(100);
    d->setNoiseWeight->setRange(1, 100, 1);
    d->setNoiseWeight->setToolTip(i18nc("@info:tooltip", "Weight to assign to Noise Algorithm"));

    d->detectCompression      = new QCheckBox(i18nc("@option:check with a short translated text",
                                                    "Compression"), d->basicView);
    d->detectCompression->setToolTip(i18nc("@info:tooltip", "Detect the amount of compression in the images passed to it"));

    d->lbl7                   = new QLabel(i18nc("@label with a short translated text",
                                                 "Weight:"), d->basicView);
    d->lbl7->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->setCompressionWeight   = new DIntNumInput(d->basicView);
    d->setCompressionWeight->setDefaultValue(100);
    d->setCompressionWeight->setRange(1, 100, 1);
    d->setCompressionWeight->setToolTip(i18nc("@info:tooltip", "Weight to assign to Compression Algorithm"));

    d->detectExposure         = new QCheckBox(i18nc("@option:check with a short translated text",
                                                    "Exposure"), d->basicView);
    d->detectExposure->setToolTip(i18nc("@info:tooltip", "Detect if the images are under-exposed or over-exposed"));

    // ------------------------------------------------------------------------------

    grid1->addWidget(d->lbl2,                  0, 0, 1, 2);
    grid1->addWidget(d->setRejectedThreshold,  0, 2, 1, 1);
    grid1->addWidget(d->lbl3,                  1, 0, 1, 2);
    grid1->addWidget(d->setPendingThreshold,   1, 2, 1, 1);
    grid1->addWidget(d->lbl4,                  2, 0, 1, 2);
    grid1->addWidget(d->setAcceptedThreshold,  2, 2, 1, 1);

    grid1->addWidget(d->detectBlur,            3, 0, 1, 1);
    grid1->addWidget(d->lbl5,                  3, 1, 1, 1);
    grid1->addWidget(d->setBlurWeight,         3, 2, 1, 1);

    grid1->addWidget(d->detectNoise,           4, 0, 1, 1);
    grid1->addWidget(d->lbl6,                  4, 1, 1, 1);
    grid1->addWidget(d->setNoiseWeight,        4, 2, 1, 1);

    grid1->addWidget(d->detectCompression,     5, 0, 1, 1);
    grid1->addWidget(d->lbl7,                  5, 1, 1, 1);
    grid1->addWidget(d->setCompressionWeight,  5, 2, 1, 1);

    grid1->addWidget(d->detectExposure,        6, 0, 1, 3);
    grid1->setContentsMargins(spacing, spacing, spacing, spacing);
    grid1->setColumnStretch(0, 1);
    grid1->setColumnStretch(1, 1);
    grid1->setColumnStretch(2, 100);
    grid1->setRowStretch(7, 10);

    addTab(d->basicView, i18nc("@title:tab", "Basic Factors"));

    // ------------------------------------------------------------------------------

    connect(d->scanMode, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->detectBlur, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->detectNoise, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->detectCompression, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->detectExposure, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->setRejected, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->setPending, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->setAccepted, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->setRejectedThreshold, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->setPendingThreshold, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->setAcceptedThreshold, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->setBlurWeight, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->setNoiseWeight, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->setCompressionWeight, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    // ------------------------------------------------------------------------------

    connect(d->detectBlur, SIGNAL(toggled(bool)),
            d->lbl5, SLOT(setEnabled(bool)));

    connect(d->detectBlur, SIGNAL(toggled(bool)),
            d->setBlurWeight, SLOT(setEnabled(bool)));

    connect(d->detectNoise, SIGNAL(toggled(bool)),
            d->lbl6, SLOT(setEnabled(bool)));

    connect(d->detectNoise, SIGNAL(toggled(bool)),
            d->setNoiseWeight, SLOT(setEnabled(bool)));

    connect(d->detectCompression, SIGNAL(toggled(bool)),
            d->lbl7, SLOT(setEnabled(bool)));

    connect(d->detectCompression, SIGNAL(toggled(bool)),
            d->setCompressionWeight, SLOT(setEnabled(bool)));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))

    connect(d->detectButtonGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::idClicked),
            this, &ImageQualityWidget::slotDisableOptionViews);

#else

    connect(d->detectButtonGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
            this, &ImageQualityWidget::slotDisableOptionViews);

#endif

    if (SettingsDisplayMode::BQM == d->displayMode)
    {
        hlay0->hide();
        hbox16->hide();
        setTabVisible(0, false);
        setCurrentIndex(1);
    }

    if (SettingsDisplayMode::Maintenance == d->displayMode)
    {
        hbox16->hide();
        setTabVisible(0, false);
        setCurrentIndex(1);
    }

    if (SettingsDisplayMode::Normal != d->displayMode)
    {
        setTabVisible(0, false);
        setCurrentIndex(1);
    }

    slotDisableOptionViews();
}

void ImageQualityWidget::applySettings()
{
    ImageQualitySettings imq = settings();
    imq.writeToConfig();
}

void ImageQualityWidget::applySettings(KConfigGroup& group)
{
    ImageQualitySettings imq = settings();
    imq.writeToConfig(group);
}

void ImageQualityWidget::readSettings()
{
    ImageQualitySettings imq;
    imq.readFromConfig();
    setSettings(imq);
}

void ImageQualityWidget::readSettings(const KConfigGroup& group)
{
    ImageQualitySettings imq;
    imq.readFromConfig(group);
    setSettings(imq);
}

void ImageQualityWidget::setSettings(const ImageQualitySettings& imq)
{
    d->scanMode->setCurrentIndex(d->scanMode->findData(imq.scanMode));

    // NOTE: Album settings are handled by AlbumSelector widget.

    d->detectBlur->setChecked(imq.detectBlur);
    d->detectNoise->setChecked(imq.detectNoise);
    d->detectCompression->setChecked(imq.detectCompression);
    d->detectExposure->setChecked(imq.detectExposure);

    if (imq.detectAesthetic)
    {
        d->detectAesthetic->setChecked(true);
    }
    else
    {
        d->detectBasicFactors->setChecked(true);
    }

    d->useFullCpuButton->setChecked(imq.useFullCpu);

    d->setRejected->setChecked(imq.lowQRejected);
    d->setPending->setChecked(imq.mediumQPending);
    d->setAccepted->setChecked(imq.highQAccepted);
    d->setRejectedThreshold->setValue(imq.rejectedThreshold);
    d->setPendingThreshold->setValue(imq.pendingThreshold);
    d->setAcceptedThreshold->setValue(imq.acceptedThreshold);
    d->setBlurWeight->setValue(imq.blurWeight);
    d->setNoiseWeight->setValue(imq.noiseWeight);
    d->setCompressionWeight->setValue(imq.compressionWeight);

    d->lbl5->setEnabled(imq.detectBlur);
    d->setBlurWeight->setEnabled(imq.detectBlur);
    d->lbl6->setEnabled(imq.detectNoise);
    d->setNoiseWeight->setEnabled(imq.detectNoise);
    d->lbl7->setEnabled(imq.detectCompression);
    d->setCompressionWeight->setEnabled(imq.detectCompression);

    slotDisableOptionViews();
}

ImageQualitySettings ImageQualityWidget::settings() const
{
    ImageQualitySettings imq;

    if (d->displayMode == SettingsDisplayMode::Normal)
    {
        imq.albums      = d->albumSelectors->selectedAlbumsAndTags();
        imq.wholeAlbums = d->albumSelectors->wholeAlbumsChecked();
    }

    if (SettingsDisplayMode::BQM != d->displayMode)
    {
        imq.scanMode = (ImageQualitySettings::ScanMode)d->scanMode->itemData(d->scanMode->currentIndex()).toInt();
    }

    imq.detectBlur        = d->detectBlur->isChecked();
    imq.detectNoise       = d->detectNoise->isChecked();
    imq.detectCompression = d->detectCompression->isChecked();
    imq.detectExposure    = d->detectExposure->isChecked();
    imq.detectAesthetic   = d->detectAesthetic->isChecked();
    imq.useFullCpu        = d->useFullCpuButton->isChecked();

    imq.lowQRejected      = d->setRejected->isChecked();
    imq.mediumQPending    = d->setPending->isChecked();
    imq.highQAccepted     = d->setAccepted->isChecked();
    imq.rejectedThreshold = d->setRejectedThreshold->value();
    imq.pendingThreshold  = d->setPendingThreshold->value();
    imq.acceptedThreshold = d->setAcceptedThreshold->value();
    imq.blurWeight        = d->setBlurWeight->value();
    imq.noiseWeight       = d->setNoiseWeight->value();
    imq.compressionWeight = d->setCompressionWeight->value();

    return imq;
}

void ImageQualityWidget::slotDisableOptionViews()
{
    d->basicView->setEnabled(d->detectBasicFactors->isChecked());

    d->lbl5->setEnabled(d->detectBlur->isChecked());
    d->setBlurWeight->setEnabled(d->detectBlur->isChecked());
    d->lbl6->setEnabled(d->detectNoise->isChecked());
    d->setNoiseWeight->setEnabled(d->detectNoise->isChecked());
    d->lbl7->setEnabled(d->detectCompression->isChecked());
    d->setCompressionWeight->setEnabled(d->detectCompression->isChecked());

    Q_EMIT signalSettingsChanged();
}

void ImageQualityWidget::resetToDefault()
{
    blockSignals(true);

    ImageQualitySettings prm;
    setSettings(prm);

    blockSignals(false);
}

ImageQualitySettings ImageQualityWidget::defaultSettings() const
{
    ImageQualitySettings prm;

    return prm;
}

} // namespace Digikam

#include "moc_imagequalitywidget.cpp"
