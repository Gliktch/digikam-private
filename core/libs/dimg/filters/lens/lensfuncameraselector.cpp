/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * SPDX-FileCopyrightText: 2008      by Adrian Schroeter <adrian at suse dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lensfuncameraselector.h"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QApplication>
#include <QStyle>
#include <QVariant>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "lensfuniface.h"
#include "dlayoutbox.h"
#include "squeezedcombobox.h"
#include "dnuminput.h"
#include "dexpanderbox.h"

// Disable deprecated API from older Lensfun.

#if !LENSFUN_TEST_VERSION(0,3,99)

#   if defined(Q_CC_GNU)
#       pragma GCC diagnostic push
#       pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#   endif

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wdeprecated-declarations"
#   endif

#endif

namespace Digikam
{

class Q_DECL_HIDDEN LensFunCameraSelector::Private
{
public:

    Private(LensFunCameraSelector* const qq)
        : q(qq)
    {
    }

    LensFunIface::MetadataMatch findFromMetadata();
    void refreshSettingsView();
    void populateDeviceCombos();
    void populateLensCombo();

public:

    bool                    passiveMetadataUsage    = false;

    QCheckBox*              metadataUsage           = nullptr;
    QLabel*                 metadataResult          = nullptr;
    QLabel*                 makeLabel               = nullptr;
    QLabel*                 modelLabel              = nullptr;
    QLabel*                 lensLabel               = nullptr;
    QLabel*                 focalLabel              = nullptr;
    QLabel*                 aperLabel               = nullptr;
    QLabel*                 distLabel               = nullptr;

    const QString           configUseMetadata       = QLatin1String("UseMetadata");
    const QString           configCameraModel       = QLatin1String("CameraModel");
    const QString           configCameraMake        = QLatin1String("CameraMake");
    const QString           configLensModel         = QLatin1String("LensModel");
    const QString           configSubjectDistance   = QLatin1String("SubjectDistance");
    const QString           configFocalLength       = QLatin1String("FocalLength");
    const QString           configCropFactor        = QLatin1String("CropFactor");
    const QString           configAperture          = QLatin1String("Aperture");
    const QString           redStyle                = QLatin1String("QLabel {color: red;}");
    const QString           orangeStyle             = QLatin1String("QLabel {color: orange;}");
    const QString           greenStyle              = QLatin1String("QLabel {color: green;}");

    DAdjustableLabel*       lensDescription         = nullptr;
    DAdjustableLabel*       makeDescription         = nullptr;
    DAdjustableLabel*       modelDescription        = nullptr;

    SqueezedComboBox*       make                    = nullptr;
    SqueezedComboBox*       model                   = nullptr;
    SqueezedComboBox*       lens                    = nullptr;

    DDoubleNumInput*        focal                   = nullptr;
    DDoubleNumInput*        aperture                = nullptr;
    DDoubleNumInput*        distance                = nullptr;

    DMetadata*              metadata                = nullptr;

