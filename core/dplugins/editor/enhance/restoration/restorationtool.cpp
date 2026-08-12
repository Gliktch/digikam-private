/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-26
 * Description : a digiKam image editor tool to restore
 *               a photograph
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "restorationtool.h"

// Qt includes

#include <QFile>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QIcon>
#include <QApplication>
#include <QComboBox>
#include <QStandardPaths>
#include <QMessageBox>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "editortoolsettings.h"
#include "greycstorationfilter.h"
#include "greycstorationsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "dactivelabel.h"
#include "dfiledialog.h"

namespace DigikamEditorRestorationToolPlugin
{

class Q_DECL_HIDDEN RestorationTool::Private
{
public:

    enum RestorationFilteringPreset
    {
        ReduceUniformNoise = 0,
        ReduceJPEGArtefacts,
        ReduceTexturing,
        Custom
    };

public:

    Private() = default;

public:

    const QString configGroupName               = QLatin1String("restoration Tool");
    const QString configPresetEntry             = QLatin1String("Preset");
    const QString configFastApproxEntry         = QLatin1String("FastApprox");
    const QString configInterpolationEntry      = QLatin1String("Interpolation");
    const QString configAmplitudeEntry          = QLatin1String("Amplitude");
    const QString configSharpnessEntry          = QLatin1String("Sharpness");
    const QString configAnisotropyEntry         = QLatin1String("Anisotropy");
    const QString configAlphaEntry              = QLatin1String("Alpha");
    const QString configSigmaEntry              = QLatin1String("Sigma");
    const QString configGaussPrecEntry          = QLatin1String("GaussPrec");
    const QString configDlEntry                 = QLatin1String("Dl");
    const QString configDaEntry                 = QLatin1String("Da");
    const QString configIterationEntry          = QLatin1String("Iteration");
    const QString configTileEntry               = QLatin1String("Tile");
    const QString configBTileEntry              = QLatin1String("BTile");

    QTabWidget*             mainTab             = nullptr;

    QComboBox*              restorationTypeCB   = nullptr;

