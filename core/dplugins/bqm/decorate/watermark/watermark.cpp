/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-28
 * Description : batch tool to add visible watermark.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Mikkel Baekhoej Christensen <mbc at baekhoej dot dk>
 * SPDX-FileCopyrightText: 2017      by Ahmed Fathi <ahmed dot fathi dot abdelmageed at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "watermark.h"

// Qt includes

#include <QFontMetrics>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QRect>
#include <QWidget>
#include <QGridLayout>
#include <QRadioButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QApplication>
#include <QStyle>
#include <QFontComboBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QRandomGenerator>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dnuminput.h"
#include "dfileselector.h"
#include "digikam_debug.h"
#include "dimg.h"
#include "blurfilter.h"
#include "dfontproperties.h"
#include "metaengine.h"
#include "dcolorselector.h"
#include "dtextedit.h"

namespace DigikamBqmWatermarkPlugin
{

class Q_DECL_HIDDEN WaterMark::Private
{

public:

    enum WaterMarkPositon
    {
        TopLeft = 0,
        TopRight,
        BottomLeft,
        BottomRight,
        Center,
        TopCenter,
        BottomCenter
    };

    enum WaterMarkPlacementType
    {
        SpecifiedLocation = 0,
        SystematicRepetition,
        RandomRepetition
    };

public:

    Private() = default;

    QGroupBox*       textSettingsGroupBox                       = nullptr;
    QGroupBox*       imageSettingsGroupBox                      = nullptr;
    QGroupBox*       useAbsoluteImageSizeGroupBox               = nullptr;
    QRadioButton*    useImageRadioButton                        = nullptr;
    QCheckBox*       ignoreWatermarkAspectCheckBox              = nullptr;
    QCheckBox*       addTransparencyToWatermarkImageCheckBox    = nullptr;
    QCheckBox*       useAbsoluteSizeCheckBox                    = nullptr;
    QCheckBox*       useBackgroundCheckBox                      = nullptr;
    QCheckBox*       denseRepetitionCheckBox                    = nullptr;
    QCheckBox*       randomizeRotationCheckBox                  = nullptr;
    QRadioButton*    useTextRadioButton                         = nullptr;

    DFileSelector*   imageFileUrlRequester                      = nullptr;
    DTextEdit*       textEdit                                   = nullptr;

    QComboBox*       placementPositionComboBox                  = nullptr;
    QComboBox*       placementTypeComboBox                      = nullptr;
    QComboBox*       rotationComboBox                           = nullptr;
    QDoubleSpinBox*  sparsityFactorSpinBox                      = nullptr;
    DFontProperties* extendedFontChooserWidget                  = nullptr;

    DColorSelector*  fontColorButton                            = nullptr;
    DColorSelector*  backgroundColorButton                      = nullptr;

    DIntNumInput*    textOpacity                                = nullptr;
    DIntNumInput*    backgroundOpacity                          = nullptr;
    DIntNumInput*    xMarginInput                               = nullptr;
    DIntNumInput*    yMarginInput                               = nullptr;
    DIntNumInput*    waterMarkSizePercent                       = nullptr;
    DIntNumInput*    waterMarkOpacityPercent                    = nullptr;