    LensFunIface*           iface                   = nullptr;
    LensFunCameraSelector*  q                       = nullptr;
};

LensFunIface::MetadataMatch LensFunCameraSelector::Private::findFromMetadata()
{
    LensFunIface::MetadataMatch ret = iface->findFromMetadata(metadata);
    refreshSettingsView();
    q->slotModelSelected();
    q->slotLensSelected();

    return ret;
}

void LensFunCameraSelector::Private::refreshSettingsView()
{
    make->blockSignals(true);
    model->blockSignals(true);
    lens->blockSignals(true);

    makeLabel->setStyleSheet(qApp->styleSheet());
    modelLabel->setStyleSheet(qApp->styleSheet());
    lensLabel->setStyleSheet(qApp->styleSheet());
    focalLabel->setStyleSheet(qApp->styleSheet());
    aperLabel->setStyleSheet(qApp->styleSheet());
    distLabel->setStyleSheet(qApp->styleSheet());

    if (!passiveMetadataUsage)
    {
        makeDescription->setAdjustedText(QString::fromLatin1("<i>%1</i>").arg(iface->makeDescription()));
    }

    int makerIdx = -1;

    if (iface->usedCamera())
    {
        makerIdx = make->findOriginalText(iface->settings().cameraMake);
        qCDebug(DIGIKAM_DIMG_LOG) << "makerIdx: "
                                  << makerIdx << " ("
                                  << iface->settings().cameraMake << ")";
    }
    else
    {
        int i = make->findOriginalText(iface->makeDescription());

        if (i == -1)
        {
            i = make->findOriginalText(QLatin1String("Generic"));
        }

        if (i >= 0)
        {
            make->setCurrentIndex(i);
            populateDeviceCombos();
        }

        if (!passiveMetadataUsage)
        {
            makeLabel->setStyleSheet(orangeStyle);
        }
    }

    if (makerIdx >= 0)
    {
        make->setCurrentIndex(makerIdx);
        make->setEnabled(passiveMetadataUsage);

        if (!passiveMetadataUsage)
        {
            makeLabel->setStyleSheet(greenStyle);
        }

        populateDeviceCombos();
    }

    // ------------------------------------------------------------------------------------------------

    if (!passiveMetadataUsage)
    {
        modelDescription->setAdjustedText(QString::fromLatin1("<i>%1</i>").arg(iface->modelDescription()));
    }

    int modelIdx = -1;

    if (iface->usedCamera())
    {
        modelIdx = model->findOriginalText(iface->settings().cameraModel);
        qCDebug(DIGIKAM_DIMG_LOG) << "modelIdx: "
                                  << modelIdx << " ("
                                  << iface->settings().cameraModel << ")";
    }

    if (modelIdx >= 0)
    {
        model->setCurrentIndex(modelIdx);
        model->setEnabled(passiveMetadataUsage);

        if (!passiveMetadataUsage)
        {
            modelLabel->setStyleSheet(greenStyle);
        }

        populateLensCombo();
    }
    else
    {
        if (!passiveMetadataUsage)
        {
            modelLabel->setStyleSheet(orangeStyle);
        }
    }

    // ------------------------------------------------------------------------------------------------

    if (!passiveMetadataUsage)
    {
        lensDescription->setAdjustedText(QString::fromLatin1("<i>%1</i>").arg(iface->lensDescription()));
    }

    int lensIdx = -1;

    if (iface->usedLens())
    {
        lensIdx = lens->findOriginalText(iface->settings().lensModel);
        qCDebug(DIGIKAM_DIMG_LOG) << "lensIdx: "
                                  << lensIdx << " ("
                                  << iface->settings().lensModel << ")";
    }

    if (lensIdx >= 0)
    {
        // found lens model directly, best case :)

        lens->setCurrentIndex(lensIdx);
        lens->setEnabled(passiveMetadataUsage);

        if (!passiveMetadataUsage)
        {
            lensLabel->setStyleSheet(greenStyle);
        }
    }
    else
    {
        if (!passiveMetadataUsage)
        {
            lensLabel->setStyleSheet(orangeStyle);
        }
    }

    // ------------------------------------------------------------------------------------------------

    if (iface->settings().focalLength != -1.0)
    {
        focal->setValue(iface->settings().focalLength);
        focal->setEnabled(passiveMetadataUsage);

        if (!passiveMetadataUsage)
        {
            focalLabel->setStyleSheet(greenStyle);
        }
    }
    else
    {
        if (!passiveMetadataUsage)
        {
            focalLabel->setStyleSheet(orangeStyle);
        }
    }

    if (iface->settings().aperture != -1.0)
    {
        aperture->setValue(iface->settings().aperture);
        aperture->setEnabled(passiveMetadataUsage);

        if (!passiveMetadataUsage)
        {
            aperLabel->setStyleSheet(greenStyle);
        }
    }
    else
    {
        if (!passiveMetadataUsage)
        {
            aperLabel->setStyleSheet(orangeStyle);
        }
    }

    if (iface->settings().subjectDistance != -1.0)
    {
        distance->setValue(iface->settings().subjectDistance);
        distance->setEnabled(passiveMetadataUsage);

        if (!passiveMetadataUsage)
        {
            distLabel->setStyleSheet(greenStyle);
        }
    }
    else
    {
        if (!passiveMetadataUsage)
        {
            distLabel->setStyleSheet(orangeStyle);
        }
    }

    make->blockSignals(false);
    model->blockSignals(false);
    lens->blockSignals(false);
}

void LensFunCameraSelector::Private::populateDeviceCombos()
{
    make->blockSignals(true);
    model->blockSignals(true);

    const lfCamera* const* it = iface->lensFunCameras();

    // reset box

    model->clear();

    bool firstRun = false;

    if (make->count() == 0)
    {
        firstRun = true;
    }

    while (*it)
    {
        if (firstRun)
        {
            // Maker DB does not change, so we fill it only once.

            if ((*it)->Maker)
            {
                QString t = QLatin1String((*it)->Maker);

                if (make->findOriginalText(t) < 0)
                {
                    make->addSqueezedItem(t);
                }
            }
        }

        // Fill models for current selected maker

        if (
            (*it)->Model &&
            (QLatin1String((*it)->Maker) == make->itemHighlighted())
           )
        {
            LensFunIface::DevicePtr dev = *it;
            QVariant b                  = QVariant::fromValue(dev);

            model->addSqueezedItem(QLatin1String(dev->Model), b);
        }

        ++it;
    }

    make->blockSignals(false);
    model->blockSignals(false);
}

void LensFunCameraSelector::Private::populateLensCombo()
{
    lens->blockSignals(true);
    lens->clear();
    lens->blockSignals(false);

    QVariant v = model->itemData(model->currentIndex());

    if (!v.isValid() || v.isNull())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Invalid variant value for device!";
        return;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "variant: " << v;

    LensFunIface::DevicePtr dev = v.value<LensFunIface::DevicePtr>();

    if (!dev)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Device is null!";
        return;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "dev: " << dev->Maker
                              << " :: " << dev->Model
                              << " :: " << dev->CropFactor;

    lens->blockSignals(true);
    const lfLens** lenses     = iface->lensFunDataBase()->FindLenses(dev, nullptr, nullptr);
    LensFunContainer settings = iface->settings();
    settings.cropFactor       = dev ? dev->CropFactor : -1.0;
    iface->setSettings(settings);

    QMultiMap<QString, QVariant> lensMap;

    while (lenses && *lenses)
    {
        LensFunIface::LensPtr lens = *lenses;
        QVariant b                 = QVariant::fromValue(lens);

        lensMap.insert(QLatin1String(lens->Model), b);
        ++lenses;
    }

    QMultiMap<QString, QVariant>::ConstIterator it = lensMap.constBegin();

    for ( ; it != lensMap.constEnd() ; ++it)
    {
        lens->addSqueezedItem(it.key(), it.value());
    }

    lens->blockSignals(false);
}

// ---------------------------------------------------------------------------

LensFunCameraSelector::LensFunCameraSelector(QWidget* const parent)
    : QWidget(parent),
      d      (new Private(this))
{
    d->iface                = new LensFunIface();

    QGridLayout* const grid = new QGridLayout(this);
    DHBox* const hbox       = new DHBox(this);
    d->metadataUsage        = new QCheckBox(i18n("Use Metadata"), hbox);
    QLabel* const space     = new QLabel(hbox);
    d->metadataResult       = new QLabel(hbox);
    hbox->setStretchFactor(space, 10);

    DHBox* const hbox1      = new DHBox(this);
    d->makeLabel            = new QLabel(i18nc("camera make",  "Make:"),  hbox1);
    QLabel* const space1    = new QLabel(hbox1);
    d->makeDescription      = new DAdjustableLabel(hbox1);
    hbox1->setStretchFactor(space1, 10);
    d->makeDescription->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    d->makeDescription->setWhatsThis(i18n("This is the camera maker description string found in image meta-data. "
                                          "This one is used to query and find relevant camera device information from Lensfun database."));

    d->make                 = new SqueezedComboBox(this);
    d->make->setCurrentIndex(0);

    DHBox* const hbox2      = new DHBox(this);
    d->modelLabel           = new QLabel(i18nc("camera model", "Model:"), hbox2);
    QLabel* const space2    = new QLabel(hbox2);
    d->modelDescription     = new DAdjustableLabel(hbox2);
    hbox2->setStretchFactor(space2, 10);
    d->modelDescription->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    d->modelDescription->setWhatsThis(i18n("This is the camera model description string found in image meta-data. "
                                           "This one is used to query and found relevant camera device information from Lensfun database."));

    d->model                = new SqueezedComboBox(this);
    d->model->setCurrentIndex(0);

    DHBox* const hbox3      = new DHBox(this);
    d->lensLabel            = new QLabel(i18nc("camera lens",  "Lens:"),  hbox3);
    QLabel* const space3    = new QLabel(hbox3);
    d->lensDescription      = new DAdjustableLabel(hbox3);

    // Workaround for layout problem with long lens names.

    d->lensDescription->setAdjustedText(QString(40, QLatin1Char(' ')));
    d->lensDescription->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    d->lensDescription->setWhatsThis(i18n("This is the lens description string found in image meta-data. "
                                          "This one is used to query and found relevant lens information from Lensfun database."));
    hbox3->setStretchFactor(space3, 10);

    d->lens                 = new SqueezedComboBox(this);
    d->lens->setCurrentIndex(0);

    d->metadataUsage->setEnabled(false);
    d->metadataUsage->setCheckState(Qt::Unchecked);
    d->metadataUsage->setWhatsThis(i18n("Set this option to try to guess the right camera/lens settings "
                                        "from the image metadata (as Exif or XMP)."));

    d->focalLabel           = new QLabel(i18n("Focal Length (mm):"), this);
    d->aperLabel            = new QLabel(i18n("Aperture:"), this);
    d->distLabel            = new QLabel(i18n("Subject Distance (m):"), this);

    d->focal                = new DDoubleNumInput(this);
    d->focal->setDecimals(1);
    d->focal->setRange(1.0, 10000.0, 0.01);
    d->focal->setDefaultValue(1.0);

    d->aperture             = new DDoubleNumInput(this);
    d->aperture->setDecimals(1);
    d->aperture->setRange(1.1, 256.0, 0.1);
    d->aperture->setDefaultValue(1.1);

    d->distance             = new DDoubleNumInput(this);
    d->distance->setDecimals(1);
    d->distance->setRange(0.0, 10000.0, 0.1);
    d->distance->setDefaultValue(0.0);

    grid->addWidget(hbox,          0, 0, 1, 3);
    grid->addWidget(hbox1,         1, 0, 1, 3);
    grid->addWidget(d->make,       2, 0, 1, 3);
    grid->addWidget(hbox2,         3, 0, 1, 3);
    grid->addWidget(d->model,      4, 0, 1, 3);
    grid->addWidget(hbox3,         5, 0, 1, 3);
    grid->addWidget(d->lens,       6, 0, 1, 3);
    grid->addWidget(d->focalLabel, 7, 0, 1, 1);
    grid->addWidget(d->focal,      7, 1, 1, 2);
    grid->addWidget(d->aperLabel,  8, 0, 1, 1);
    grid->addWidget(d->aperture,   8, 1, 1, 2);
    grid->addWidget(d->distLabel,  9, 0, 1, 1);
    grid->addWidget(d->distance,   9, 1, 1, 2);
    grid->setSpacing(layoutSpacing());
    grid->setContentsMargins(QMargins());

    connect(d->metadataUsage, SIGNAL(toggled(bool)),
            this, SLOT(slotUseMetadata(bool)));

    connect(d->make, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotMakeSelected()));

