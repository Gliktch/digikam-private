/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-10
 * Description : rotate image batch tool.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rotate.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "dcombobox.h"
#include "dimg.h"
#include "dimgbuiltinfilter.h"
#include "dmetadata.h"
#include "jpegutils.h"
#include "freerotationfilter.h"
#include "freerotationsettings.h"

namespace DigikamBqmRotatePlugin
{

class Q_DECL_HIDDEN Rotate::Private
{
public:

    Private() = default;

public:

    const int             CUSTOM_ANGLE  = DImg::ROT270 + 1;

    QLabel*               label         = nullptr;

    QCheckBox*            useExif       = nullptr;

    DComboBox*            comboBox      = nullptr;

    FreeRotationSettings* frSettings    = nullptr;
};

Rotate::Rotate(QObject* const parent)
    : BatchTool(QLatin1String("Rotate"), TransformTool, parent),
      d        (new Private)
{
}

Rotate::~Rotate()
{
    delete d;
}

BatchTool* Rotate::clone(QObject* const parent) const
{
    return new Rotate(parent);
}

void Rotate::registerSettingsWidget()
{
    DVBox* const vbox   = new DVBox;
    d->useExif          = new QCheckBox(i18nc("@title", "Use Exif Orientation"), vbox);

    d->label            = new QLabel(vbox);
    d->comboBox         = new DComboBox(vbox);
    d->comboBox->insertItem(DImg::ROT90,     i18nc("@item: angle", "90 degrees"));
    d->comboBox->insertItem(DImg::ROT180,    i18nc("@item: angle", "180 degrees"));
    d->comboBox->insertItem(DImg::ROT270,    i18nc("@item: angle", "270 degrees"));
    d->comboBox->insertItem(d->CUSTOM_ANGLE, i18nc("@item: angle", "Custom"));
    d->comboBox->setDefaultIndex(DImg::ROT90);
    d->label->setText(i18nc("@label", "Angle:"));

    d->frSettings       = new FreeRotationSettings(vbox);

    QLabel* const space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    m_settingsWidget    = vbox;

    setNeedResetExifOrientation(true);

    connect(d->comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useExif, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->frSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    slotSettingsChanged();

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Rotate::defaultSettings()
{
    BatchToolSettings settings;
    FreeRotationContainer defaultPrm = d->frSettings->defaultSettings();

    settings.insert(QLatin1String("useExif"),         true);
    settings.insert(QLatin1String("rotation"),        d->comboBox->defaultIndex());
    settings.insert(QLatin1String("angle"),           defaultPrm.angle);
    settings.insert(QLatin1String("antiAlias"),       defaultPrm.antiAlias);
    settings.insert(QLatin1String("autoCrop"),        defaultPrm.autoCrop);
    settings.insert(QLatin1String("backgroundColor"), defaultPrm.backgroundColor);

    return settings;
}

void Rotate::slotAssignSettings2Widget()
{
    d->useExif->setChecked(settings().value(QLatin1String("useExif")).toBool());
    d->comboBox->setCurrentIndex(settings().value(QLatin1String("rotation")).toInt());
    FreeRotationContainer prm;
    prm.angle           = settings().value(QLatin1String("angle")).toDouble();
    prm.antiAlias       = settings().value(QLatin1String("antiAlias")).toBool();
    prm.autoCrop        = settings().value(QLatin1String("autoCrop")).toInt();
    prm.backgroundColor = settings().value(QLatin1String("backgroundColor")).value<QColor>();
    d->frSettings->setSettings(prm);
}

void Rotate::slotSettingsChanged()
{
    d->label->setEnabled(!d->useExif->isChecked());
    d->comboBox->setEnabled(!d->useExif->isChecked());
    d->frSettings->setEnabled(d->comboBox->isEnabled() && d->comboBox->currentIndex() == d->CUSTOM_ANGLE);

    BatchToolSettings settings;
    FreeRotationContainer currentPrm = d->frSettings->settings();

    settings.insert(QLatin1String("useExif"),         d->useExif->isChecked());
    settings.insert(QLatin1String("rotation"),        d->comboBox->currentIndex());
    settings.insert(QLatin1String("angle"),           currentPrm.angle);
    settings.insert(QLatin1String("antiAlias"),       currentPrm.antiAlias);
    settings.insert(QLatin1String("autoCrop"),        currentPrm.autoCrop);
    settings.insert(QLatin1String("backgroundColor"), currentPrm.backgroundColor);

    BatchTool::slotSettingsChanged(settings);
}

bool Rotate::toolOperations()
{
    FreeRotationContainer prm;
    bool useExif        = settings().value(QLatin1String("useExif")).toBool();
    int rotation        = settings().value(QLatin1String("rotation")).toInt();
    prm.angle           = settings().value(QLatin1String("angle")).toDouble();
    prm.antiAlias       = settings().value(QLatin1String("antiAlias")).toBool();
    prm.autoCrop        = settings().value(QLatin1String("autoCrop")).toInt();
    prm.backgroundColor = settings().value(QLatin1String("backgroundColor")).value<QColor>();

    // JPEG image : lossless method if non-custom rotation angle.

    if (JPEGUtils::isJpegImage(inputUrl().toLocalFile()) && image().isNull())
    {
        JPEGUtils::JpegRotator rotator(inputUrl().toLocalFile());
        rotator.setDestinationFile(outputUrl().toLocalFile());

        if (useExif)
        {
            return rotator.autoExifTransform();
        }
        else
        {
            switch (rotation)
            {
                case DImg::ROT90:
                {
                    return rotator.exifTransform(MetaEngineRotation::Rotate90);
                }

                case DImg::ROT180:
                {
                    return rotator.exifTransform(MetaEngineRotation::Rotate180);
                }

                case DImg::ROT270:
                {
                    return rotator.exifTransform(MetaEngineRotation::Rotate270);
                }

                default:
                {
                    // there is no lossless method to turn JPEG image with a custom angle.
                    // fall through
                    break;
                }
            }
        }
    }

    // Non-JPEG image: DImg

    if (!loadToDImg())
    {
        return false;
    }

    if (useExif)
    {
        // Exif rotation is currently not recorded to image history

        image().rotateAndFlip(image().exifOrientation(inputUrl().toLocalFile()));
    }
    else
    {
        switch (rotation)
        {
            case DImg::ROT90:
            {
                DImgBuiltinFilter filter(DImgBuiltinFilter::Rotate90);
                applyFilter(&filter);
                break;
            }

            case DImg::ROT180:
            {
                DImgBuiltinFilter filter(DImgBuiltinFilter::Rotate180);
                applyFilter(&filter);
                break;
            }

            case DImg::ROT270:
            {
                DImgBuiltinFilter filter(DImgBuiltinFilter::Rotate270);
                applyFilter(&filter);
                break;
            }

            default:      // Custom value
            {
                FreeRotationFilter fr(&image(), nullptr, prm);
                applyFilterChangedProperties(&fr);
                break;
            }
        }
    }

    return (savefromDImg());
}

} // namespace DigikamBqmRotatePlugin

#include "moc_rotate.cpp"