    bool             changeSettings                             = true;
};

WaterMark::WaterMark(QObject* const parent)
    : BatchTool(QLatin1String("WaterMark"), DecorateTool, parent),
      d        (new Private)
{
}

WaterMark::~WaterMark()
{
    delete d;
}

BatchTool* WaterMark::clone(QObject* const parent) const
{
    return new WaterMark(parent);
}

void WaterMark::registerSettingsWidget()
{
    const int spacing = layoutSpacing();


    DVBox* const vbox = new DVBox;
    vbox->setContentsMargins(spacing, spacing, spacing, spacing);
    vbox->setSpacing(spacing);

    DHBox* const hbox = new DHBox(vbox);
    hbox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hbox->setSpacing(spacing);

    d->useAbsoluteImageSizeGroupBox                       = new QGroupBox(vbox);
    QVBoxLayout* const useAbsoluteImageSizeGroupBoxLayout = new QVBoxLayout;
    useAbsoluteImageSizeGroupBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    useAbsoluteImageSizeGroupBoxLayout->addStretch(1);
    d->useAbsoluteImageSizeGroupBox->setLayout(useAbsoluteImageSizeGroupBoxLayout);

    DHBox* const useAbsoluteSizeHBox   = new DHBox();
    useAbsoluteSizeHBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    useAbsoluteSizeHBox->setSpacing(spacing);
    d->useAbsoluteSizeCheckBox         = new QCheckBox(useAbsoluteSizeHBox);
    d->useAbsoluteSizeCheckBox->setWhatsThis(i18n("Check this if you want the watermark to use the given size of "
                                                  "the font or the image without any adjustment to the actual image."));
    QLabel* const useAbsoluteSizeLabel = new QLabel(useAbsoluteSizeHBox);
    useAbsoluteSizeLabel->setText(i18n("Use Absolute Size"));
    d->useAbsoluteSizeCheckBox->setChecked(false);

    useAbsoluteImageSizeGroupBoxLayout->addWidget(useAbsoluteSizeHBox);

    QLabel* const watermarkTypeLabel   = new QLabel(hbox);
    watermarkTypeLabel->setText(i18n("Watermark type: "));

    d->useImageRadioButton      = new QRadioButton(hbox);
    d->useImageRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QLabel* const useImageLabel = new QLabel(hbox);
    useImageLabel ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->useTextRadioButton       = new QRadioButton(hbox);
    d->useTextRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QLabel* const useTextLabel  = new QLabel(hbox);
    useTextLabel ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    useImageLabel->setText(i18n("Image"));
    useTextLabel->setText(i18n("Text"));

    useImageLabel->setAlignment(Qt::AlignLeft);
    useTextLabel->setAlignment(Qt::AlignLeft);
    d->useImageRadioButton->setChecked(true);

    // ---

    d->imageSettingsGroupBox                       = new QGroupBox(vbox);
    d->imageSettingsGroupBox->setTitle(i18n("Image settings"));
    QVBoxLayout* const imageSettingsGroupBoxLayout = new QVBoxLayout;
    imageSettingsGroupBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    imageSettingsGroupBoxLayout->addStretch(1);
    d->imageSettingsGroupBox->setLayout(imageSettingsGroupBoxLayout);

    DHBox* const imageSelectHBox = new DHBox();
    QLabel* const label          = new QLabel(imageSelectHBox);
    d->imageFileUrlRequester     = new DFileSelector(imageSelectHBox);
    d->imageFileUrlRequester->lineEdit()->setPlaceholderText(i18n("Click to select watermark image."));
    label->setText(i18n("Watermark image: "));
    imageSettingsGroupBoxLayout->addWidget(imageSelectHBox);

    DHBox* const ignoreWatermarkAspectRatioHBox   = new DHBox();
    ignoreWatermarkAspectRatioHBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ignoreWatermarkAspectRatioHBox->setSpacing(spacing);

    d->ignoreWatermarkAspectCheckBox              = new QCheckBox(ignoreWatermarkAspectRatioHBox);
    d->ignoreWatermarkAspectCheckBox->setWhatsThis(i18n("Check this if you want the watermark to ignore its own "
                                                        "aspect ratio and use the image's aspect ratio instead."));

    QLabel* const ignoreWatermarkAspectRatioLabel = new QLabel(ignoreWatermarkAspectRatioHBox);
    ignoreWatermarkAspectRatioLabel->setText(i18n("Ignore Watermark aspect Ratio"));
    d->ignoreWatermarkAspectCheckBox->setChecked(false);
    imageSettingsGroupBoxLayout->addWidget(ignoreWatermarkAspectRatioHBox);

    DHBox* const addTransparencyToWatermarkImageHBox = new DHBox();
    addTransparencyToWatermarkImageHBox->setContentsMargins(QMargins());
    addTransparencyToWatermarkImageHBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    addTransparencyToWatermarkImageHBox->setSpacing(spacing);

    d->addTransparencyToWatermarkImageCheckBox       = new QCheckBox(addTransparencyToWatermarkImageHBox);
    d->addTransparencyToWatermarkImageCheckBox->setWhatsThis(i18n("Check this if you want watermark to be transparent."));
    d->addTransparencyToWatermarkImageCheckBox->setChecked(false);

    QLabel* const addTransparencyToWatermarkLabel    = new QLabel(addTransparencyToWatermarkImageHBox);
    addTransparencyToWatermarkLabel->setText(i18n("Add transparency to watermark image"));
    imageSettingsGroupBoxLayout->addWidget(addTransparencyToWatermarkImageHBox);

    DHBox* const watermarkImageOpacityBox = new DHBox();
    watermarkImageOpacityBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    watermarkImageOpacityBox->setSpacing(spacing);

    QLabel* const opacityLabel            = new QLabel(watermarkImageOpacityBox);
    d->waterMarkOpacityPercent            = new DIntNumInput(watermarkImageOpacityBox);
    d->waterMarkOpacityPercent->setRange(0, 100, 1);
    d->waterMarkOpacityPercent->setDefaultValue(100);
    d->waterMarkOpacityPercent->setWhatsThis(i18n("Opacity of watermark"));
    opacityLabel->setText(i18n("Opacity (%): "));
    imageSettingsGroupBoxLayout->addWidget(watermarkImageOpacityBox);

    // ---

    d->textSettingsGroupBox                       = new QGroupBox(vbox);
    d->textSettingsGroupBox->setTitle(i18n("Text settings"));
    QVBoxLayout* const textSettingsGroupBoxLayout = new QVBoxLayout;
    textSettingsGroupBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    textSettingsGroupBoxLayout->addStretch(1);
    d->textSettingsGroupBox->setLayout(textSettingsGroupBoxLayout);

    QLabel* const textEditLabel = new QLabel(vbox);
    d->textEdit                 = new DTextEdit(vbox);
    d->textEdit->setPlaceholderText(i18n("Enter your watermark string here."));
    textEditLabel->setText(i18n("Watermark text: "));
    textSettingsGroupBoxLayout->addWidget(textEditLabel);
    textSettingsGroupBoxLayout->addWidget(d->textEdit);

    d->extendedFontChooserWidget = new DFontProperties(nullptr, DFontProperties::NoDisplayFlags);
    d->extendedFontChooserWidget->setSampleBoxVisible(true);
    d->extendedFontChooserWidget->enableColumn(0x04, false);
    d->extendedFontChooserWidget->setWhatsThis(i18n("Choose the font type and style. Size is auto calculated."));
    textSettingsGroupBoxLayout->addWidget(d->extendedFontChooserWidget);

    DHBox* const fontColorBox = new DHBox();
    QLabel* const label3 = new QLabel(fontColorBox);
    d->fontColorButton   = new DColorSelector(fontColorBox);
    d->fontColorButton->setColor(Qt::black);
    d->fontColorButton->setWhatsThis(i18n("Set the font color to use here."));
    label3->setText(i18n("Font color: "));
    textSettingsGroupBoxLayout->addWidget(fontColorBox);

    DHBox* const fontOpacityBox    = new DHBox();
    QLabel* const textOpacityLabel = new QLabel(fontOpacityBox);
    textOpacityLabel->setText(i18n("Text opacity: "));
    d->textOpacity                 = new DIntNumInput(fontOpacityBox);
    d->textOpacity->setRange(0, 100, 1);
    d->textOpacity->setDefaultValue(100);
    d->textOpacity->setWhatsThis(i18n("Set the opacity of the watermark text. "
                                      "100 is fully opaque, 0 is fully transparent."));
    textSettingsGroupBoxLayout->addWidget(fontOpacityBox);

    DHBox* const useBackgroundHBox   = new DHBox();
    useBackgroundHBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    useBackgroundHBox->setSpacing(spacing);

    d->useBackgroundCheckBox         = new QCheckBox(useBackgroundHBox);
    d->useBackgroundCheckBox->setWhatsThis(i18n("Check this if you want a background fill behind the text."));
    QLabel* const useBackgroundLabel = new QLabel(useBackgroundHBox);
    useBackgroundLabel->setText(i18n("Use background"));
    textSettingsGroupBoxLayout->addWidget(useBackgroundHBox);

    DHBox* const useBackgroundColorHBox = new DHBox();
    QLabel* const backgroundColorLabel  = new QLabel(useBackgroundColorHBox);
    d->backgroundColorButton            = new DColorSelector(useBackgroundColorHBox);
    d->backgroundColorButton->setColor(QColor(0xCC, 0xCC, 0xCC));
    d->backgroundColorButton->setWhatsThis(i18n("Choose the color of the watermark background."));
    backgroundColorLabel ->setText(i18n("Background color: "));
    textSettingsGroupBoxLayout->addWidget(useBackgroundColorHBox);

    DHBox* const useBackgroundOpacityHBox  = new DHBox();
    QLabel* const backgroundOpacityLabel   = new QLabel(useBackgroundOpacityHBox);
    backgroundOpacityLabel->setText(i18n("Background opacity: "));
    d->backgroundOpacity                   = new DIntNumInput(useBackgroundOpacityHBox);
    d->backgroundOpacity->setRange(0, 100, 1);
    d->backgroundOpacity->setDefaultValue(100);
    d->backgroundOpacity->setWhatsThis(i18n("Set the opacity of the watermark background. "
                                            "100 is fully opaque, 0 is fully transparent."));
    textSettingsGroupBoxLayout->addWidget(useBackgroundOpacityHBox);

    d->imageSettingsGroupBox->setVisible(true);
    d->textSettingsGroupBox->setVisible(false);

    // ---

    DHBox* const placementTypeHBox = new DHBox(vbox);
    QLabel* const placementTypeLabel = new QLabel(placementTypeHBox);
    d->placementTypeComboBox         = new QComboBox(placementTypeHBox);
    d->placementTypeComboBox->insertItem(Private::SpecifiedLocation,    i18n("Specific Location"));
    d->placementTypeComboBox->insertItem(Private::SystematicRepetition, i18n("Systematic Repetition"));
    d->placementTypeComboBox->insertItem(Private::RandomRepetition,     i18n("Random Repetition"));
    placementTypeLabel->setText(i18n("Placement Type: "));

    DHBox* const placementHBox = new DHBox(vbox);
    placementHBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    placementHBox->setSpacing(spacing);

    QLabel* const spaceLabel   = new QLabel(placementHBox);
    d->denseRepetitionCheckBox = new QCheckBox(placementHBox);
    d->denseRepetitionCheckBox->setWhatsThis(i18n("When you choose to have the watermark repeated many times in the "
                                                  "placement combo box, you can specify here whether the repetition."));
    d->denseRepetitionCheckBox->setChecked(false);
    d->denseRepetitionCheckBox->setEnabled(false);
    spaceLabel->setText(QLatin1String(" "));
    QLabel* const placementDensityLabel = new QLabel(placementHBox);
    placementDensityLabel->setText(i18n("Density of watermark repetition (Disabled in \"Specific Location\" mode)"));

    DHBox* const randomizeHBox          = new DHBox(vbox);
    randomizeHBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    randomizeHBox->setSpacing(spacing);

    QLabel* const spaceLabel2           = new QLabel(randomizeHBox);
    d->randomizeRotationCheckBox        = new QCheckBox(randomizeHBox);
    d->randomizeRotationCheckBox->setWhatsThis(i18n("When you choose to have the watermark repeated randomly in "
                                                    "the placement combo box, you can specify here to randomly "
                                                    "rotate the watermark (0, 90, 180, 270 degrees)."));
    d->randomizeRotationCheckBox->setChecked(true);
    d->denseRepetitionCheckBox->setEnabled(false);
    spaceLabel2->setText(QLatin1String(" "));
    QLabel* const randomizeRotation   = new QLabel(randomizeHBox);
    randomizeRotation->setText(i18n("Randomize watermark orientation (Enabled in \"Random Repetition\" mode only)"));

    DHBox* const sparsityHBox         = new DHBox(vbox);
    sparsityHBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sparsityHBox->setSpacing(spacing);

    QLabel* const sparsityFactorLabel = new QLabel(sparsityHBox);
    d->sparsityFactorSpinBox          = new QDoubleSpinBox(sparsityHBox);
    d->sparsityFactorSpinBox->setMinimum(0.0);
    d->sparsityFactorSpinBox->setValue(1);
    d->sparsityFactorSpinBox->setSingleStep(0.1);
    d->sparsityFactorSpinBox->setWhatsThis(i18n("Use this to get more control over the sparsity of watermark "
                                           "repetition. The higher the value the sparser the watermarks get. "
                                           "Use floating point values, typically between 1.0 and 3.0. It can "
                                           "also be less than 1.0"));
    sparsityFactorLabel->setText(i18n("Sparsity Factor: "));

    DHBox* const placementPositionHBox = new DHBox(vbox);
    QLabel* const label4         = new QLabel(placementPositionHBox);
    d->placementPositionComboBox = new QComboBox(placementPositionHBox);
    d->placementPositionComboBox->insertItem(Private::TopLeft,      i18nc("@item: placement position", "Top left"));
    d->placementPositionComboBox->insertItem(Private::TopRight,     i18nc("@item: placement position", "Top right"));
    d->placementPositionComboBox->insertItem(Private::BottomLeft,   i18nc("@item: placement position", "Bottom left"));
    d->placementPositionComboBox->insertItem(Private::BottomRight,  i18nc("@item: placement position", "Bottom right"));
    d->placementPositionComboBox->insertItem(Private::Center,       i18nc("@item: placement position", "Center"));
    d->placementPositionComboBox->insertItem(Private::TopCenter,    i18nc("@item: placement position", "Top center"));
    d->placementPositionComboBox->insertItem(Private::BottomCenter, i18nc("@item: placement position", "Bottom center"));

    label4->setText(i18n("Placement Position: "));

    DHBox* const placementRotationHBox = new DHBox(vbox);
    QLabel* const labelRotation  = new QLabel(placementRotationHBox);
    d->rotationComboBox          = new QComboBox(placementRotationHBox);
    d->rotationComboBox->insertItem(0, i18n("0 degrees"));
    d->rotationComboBox->insertItem(1, i18n("90 degrees CW"));
    d->rotationComboBox->insertItem(2, i18n("180 degrees"));
    d->rotationComboBox->insertItem(3, i18n("270 degrees CW"));
    labelRotation->setText(i18n("Rotation: "));

    DHBox* const placementSizeHBox = new DHBox(vbox);
    QLabel* const label5    = new QLabel(placementSizeHBox);
    d->waterMarkSizePercent = new DIntNumInput(placementSizeHBox);
    d->waterMarkSizePercent->setRange(0, 100, 1);
    d->waterMarkSizePercent->setDefaultValue(30);
    d->waterMarkSizePercent->setWhatsThis(i18n("Size of watermark, as a percentage of the marked image."));
    label5->setText(i18n("Size (%): "));

    DHBox* const placementXMarginHBox = new DHBox(vbox);
    QLabel* const label6 = new QLabel(placementXMarginHBox);
    d->xMarginInput      = new DIntNumInput(placementXMarginHBox);
    d->xMarginInput->setRange(0, 100, 1);
    d->xMarginInput->setDefaultValue(2);
    d->xMarginInput->setWhatsThis(i18n("Margin from edge in X direction, as a percentage of the marked image."));
    label6->setText(i18n("X margin (%): "));

    DHBox* const placementYMarginHBox = new DHBox(vbox);
    QLabel* const label7 = new QLabel(placementYMarginHBox);
    d->yMarginInput      = new DIntNumInput(placementYMarginHBox);
    d->yMarginInput->setRange(0, 100, 1);
    d->yMarginInput->setDefaultValue(2);
    d->yMarginInput->setWhatsThis(i18n("Margin from edge in Y direction, as a percentage of the marked image."));
    label7->setText(i18n("Y margin (%): "));

    QLabel* const space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    m_settingsWidget    = vbox;

    // ------------------------------------------------------------------------------------------------------

    connect(d->useImageRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useTextRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->imageFileUrlRequester->lineEdit(), SIGNAL(textChanged(QString)),
            this, SLOT(slotSettingsChanged()));

    connect(d->extendedFontChooserWidget, SIGNAL(fontSelected(QFont)),
            this, SLOT(slotSettingsChanged()));

    connect(d->fontColorButton, SIGNAL(signalColorSelected(QColor)),
            this, SLOT(slotSettingsChanged()));

    connect(d->textEdit, SIGNAL(textChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->textOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useBackgroundCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->ignoreWatermarkAspectCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->addTransparencyToWatermarkImageCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useAbsoluteSizeCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->backgroundColorButton, SIGNAL(signalColorSelected(QColor)),
            this, SLOT(slotSettingsChanged()));

    connect(d->backgroundOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->placementTypeComboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->placementPositionComboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->denseRepetitionCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->randomizeRotationCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->sparsityFactorSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(slotSettingsChanged()));

    connect(d->rotationComboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->waterMarkSizePercent, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->yMarginInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->xMarginInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->waterMarkOpacityPercent, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings WaterMark::defaultSettings()
{
    BatchToolSettings settings;

    settings.insert(QLatin1String("Use image"),          true);
    settings.insert(QLatin1String("Watermark image"),    QString());
    settings.insert(QLatin1String("Text"),               QString());
    settings.insert(QLatin1String("Font"),               QFont());
    settings.insert(QLatin1String("Color"),              QColor(Qt::black));
    settings.insert(QLatin1String("Text opacity"),       100);
    settings.insert(QLatin1String("Use background"),     true);
    settings.insert(QLatin1String("Background color"),   QColor(0xCC, 0xCC, 0xCC));
    settings.insert(QLatin1String("Background opacity"), 0xCC);
    settings.insert(QLatin1String("PlacementType"),      Private::SpecifiedLocation); // specified location for the watermark
    settings.insert(QLatin1String("Dense Repetition"),   false);
    settings.insert(QLatin1String("Randomize Rotation"), true);
    settings.insert(QLatin1String("Sparsity Factor"),    1.0);
    settings.insert(QLatin1String("Placement"),          Private::BottomRight);
    settings.insert(QLatin1String("Rotation"),           0);
    settings.insert(QLatin1String("Watermark size"),     25);
    settings.insert(QLatin1String("X margin"),           2);
    settings.insert(QLatin1String("Y margin"),           2);
    settings.insert(QLatin1String("Opacity"),            100);

    return settings;
}

void WaterMark::slotAssignSettings2Widget()
{
    d->changeSettings = false;
    d->useImageRadioButton->setChecked(settings().value(QLatin1String("Use image")).toBool());
    d->useTextRadioButton->setChecked(!settings().value(QLatin1String("Use image")).toBool());
    d->imageFileUrlRequester->setFileDlgPath(settings().value(QLatin1String("Watermark image")).toString());

    QString txt       = settings().value(QLatin1String("Text")).toString();

    if (d->textEdit->text() != txt)
    {
        d->textEdit->setText(txt);
    }

    d->extendedFontChooserWidget->setFont(qvariant_cast<QFont>(settings().value(QLatin1String("Font"))));
    d->fontColorButton->setColor(settings().value(QLatin1String("Color")).toString());
    d->textOpacity->setValue(settings().value(QLatin1String("Text opacity")).toInt());
    d->useBackgroundCheckBox->setChecked(settings().value(QLatin1String("Use background")).toBool());
    d->backgroundColorButton->setColor(settings().value(QLatin1String("Background color")).toString());
    d->backgroundOpacity->setValue(settings().value(QLatin1String("Background opacity")).toInt());
    d->placementPositionComboBox->setCurrentIndex(settings().value(QLatin1String("PlacementType")).toInt());
    d->denseRepetitionCheckBox->setChecked(settings().value(QLatin1String("Dense Repetition")).toBool());
    d->randomizeRotationCheckBox->setChecked(settings().value(QLatin1String("Randomize Rotation")).toBool());
    d->sparsityFactorSpinBox->setValue(settings().value(QLatin1String("Sparsity Factor")).toDouble());
    d->placementPositionComboBox->setCurrentIndex(settings().value(QLatin1String("Placement")).toInt());
    d->rotationComboBox->setCurrentIndex(settings().value(QLatin1String("Rotation")).toInt());
    d->waterMarkSizePercent->setValue(settings().value(QLatin1String("Watermark size")).toInt());
    d->xMarginInput->setValue(settings().value(QLatin1String("X margin")).toInt());
    d->yMarginInput->setValue(settings().value(QLatin1String("Y margin")).toInt());
    d->waterMarkOpacityPercent->setValue(settings().value(QLatin1String("Opacity")).toInt());
    d->changeSettings = true;
}

void WaterMark::slotSettingsChanged()
{
    if      (d->useImageRadioButton->isChecked())
    {
        d->textSettingsGroupBox->setVisible(false);
        d->imageSettingsGroupBox->setVisible(true);
    }
    else if (d->useTextRadioButton->isChecked())
    {
        d->imageSettingsGroupBox->setVisible(false);
        d->textSettingsGroupBox->setVisible(true);
    }

    d->waterMarkSizePercent->setEnabled(!d->useAbsoluteSizeCheckBox->isChecked());
    d->extendedFontChooserWidget->enableColumn(0x04, d->useAbsoluteSizeCheckBox->isChecked());
    d->placementPositionComboBox->setEnabled(((int)d->placementTypeComboBox->currentIndex() == Private::SpecifiedLocation));
    d->denseRepetitionCheckBox->setEnabled(((int)d->placementTypeComboBox->currentIndex() != Private::SpecifiedLocation));
    d->sparsityFactorSpinBox->setEnabled(((int)d->placementTypeComboBox->currentIndex() != Private::SpecifiedLocation));
    d->randomizeRotationCheckBox->setEnabled(((int)d->placementTypeComboBox->currentIndex() == Private::RandomRepetition));
    d->rotationComboBox->setEnabled(!(d->randomizeRotationCheckBox->isEnabled() && d->randomizeRotationCheckBox->isChecked()));
    d->waterMarkOpacityPercent->setEnabled(d->addTransparencyToWatermarkImageCheckBox->isChecked());

    if (d->changeSettings)
    {
        BatchToolSettings settings;
        settings.insert(QLatin1String("Use image"),                     d->useImageRadioButton->isChecked());
        settings.insert(QLatin1String("Watermark image"),               d->imageFileUrlRequester->fileDlgPath());
        settings.insert(QLatin1String("Text"),                          d->textEdit->text());
        settings.insert(QLatin1String("Font"),                          d->extendedFontChooserWidget->font());
        settings.insert(QLatin1String("Color"),                         d->fontColorButton->color());
        settings.insert(QLatin1String("Text opacity"),                  d->textOpacity->value());
        settings.insert(QLatin1String("Use background"),                d->useBackgroundCheckBox->isChecked());
        settings.insert(QLatin1String("Ignore Watermark Aspect Ratio"), d->ignoreWatermarkAspectCheckBox->isChecked());
        settings.insert(QLatin1String("Use Absolute Size"),             d->useAbsoluteSizeCheckBox->isChecked());
        settings.insert(QLatin1String("Background color"),              d->backgroundColorButton->color());
        settings.insert(QLatin1String("Background opacity"),            d->backgroundOpacity->value());
        settings.insert(QLatin1String("Dense Repetition"),              d->denseRepetitionCheckBox->isChecked());
        settings.insert(QLatin1String("Randomize Rotation"),            d->randomizeRotationCheckBox->isChecked());
        settings.insert(QLatin1String("Sparsity Factor"),               (double)d->sparsityFactorSpinBox->value());
        settings.insert(QLatin1String("PlacementType"),                 (int)d->placementTypeComboBox->currentIndex());
        settings.insert(QLatin1String("Placement"),                     (int)d->placementPositionComboBox->currentIndex());
        settings.insert(QLatin1String("Rotation"),                      (int)d->rotationComboBox->currentIndex());
        settings.insert(QLatin1String("Watermark size"),                (int)d->waterMarkSizePercent->value());
        settings.insert(QLatin1String("X margin"),                      (int)d->xMarginInput->value());
        settings.insert(QLatin1String("Y margin"),                      (int)d->yMarginInput->value());
        settings.insert(QLatin1String("Opacity"),                       (int)d->waterMarkOpacityPercent->value());
        settings.insert(QLatin1String("Add transparency"),              d->addTransparencyToWatermarkImageCheckBox->isChecked());
        BatchTool::slotSettingsChanged(settings);
    }
}

bool WaterMark::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    QString fileName                             = settings().value(QLatin1String("Watermark image")).toString();
    int placementPosition                        = settings().value(QLatin1String("Placement")).toInt();
    int placementType                            = settings().value(QLatin1String("PlacementType")).toInt();
    bool denseRepetition                         = settings().value(QLatin1String("Dense Repetition")).toBool();
    bool randomizeRotation                       = settings().value(QLatin1String("Randomize Rotation")).toBool();
    double userSparsityFactor                    = settings().value(QLatin1String("Sparsity Factor")).toDouble();
    int size                                     = settings().value(QLatin1String("Watermark size")).toInt();
    int xMargin                                  = settings().value(QLatin1String("X margin")).toInt();
    int yMargin                                  = settings().value(QLatin1String("Y margin")).toInt();
    bool useImage                                = settings().value(QLatin1String("Use image")).toBool();
    QString text                                 = settings().value(QLatin1String("Text")).toString();
    QFont font                                   = qvariant_cast<QFont>(settings().value(QLatin1String("Font")));

    QColor fontColor                             = settings().value(QLatin1String("Color")).toString();
    int textOpacity                              = settings().value(QLatin1String("Text opacity")).toInt();
    bool useBackground                           = settings().value(QLatin1String("Use background")).toBool();
    QColor backgroundColor                       = settings().value(QLatin1String("Background color")).toString();
    int backgroundOpacity                        = settings().value(QLatin1String("Background opacity")).toInt();
    Qt::AspectRatioMode watermarkAspectRatioMode = settings().value(QLatin1String("Ignore Watermark Aspect Ratio")).toBool() ?
                                                              Qt::IgnoreAspectRatio : Qt::KeepAspectRatio;
    bool useAbsoluteSize                         = settings().value(QLatin1String("Use Absolute Size")).toBool();

    DImg watermarkImage;
    DColorComposer* const composer               = DColorComposer::getComposer(DColorComposer::PorterDuffNone);
    int marginW                                  = lround(image().width()  * (xMargin / 100.0));
    int marginH                                  = lround(image().height() * (yMargin / 100.0));
    int rotationIndex = settings().value(QLatin1String("Rotation")).toInt();
    double opacity                               = settings().value(QLatin1String("Opacity")).toInt()/100.0;
    bool addTransparency                         = settings().value(QLatin1String("Add transparency")).toBool();

    DImg::ANGLE rotationAngle = (rotationIndex == 1) ? DImg::ANGLE::ROT90  :
                                (rotationIndex == 2) ? DImg::ANGLE::ROT180 :
                                (rotationIndex == 3) ? DImg::ANGLE::ROT270 :
                                                       DImg::ANGLE::ROTNONE;

    // rotate and/or flip the image depending on the exif information to allow for the expected watermark placement.
    // note that this operation is reversed after proper watermark generation to leave everything as it was.

    image().exifRotate(inputUrl().toLocalFile());

    float ratio = (float)image().height() / image().width();

    if ((rotationAngle == DImg::ANGLE::ROT90) || (rotationAngle == DImg::ANGLE::ROT270))
    {
        size = size * ratio;
    }
    else
    {
        // For Images whose height are much larger than their width, this helps keep
        // the watermark size reasonable

        if (ratio > 1.5)
        {
            int tempSize  = size * ratio;

            if (tempSize < 35)
            {
                tempSize *= 1.5;
            }

            size          = (tempSize < 100) ? tempSize : 100;
        }
    }

    if (useImage)
    {
        watermarkImage = DImg(fileName);

        if (watermarkImage.isNull())
        {
            return false;
        }

        if (!useAbsoluteSize)
        {
            DImg tempImage = watermarkImage.smoothScale(image().width()  * size / 100,
                                                        image().height() * size / 100,
                                                        watermarkAspectRatioMode);
            watermarkImage = tempImage;
        }

        if (addTransparency)
        {
            if (!watermarkImage.hasAlpha())
            {
                return false;
            }

            for (uint i = 0 ; i < watermarkImage.width() ; ++i)
            {
                for (uint j = 0 ; j < watermarkImage.height() ; ++j)
                {
                    DColor color = watermarkImage.getPixelColor(i, j);
                    color.setAlpha((int)(opacity*color.alpha()));
                    watermarkImage.setPixelColor(i, j, color);
                }
            }
        }
    }
    else
    {
        int alignMode;
        const int radius = 10;

        if (text.isEmpty())
        {
            return false;
        }

        int fontSize = queryFontSize(text, font, size);

        if (fontSize == 0)
        {
            return false;
        }

        switch (placementPosition)
        {
            case Private::TopLeft:
            {
                alignMode = Qt::AlignLeft;
                break;
            }

            case Private::TopRight:
            {
                alignMode = Qt::AlignRight;
                break;
            }

            case Private::BottomLeft:
            {
                alignMode = Qt::AlignLeft;
                break;
            }

            case Private::Center:
            case Private::TopCenter:
            case Private::BottomCenter:
            {
                alignMode = Qt::AlignCenter;
                break;
            }

            default:    // BottomRight
            {
                alignMode = Qt::AlignRight;
                break;
            }
        }

        if (!useAbsoluteSize)
        {
            font.setPointSizeF(fontSize);
        }

        QFontMetrics fontMt(font);
        QRect fontRect = fontMt.boundingRect(radius, radius, image().width(), image().height(), 0, text);

        // Add a transparent layer.

        QRect backgroundRect(fontRect.x() - radius, fontRect.y() - radius,
                             fontRect.width() + 2 * radius, fontRect.height() + 2 * radius);
        DImg backgroundLayer(backgroundRect.width(), backgroundRect.height(), image().sixteenBit(), true);
        DColor transparent(QColor(0, 0, 0));
        transparent.setAlpha(0);

        if (image().sixteenBit())
        {
            transparent.convertToSixteenBit();
        }

        backgroundLayer.fill(transparent);

        DImg grayTransLayer(fontRect.width(), fontRect.height(), image().sixteenBit(), true);

        if (useBackground)
        {
            DColor grayTrans(backgroundColor);
            grayTrans.setAlpha(backgroundOpacity * 255 / 100);

            if (image().sixteenBit())
            {
                grayTrans.convertToSixteenBit();
            }

            grayTransLayer.fill(grayTrans);
            backgroundLayer.bitBlendImage(composer, &grayTransLayer, 0, 0,
                                          grayTransLayer.width(), grayTransLayer.height(),
                                          radius, radius);
        }

        BlurFilter blur(&backgroundLayer, nullptr, radius);
        blur.startFilterDirectly();
        backgroundLayer.putImageData(blur.getTargetImage().bits());

        // Draw text

        QImage img = backgroundLayer.copyQImage(fontRect);
        QPainter p(&img);
        fontColor.setAlpha(textOpacity * 255 / 100);
        p.setPen(QPen(fontColor, 1));
        p.setFont(font);
        p.save();
        p.drawText(0, 0, fontRect.width(), fontRect.height(), alignMode, text);
        p.restore();
        p.end();

        watermarkImage = DImg(img);
    }

    watermarkImage.convertToDepthOfImage(&image());

    QRect watermarkRect(0, 0, watermarkImage.width(), watermarkImage.height());

    int xAdditionalValue = 0;
    int yAdditionalValue = 0;

    watermarkImage.rotate(rotationAngle);

    if (placementType == Private::SpecifiedLocation)
    {
        switch (placementPosition)
        {
            case Private::TopLeft:
            {
                watermarkRect.moveTopLeft(QPoint(marginW, marginH));
                break;
            }

            case Private::TopRight:
            {
                if ((rotationAngle == DImg::ANGLE::ROT270) || (rotationAngle == DImg::ANGLE::ROT90))
                {
                    xAdditionalValue += watermarkRect.width() - watermarkRect.height();
                }

                watermarkRect.moveTopRight(QPoint(image().width() + xAdditionalValue - marginW - 1,
                                                  marginH));
                break;
            }

            case Private::BottomLeft:
            {
                if ((rotationAngle == DImg::ANGLE::ROT90) || (rotationAngle == DImg::ANGLE::ROT270))
                {
                    yAdditionalValue += watermarkRect.height() - watermarkRect.width();
                }

                watermarkRect.moveBottomLeft(QPoint(marginW,
                                                    image().height() + yAdditionalValue - marginH - 1));
                break;
            }

            case Private::Center:
            {
                if ((rotationAngle == DImg::ANGLE::ROT90) || (rotationAngle == DImg::ANGLE::ROT270))
                {
                    xAdditionalValue += (watermarkRect.width()  - watermarkRect.height()) / 2;
                    yAdditionalValue += (watermarkRect.height() - watermarkRect.width())  / 2;
                }

                watermarkRect.moveCenter(QPoint((int)(image().width()  / 2 + xAdditionalValue),
                                                (int)(image().height() / 2 + yAdditionalValue)));
                break;
            }

            case Private::TopCenter:
            {
                if ((rotationAngle == DImg::ANGLE::ROT90) || (rotationAngle == DImg::ANGLE::ROT270))
                {
                    xAdditionalValue += (watermarkRect.width() - watermarkRect.height()) / 2;
                }

                watermarkRect.moveCenter(QPoint((int)(image().width()   / 2  + xAdditionalValue),
                                                (watermarkRect.height() / 2) + marginH));
                break;
            }

            case Private::BottomCenter:
            {
                if ((rotationAngle == DImg::ANGLE::ROT90) || (rotationAngle == DImg::ANGLE::ROT270))
                {
                    xAdditionalValue += (watermarkRect.width() - watermarkRect.height()) / 2;
                    yAdditionalValue += watermarkRect.height() - watermarkRect.width();
                }

                watermarkRect.moveCenter(QPoint((int)(image().width()   / 2  + xAdditionalValue),
                                                image().height() + yAdditionalValue -
                                                (watermarkRect.height() / 2) - marginH - 1));
                break;
            }

            default:    // BottomRight
            {
                if ((rotationAngle == DImg::ANGLE::ROT90) || (rotationAngle == DImg::ANGLE::ROT270))
                {
                    xAdditionalValue += watermarkRect.width()  - watermarkRect.height();
                    yAdditionalValue += watermarkRect.height() - watermarkRect.width();
                }

                watermarkRect.moveBottomRight(QPoint(image().width()  + xAdditionalValue - marginW - 1,
                                                     image().height() + yAdditionalValue - marginH - 1));
                break;
            }
        }

        image().bitBlendImage(composer,
                              &watermarkImage, 0, 0, watermarkImage.width(), watermarkImage.height(),
                              watermarkRect.left(), watermarkRect.top());
    }
    else
    {
        const float DENSE_SPACING_FACTOR  = 1.2F;
        const float SPARSE_SPACING_FACTOR = 1.8F;
        float widthRatio                  = (float)watermarkRect.width()  / image().width();
        float heightRatio                 = (float)watermarkRect.height() / image().height();
        float spacingFactor               = (denseRepetition) ? DENSE_SPACING_FACTOR : SPARSE_SPACING_FACTOR;
        spacingFactor                    *= userSparsityFactor;

        if      (placementType == Private::SystematicRepetition)
        {
            if ((rotationAngle == DImg::ANGLE::ROT270) || (rotationAngle == DImg::ANGLE::ROT90))
            {
                widthRatio  = (float)watermarkRect.height() / image().width();
                heightRatio = (float)watermarkRect.width()  / image().height();
            }

            for (uint i = 0 ; i < image().width() ; i += spacingFactor * widthRatio * image().width())
            {
                for (uint j = 0 ; j < image().height() ; j += spacingFactor * heightRatio * image().height())
                {
                    image().bitBlendImage(composer, &watermarkImage, 0, 0, watermarkImage.width(), watermarkImage.height(), i, j);
                }
            }
        }
        else if (placementType == Private::RandomRepetition)
        {
            widthRatio  = (widthRatio > heightRatio) ? widthRatio : heightRatio;
            heightRatio = widthRatio;

            auto randomGenerator = QRandomGenerator::global();

            for (uint i = 0 ; i < image().width() ; i += spacingFactor * widthRatio * image().width())
            {
                for (uint j = 0 ; j < image().height() ; j += spacingFactor * heightRatio * image().height())
                {
                    int number = (denseRepetition) ? 2 : 3;

                    if (randomGenerator->bounded(number) == 0)
                    {
                        if (randomizeRotation)
                        {
                            int x = randomGenerator->bounded(4);
                            watermarkImage.rotate((DImg::ANGLE)x);
                        }

                        image().bitBlendImage(composer, &watermarkImage, 0, 0, watermarkImage.width(), watermarkImage.height(), i, j);
                    }
                }
            }
        }
    }

    // TODO: Create watermark filter, move code there, implement FilterAction

    delete composer;
    image().reverseExifRotate(inputUrl().toLocalFile());

    return (savefromDImg());
}

int WaterMark::queryFontSize(const QString& text, const QFont& font, int length) const
{
    // Find font size using relative length compared to image width.

    QFont fnt = font;
    QRect fontRect;

    for (int i = 1 ; i <= 1000 ; ++i)
    {
        fnt.setPointSizeF(i);
        QFontMetrics fontMt(fnt);
        fontRect = fontMt.boundingRect(0, 0, image().width(), image().height(), 0, text);

        if (fontRect.width() > lround((image().width() * length) / 100.0))
        {
            return (i - 1);
        }
    }

    return 0;
}

} // namespace DigikamBqmWatermarkPlugin

#include "moc_watermark.cpp"