    connect(d->model, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotModelChanged()));

    connect(d->lens, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotLensSelected()));

    connect(d->focal, SIGNAL(valueChanged(double)),
            this, SLOT(slotFocalChanged()));

    connect(d->aperture, SIGNAL(valueChanged(double)),
            this, SLOT(slotApertureChanged()));

    connect(d->distance, SIGNAL(valueChanged(double)),
            this, SLOT(slotDistanceChanged()));

    d->populateDeviceCombos();
    d->populateLensCombo();
}

LensFunCameraSelector::~LensFunCameraSelector()
{
    delete d->metadata;
    delete d->iface;
    delete d;
}

LensFunContainer LensFunCameraSelector::settings()
{
    // Update settings in LensFun interface

    blockSignals(true);

    slotModelSelected();
    slotLensSelected();
    slotFocalChanged();
    slotApertureChanged();
    slotDistanceChanged();

    blockSignals(false);

    return d->iface->settings();
}

void LensFunCameraSelector::setSettings(const LensFunContainer& settings)
{
    blockSignals(true);

    d->iface->setSettings(settings);
    d->refreshSettingsView();

    blockSignals(false);
}

void LensFunCameraSelector::resetToDefault()
{
    setUseMetadata(true);
}

void LensFunCameraSelector::readSettings(const KConfigGroup& group)
{
    setUseMetadata(group.readEntry(d->configUseMetadata, true));

    if (!useMetadata())
    {
        LensFunContainer set = d->iface->settings();
        set.cameraModel      = group.readEntry(d->configCameraModel, QString());
        set.cameraMake       = group.readEntry(d->configCameraMake,  QString());
        set.lensModel        = group.readEntry(d->configLensModel,   QString());

        if (set.subjectDistance <= 0.0)
        {
            set.subjectDistance = group.readEntry(d->configSubjectDistance, -1.0);
        }

        if (set.focalLength <= 0.0)
        {
            set.focalLength = group.readEntry(d->configFocalLength, -1.0);
        }

        set.cropFactor = group.readEntry(d->configCropFactor, -1.0);

        if (set.aperture <= 0.0)
        {
            set.aperture = group.readEntry(d->configAperture, -1.0);
        }

        setSettings(set);
    }

    slotUseMetadata(useMetadata());
}