    GreycstorationSettings* settingsWidget      = nullptr;
    ImageRegionWidget*      previewWidget       = nullptr;
    EditorToolSettings*     gboxSettings        = nullptr;
};


// --------------------------------------------------------

RestorationTool::RestorationTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("restoration"));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(nullptr);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Try);

    QGridLayout* const gridSettings = new QGridLayout(d->gboxSettings->plainPage());
    d->mainTab                      = new QTabWidget( d->gboxSettings->plainPage() );

    QWidget* const firstPage = new QWidget( d->mainTab );
    QGridLayout* const grid  = new QGridLayout(firstPage);
    d->mainTab->addTab( firstPage, i18n("Preset") );

    DActiveLabel* const cimgLogoLabel = new DActiveLabel(QUrl(QLatin1String("http://cimg.sourceforge.net")),    // krazy:exclude=insecurenet
                                                         QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/logo-cimg.webp")));
    cimgLogoLabel->setToolTip(i18n("Visit CImg library website"));

    QLabel* const typeLabel = new QLabel(i18n("Filtering type:"), firstPage);
    typeLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    d->restorationTypeCB    = new QComboBox(firstPage);
    d->restorationTypeCB->addItem( i18n("Reduce Uniform Noise") );
    d->restorationTypeCB->addItem( i18n("Reduce JPEG Artifacts") );
    d->restorationTypeCB->addItem( i18n("Reduce Texturing") );
    d->restorationTypeCB->addItem( i18nc("custom restoration settings", "Custom") );
    d->restorationTypeCB->setWhatsThis(i18n("<p>Select the filter preset to use for photograph restoration here:</p>"
                                            "<p><b>Reduce Uniform Noise</b>: reduce small image artifacts such as sensor noise.<br>"
                                            "<b>Reduce JPEG Artifacts</b>: reduce large image artifacts, such as a JPEG compression mosaic.<br>"
                                            "<b>Reduce Texturing</b>: reduce image artifacts, such as paper texture, or Moire patterns on scanned images.<br>"
                                            "<b>Custom</b>: Puts settings to most common values, fully customizable.</p>"));

    const int spacing = d->gboxSettings->spacingHint();

    grid->addWidget(cimgLogoLabel,        0, 1, 1, 1);
    grid->addWidget(typeLabel,            1, 0, 1, 1);
    grid->addWidget(d->restorationTypeCB, 1, 1, 1, 1);
    grid->setRowStretch(1, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(0);

    // -------------------------------------------------------------

    d->settingsWidget = new GreycstorationSettings( d->mainTab );
    gridSettings->addWidget(d->mainTab,                               0, 1, 1, 1);
    gridSettings->addWidget(new QLabel(d->gboxSettings->plainPage()), 1, 1, 1, 1);
    gridSettings->setContentsMargins(spacing, spacing, spacing, spacing);
    gridSettings->setSpacing(spacing);
    gridSettings->setRowStretch(2, 10);

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    connect(d->restorationTypeCB, SIGNAL(activated(int)),
            this, SLOT(slotResetValues(int)));

    // -------------------------------------------------------------

    GreycstorationContainer defaults;
    defaults.setRestorationDefaultSettings();
    d->settingsWidget->setDefaultSettings(defaults);
}

RestorationTool::~RestorationTool()
{
    delete d;
}

void RestorationTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    GreycstorationContainer prm;
    GreycstorationContainer defaults;
    defaults.setRestorationDefaultSettings();

    prm.fastApprox = group.readEntry(d->configFastApproxEntry,    defaults.fastApprox);
    prm.interp     = group.readEntry(d->configInterpolationEntry, defaults.interp);
    prm.amplitude  = group.readEntry(d->configAmplitudeEntry,     (double)defaults.amplitude);
    prm.sharpness  = group.readEntry(d->configSharpnessEntry,     (double)defaults.sharpness);
    prm.anisotropy = group.readEntry(d->configAnisotropyEntry,    (double)defaults.anisotropy);
    prm.alpha      = group.readEntry(d->configAlphaEntry,         (double)defaults.alpha);
    prm.sigma      = group.readEntry(d->configSigmaEntry,         (double)defaults.sigma);
    prm.gaussPrec  = group.readEntry(d->configGaussPrecEntry,     (double)defaults.gaussPrec);
    prm.dl         = group.readEntry(d->configDlEntry,            (double)defaults.dl);
    prm.da         = group.readEntry(d->configDaEntry,            (double)defaults.da);
    prm.nbIter     = group.readEntry(d->configIterationEntry,     defaults.nbIter);
    prm.tile       = group.readEntry(d->configTileEntry,          defaults.tile);
    prm.btile      = group.readEntry(d->configBTileEntry,         defaults.btile);
    d->settingsWidget->setSettings(prm);

    int p = group.readEntry(d->configPresetEntry, (int)Private::ReduceUniformNoise);
    d->restorationTypeCB->setCurrentIndex(p);

    if (p == Private::Custom)
    {
        d->settingsWidget->setEnabled(true);
    }
    else
    {
        d->settingsWidget->setEnabled(false);
    }
}

void RestorationTool::writeSettings()
{
    GreycstorationContainer prm = d->settingsWidget->settings();
    KSharedConfig::Ptr config   = KSharedConfig::openConfig();
    KConfigGroup group          = config->group(d->configGroupName);

    group.writeEntry(d->configPresetEntry,        d->restorationTypeCB->currentIndex());
    group.writeEntry(d->configFastApproxEntry,    prm.fastApprox);
    group.writeEntry(d->configInterpolationEntry, prm.interp);
    group.writeEntry(d->configAmplitudeEntry,     (double)prm.amplitude);
    group.writeEntry(d->configSharpnessEntry,     (double)prm.sharpness);
    group.writeEntry(d->configAnisotropyEntry,    (double)prm.anisotropy);
    group.writeEntry(d->configAlphaEntry,         (double)prm.alpha);
    group.writeEntry(d->configSigmaEntry,         (double)prm.sigma);
    group.writeEntry(d->configGaussPrecEntry,     (double)prm.gaussPrec);
    group.writeEntry(d->configDlEntry,            (double)prm.dl);
    group.writeEntry(d->configDaEntry,            (double)prm.da);
    group.writeEntry(d->configIterationEntry,     prm.nbIter);
    group.writeEntry(d->configTileEntry,          prm.tile);
    group.writeEntry(d->configBTileEntry,         prm.btile);
    group.sync();
}

