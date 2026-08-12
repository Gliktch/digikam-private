/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-17
 * Description : resize image batch tool.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "resize.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QSize>
#include <QWidget>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dnuminput.h"
#include "dimg.h"
#include "dimgbuiltinfilter.h"

namespace DigikamBqmResizePlugin
{

class Q_DECL_HIDDEN Resize::Private
{
public:

    enum WidthPreset
    {
        Tiny = 0,
        Small,
        Medium,
        Big,
        Large,
        Huge,
        QXGA,
        QHD,
        UHD
    };

public:

    Private() = default;

    int presetLengthValue(WidthPreset preset);

public:

    QLabel*       labelPreset       = nullptr;

    QCheckBox*    useCustom         = nullptr;
    QCheckBox*    scaleDown         = nullptr;
    QCheckBox*    usePercent        = nullptr;

    DIntNumInput* customLength      = nullptr;

    QComboBox*    presetCBox        = nullptr;

    bool          changeSettings    = true;
};

int Resize::Private::presetLengthValue(WidthPreset preset)
{
    int length;

    switch (preset)
    {
        case Private::Tiny:
        {
            length = 480;
            break;
        }

        case Private::Small:
        {
            length = 640;
            break;
        }

        case Private::Medium:
        {
            length = 800;
            break;
        }

        case Private::Big:
        {
            length = 1024;
            break;
        }

        case Private::Large:
        {
            length = 1280;
            break;
        }

        case Private::Huge:
        {
            length = 1600;
            break;
        }

        case Private::QXGA:
        {
            length = 2048;
            break;
        }

        case Private::QHD:
        {
            length = 2560;
            break;
        }

        default:   // Private::UHD
        {
            length = 3840;
            break;
        }
    }

    return length;
}

// ------------------------------------------------------------------------------

Resize::Resize(QObject* const parent)
    : BatchTool(QLatin1String("Resize"), TransformTool, parent),
      d        (new Private)
{
}

Resize::~Resize()
{
    delete d;
}

BatchTool* Resize::clone(QObject* const parent) const
{
    return new Resize(parent);
}

void Resize::registerSettingsWidget()
{
    DVBox* const vbox   = new DVBox;
    d->labelPreset      = new QLabel(i18n("Preset Length:"), vbox);
    d->presetCBox       = new QComboBox(vbox);
    d->presetCBox->insertItem(Private::Tiny,   i18np("Tiny (1 pixel)",   "Tiny (%1 pixels)",   d->presetLengthValue(Private::Tiny)));
    d->presetCBox->insertItem(Private::Small,  i18np("Small (1 pixel)",  "Small (%1 pixels)",  d->presetLengthValue(Private::Small)));
    d->presetCBox->insertItem(Private::Medium, i18np("Medium (1 pixel)", "Medium (%1 pixels)", d->presetLengthValue(Private::Medium)));
    d->presetCBox->insertItem(Private::Big,    i18np("Big (1 pixel)",    "Big (%1 pixels)",    d->presetLengthValue(Private::Big)));
    d->presetCBox->insertItem(Private::Large,  i18np("Large (1 pixel)",  "Large (%1 pixels)",  d->presetLengthValue(Private::Large)));
    d->presetCBox->insertItem(Private::Huge,   i18np("Huge (1 pixel)",   "Huge (%1 pixels)",   d->presetLengthValue(Private::Huge)));
    d->presetCBox->insertItem(Private::QXGA,   i18np("QXGA (1 pixel)",   "QXGA (%1 pixels)",   d->presetLengthValue(Private::QXGA)));
    d->presetCBox->insertItem(Private::QHD,    i18np("QHD (1 pixel)",    "QHD (%1 pixels)",    d->presetLengthValue(Private::QHD)));
    d->presetCBox->insertItem(Private::UHD,    i18np("UHD (1 pixel)",    "UHD (%1 pixels)",    d->presetLengthValue(Private::UHD)));

    d->scaleDown        = new QCheckBox(i18n("Do Not Enlarge Small Images"), vbox);
    d->useCustom        = new QCheckBox(i18n("Use Custom Length"),           vbox);
    d->usePercent       = new QCheckBox(i18n("Use Percentage"),              vbox);
    d->customLength     = new DIntNumInput(vbox);
    d->customLength->setSuffix(i18n(" Pixels"));
    d->customLength->setRange(10, 10000, 1);
    d->customLength->setDefaultValue(1024);

    QLabel* const space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    m_settingsWidget    = vbox;

    connect(d->presetCBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->customLength, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useCustom, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->scaleDown, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->usePercent, SIGNAL(toggled(bool)),
            this, SLOT(slotPercentChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Resize::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("ScaleDown"),    false);
    settings.insert(QLatin1String("UseCustom"),    false);
    settings.insert(QLatin1String("UsePercent"),   false);
    settings.insert(QLatin1String("LengthCustom"), 1024);
    settings.insert(QLatin1String("LengthPreset"), Private::Medium);

    return settings;
}

void Resize::slotAssignSettings2Widget()
{
    d->changeSettings = false;
    d->scaleDown->setChecked(settings().value(QLatin1String("ScaleDown")).toBool());
    d->useCustom->setChecked(settings().value(QLatin1String("UseCustom")).toBool());
    d->usePercent->setChecked(settings().value(QLatin1String("UsePercent")).toBool());
    d->customLength->setValue(settings().value(QLatin1String("LengthCustom")).toInt());
    d->presetCBox->setCurrentIndex(settings().value(QLatin1String("LengthPreset")).toInt());
    d->changeSettings = true;
}

void Resize::slotSettingsChanged()
{
    d->labelPreset->setEnabled(!d->useCustom->isChecked());
    d->customLength->setEnabled(d->useCustom->isChecked());
    d->presetCBox->setEnabled(!d->useCustom->isChecked());
    d->usePercent->setEnabled(d->useCustom->isChecked());

    if (d->changeSettings)
    {
        BatchToolSettings settings;
        settings.insert(QLatin1String("ScaleDown"),    d->scaleDown->isChecked());
        settings.insert(QLatin1String("UseCustom"),    d->useCustom->isChecked());
        settings.insert(QLatin1String("UsePercent"),   d->usePercent->isChecked());
        settings.insert(QLatin1String("LengthCustom"), d->customLength->value());
        settings.insert(QLatin1String("LengthPreset"), d->presetCBox->currentIndex());
        BatchTool::slotSettingsChanged(settings);
    }
}

void Resize::slotPercentChanged()
{
    if (d->usePercent->isChecked())
    {
        d->customLength->setSuffix(QLatin1String("%"));
        d->customLength->setRange(1, 1000, 1);
        d->customLength->setDefaultValue(100);
        d->customLength->setValue(100);
    }
    else
    {
        d->customLength->setSuffix(i18n(" Pixels"));
        d->customLength->setRange(10, 10000, 1);
        d->customLength->setDefaultValue(1024);
        d->customLength->setValue(1024);
    }

    slotSettingsChanged();
}

bool Resize::toolOperations()
{
    bool scaleDown              = settings().value(QLatin1String("ScaleDown")).toBool();
    bool useCustom              = settings().value(QLatin1String("UseCustom")).toBool();
    bool usePercent             = settings().value(QLatin1String("UsePercent")).toBool();
    int length                  = settings().value(QLatin1String("LengthCustom")).toInt();
    Private::WidthPreset preset = (Private::WidthPreset)(settings().value(QLatin1String("LengthPreset")).toInt());

    if (!loadToDImg())
    {
        return false;
    }

    int longest = qMax(image().width(), image().height());

    if      (!useCustom)
    {
        length = d->presetLengthValue(preset);
    }
    else if (usePercent)
    {
        length = (int)(longest * (double)length / 100.0);
    }

    if (!scaleDown || (longest > length))
    {
        QSize newSize(image().size());
        newSize.scale(QSize(length, length), Qt::KeepAspectRatio);

        if (!newSize.isValid())
        {
            return false;
        }

        DImgBuiltinFilter filter(DImgBuiltinFilter::Resize, newSize);
        applyFilter(&filter);
    }

    return savefromDImg();
}

} // namespace DigikamBqmResizePlugin

#include "moc_resize.cpp"