void LensFunCameraSelector::writeSettings(KConfigGroup& group)
{
    group.writeEntry(d->configUseMetadata,     useMetadata());
    group.writeEntry(d->configCameraModel,     d->iface->settings().cameraModel);
    group.writeEntry(d->configCameraMake,      d->iface->settings().cameraMake);
    group.writeEntry(d->configLensModel,       d->iface->settings().lensModel);
    group.writeEntry(d->configSubjectDistance, d->iface->settings().subjectDistance);
    group.writeEntry(d->configFocalLength,     d->iface->settings().focalLength);
    group.writeEntry(d->configCropFactor,      d->iface->settings().cropFactor);
    group.writeEntry(d->configAperture,        d->iface->settings().aperture);
}

void LensFunCameraSelector::setMetadata(const MetaEngineData& meta)
{
    if (d->metadata)
    {
        delete d->metadata;
    }

    d->metadata = new DMetadata(meta);

    if (d->metadata->isEmpty())
    {
        d->metadataUsage->setCheckState(Qt::Unchecked);
        setEnabledUseMetadata(false);
    }
    else
    {
        setEnabledUseMetadata(true);
        d->findFromMetadata();
    }
}

void LensFunCameraSelector::setEnabledUseMetadata(bool b)
{
    d->metadataUsage->setEnabled(b);
}