void RestorationTool::slotResetValues(int i)
{
    if (i == Private::Custom)
    {
        d->settingsWidget->setEnabled(true);
    }
    else
    {
        d->settingsWidget->setEnabled(false);
    }

    slotResetSettings();
}

void RestorationTool::slotResetSettings()
{
    GreycstorationContainer settings;
    settings.setRestorationDefaultSettings();

    switch (d->restorationTypeCB->currentIndex())
    {
        case Private::ReduceUniformNoise:
        {
            settings.amplitude = 40.0;
            break;
        }

        case Private::ReduceJPEGArtefacts:
        {
            settings.sharpness = 0.3F;
            settings.sigma     = 1.0;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }

        case Private::ReduceTexturing:
        {
            settings.sharpness = 0.5F;
            settings.sigma     = 1.5;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }

        default: // Custom
        {
            break;
        }
    }

    d->settingsWidget->setSettings(settings);
}

void RestorationTool::preparePreview()
{
    DImg previewImage = d->previewWidget->getOriginalRegionImage();

    setFilter(new GreycstorationFilter(&previewImage,
                                       d->settingsWidget->settings(), GreycstorationFilter::Restore,
                                       0, 0, QImage(), this));
}

void RestorationTool::prepareFinal()
{
    ImageIface iface;
    DImg originalImage = iface.original()->copy();

    setFilter(new GreycstorationFilter(&originalImage,
                                       d->settingsWidget->settings(), GreycstorationFilter::Restore,
                                       0, 0, QImage(), this));
}

void RestorationTool::setPreviewImage()
{
    DImg imDest = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(imDest);
}

void RestorationTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Restoration"), filter()->filterAction(), filter()->getTargetImage());
}

void RestorationTool::slotLoadSettings()
{
    QUrl loadRestorationFile = DFileDialog::getOpenFileUrl(qApp->activeWindow(), i18nc("@title:window", "Photograph Restoration Settings File to Load"),
                                                           QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
                                                           QLatin1String("*"));

    if ( loadRestorationFile.isEmpty() )
    {
        return;
    }

    QFile file(loadRestorationFile.toLocalFile());

    if ( file.open(QIODevice::ReadOnly) )
    {
        if (!d->settingsWidget->loadSettings(file, QLatin1String("# Photograph Restoration Configuration File V2")))
        {
            QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                                  i18n("\"%1\" is not a Photograph Restoration settings text file.",
                                       loadRestorationFile.fileName()));
            file.close();
            return;
        }

        slotPreview();
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("Cannot load settings from the Photograph Restoration text file."));
    }

    file.close();
    d->restorationTypeCB->blockSignals(true);
    d->restorationTypeCB->setCurrentIndex((int)Private::Custom);
    d->restorationTypeCB->blockSignals(false);
    d->settingsWidget->setEnabled(true);
}

void RestorationTool::slotSaveAsSettings()
{
    QUrl saveRestorationFile = DFileDialog::getSaveFileUrl(qApp->activeWindow(), i18nc("@title:window", "Photograph Restoration Settings File to Save"),
                                                           QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
                                                           QLatin1String("*"));

    if ( saveRestorationFile.isEmpty() )
    {
        return;
    }

    QFile file(saveRestorationFile.toLocalFile());

    if ( file.open(QIODevice::WriteOnly) )
    {
        d->settingsWidget->saveSettings(file, QLatin1String("# Photograph Restoration Configuration File V2"));
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("Cannot save settings to the Photograph Restoration text file."));
    }

    file.close();
}

} // namespace DigikamEditorRestorationToolPlugin

#include "moc_restorationtool.cpp"