void LensFunCameraSelector::setUseMetadata(bool b)
{
    d->metadataUsage->setChecked(b);
}

bool LensFunCameraSelector::useMetadata() const
{
    return (d->metadataUsage->isChecked());
}

void LensFunCameraSelector::setPassiveMetadataUsage(bool b)
{
    d->passiveMetadataUsage = b;
}

void LensFunCameraSelector::slotUseMetadata(bool b)
{
    d->makeDescription->setAdjustedText();
    d->modelDescription->setAdjustedText();
    d->lensDescription->setAdjustedText();
    d->metadataResult->clear();
    d->makeLabel->setStyleSheet(qApp->styleSheet());
    d->modelLabel->setStyleSheet(qApp->styleSheet());
    d->lensLabel->setStyleSheet(qApp->styleSheet());
    d->focalLabel->setStyleSheet(qApp->styleSheet());
    d->aperLabel->setStyleSheet(qApp->styleSheet());
    d->distLabel->setStyleSheet(qApp->styleSheet());
    d->make->setEnabled(true);
    d->model->setEnabled(true);
    d->lens->setEnabled(true);
    d->focal->setEnabled(true);
    d->aperture->setEnabled(true);
    d->distance->setEnabled(true);

    if (b)
    {
        if (d->passiveMetadataUsage)
        {
            d->make->setEnabled(false);
            d->model->setEnabled(false);
            d->lens->setEnabled(false);
            d->focal->setEnabled(false);
            d->aperture->setEnabled(false);
            d->distance->setEnabled(false);

            Q_EMIT signalLensSettingsChanged();
        }
        else
        {
            LensFunIface::MetadataMatch ret = d->findFromMetadata();

            switch (ret)
            {
                case LensFunIface::MetadataUnavailable:
                {
                    d->metadataResult->setText(i18n("(no metadata available)"));
                    d->metadataResult->setStyleSheet(d->redStyle);
                    break;
                }

                case LensFunIface::MetadataNoMatch:
                {
                    d->metadataResult->setText(i18n("(no match found)"));
                    d->metadataResult->setStyleSheet(d->redStyle);
                    break;
                }

                case LensFunIface::MetadataPartialMatch:
                {
                    d->metadataResult->setText(i18n("(partial match found)"));
                    d->metadataResult->setStyleSheet(d->orangeStyle);
                    break;
                }

                default:
                {
                    d->metadataResult->setText(i18n("(exact match found)"));
                    d->metadataResult->setStyleSheet(d->greenStyle);
                    break;
                }
            }
        }
    }
}

void LensFunCameraSelector::slotMakeSelected()
{
    d->populateDeviceCombos();
    slotModelSelected();

    // Fill Lens list for current Maker & Model and fire signalLensSettingsChanged().

    d->populateLensCombo();
    slotLensSelected();
}

void LensFunCameraSelector::slotModelChanged()
{
    d->populateLensCombo();
    slotModelSelected();
}

void LensFunCameraSelector::slotModelSelected()
{
    QVariant v = d->model->itemData(d->model->currentIndex());
    d->iface->setUsedCamera((d->metadataUsage->isChecked() && d->passiveMetadataUsage) ? nullptr
                                                                                       : v.value<LensFunIface::DevicePtr>());
    Q_EMIT signalLensSettingsChanged();
}

void LensFunCameraSelector::slotLensSelected()
{
    QVariant v = d->lens->itemData(d->lens->currentIndex());
    d->iface->setUsedLens((d->metadataUsage->isChecked() && d->passiveMetadataUsage) ? nullptr
                                                                                     : v.value<LensFunIface::LensPtr>());

    LensFunContainer set = d->iface->settings();

    if (
        d->iface->usedLens() &&
        (set.cropFactor <= 0.0) // this should not happen
       )
    {

#if LENSFUN_TEST_VERSION(0,3,99)

        qCDebug(DIGIKAM_DIMG_LOG) << "No crop factor is set for camera, using 1.0 default value.";

        set.cropFactor = 1.0;

#else

        qCDebug(DIGIKAM_DIMG_LOG) << "No crop factor is set for camera, using lens calibration data: "
                                  << d->iface->usedLens()->CropFactor;

        set.cropFactor = d->iface->usedLens()->CropFactor;

#endif

    }

    d->iface->setSettings(set);

    Q_EMIT signalLensSettingsChanged();
}

void LensFunCameraSelector::slotFocalChanged()
{
    LensFunContainer set = d->iface->settings();
    set.focalLength      = (d->metadataUsage->isChecked() && d->passiveMetadataUsage) ? -1.0
                                                                                      : d->focal->value();
    d->iface->setSettings(set);

    Q_EMIT signalLensSettingsChanged();
}

void LensFunCameraSelector::slotApertureChanged()
{
    LensFunContainer set = d->iface->settings();
    set.aperture         = d->metadataUsage->isChecked() && d->passiveMetadataUsage ? -1.0
                                                                                    : d->aperture->value();
    d->iface->setSettings(set);

    Q_EMIT signalLensSettingsChanged();
}

void LensFunCameraSelector::slotDistanceChanged()
{
    LensFunContainer set = d->iface->settings();
    set.subjectDistance  = d->metadataUsage->isChecked() && d->passiveMetadataUsage ? -1.0
                                                                                    : d->distance->value();
    d->iface->setSettings(set);

    Q_EMIT signalLensSettingsChanged();
}

bool LensFunCameraSelector::supportsCCA() const
{
    return d->iface->supportsCCA();
}

bool LensFunCameraSelector::supportsVig() const
{
    return d->iface->supportsVig();
}

bool LensFunCameraSelector::supportsDistortion() const
{
    return d->iface->supportsDistortion();
}

bool LensFunCameraSelector::supportsGeometry() const
{
    return d->iface->supportsGeometry();
}

} // namespace Digikam

// Restore warnings with older Lensfun API

#if !LENSFUN_TEST_VERSION(0,3,99)

#   if defined(Q_CC_GNU)
#       pragma GCC diagnostic pop
#   endif

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic pop
#   endif

#endif

#include "moc_lensfuncameraselector.cpp"
