/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-21
 * Description : a view to show Queue Settings.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "queuesettingsview.h"

// Qt includes

#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QTimer>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "advancedrenamewidget.h"
#include "defaultrenameparser.h"
#include "album.h"
#include "albumselectwidget.h"
#include "batchtool.h"
#include "queuesettings.h"
#include "dpluginloader.h"
#include "drawdecoderwidget.h"
#include "filesaveconflictbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN QueueSettingsView::Private
{
public:

    enum SettingsTabs
    {
        TARGET = 0,
        RENAMING,
        BEHAVIOR,
        RAW,
        SAVE
    };

public:

    Private() = default;

    QLabel*                rawLoadingLabel          = nullptr;

    QButtonGroup*          renamingButtonGroup      = nullptr;
    QButtonGroup*          rawLoadingButtonGroup    = nullptr;

    QRadioButton*          renameOriginal           = nullptr;
    QRadioButton*          renameManual             = nullptr;
    QRadioButton*          extractJPEGButton        = nullptr;
    QRadioButton*          demosaicingButton        = nullptr;

    QCheckBox*             useOrgAlbum              = nullptr;
    QCheckBox*             asNewVersion             = nullptr;
    QCheckBox*             useMutiCoreCPU           = nullptr;

    FileSaveConflictBox*   conflictBox              = nullptr;
    AlbumSelectWidget*     albumSel                 = nullptr;

    AdvancedRenameManager* advancedRenameManager    = nullptr;
    AdvancedRenameWidget*  advancedRenameWidget     = nullptr;

    DRawDecoderWidget*     rawSettings              = nullptr;

    DImgLoaderSettings*    jpgSettings              = nullptr;
    DImgLoaderSettings*    pngSettings              = nullptr;
    DImgLoaderSettings*    tifSettings              = nullptr;

#ifdef HAVE_JASPER

    DImgLoaderSettings*    j2kSettings              = nullptr;

#endif // HAVE_JASPER

#ifdef HAVE_X265

    DImgLoaderSettings*    heifSettings             = nullptr;

#endif // HAVE_X265

    DImgLoaderSettings*    pgfSettings              = nullptr;
    DImgLoaderSettings*    jxlSettings              = nullptr;
    DImgLoaderSettings*    webpSettings             = nullptr;
    DImgLoaderSettings*    avifSettings             = nullptr;
};

QueueSettingsView::QueueSettingsView(QWidget* const parent)
    : QTabWidget(parent),
      d         (new Private)
{
    setTabsClosable(false);

    // --------------------------------------------------------

    const int spacing = layoutSpacing();


    QScrollArea* const sv3   = new QScrollArea(this);
    DVBox* const vbox3       = new DVBox(sv3->viewport());
    sv3->setWidget(vbox3);
    sv3->setWidgetResizable(true);
    vbox3->setContentsMargins(spacing, spacing, spacing, spacing);
    vbox3->setSpacing(spacing);

    d->useOrgAlbum           = new QCheckBox(i18n("Use original Album"), vbox3);
    d->albumSel              = new AlbumSelectWidget(vbox3);
    insertTab(Private::TARGET, sv3, QIcon::fromTheme(QLatin1String("folder-pictures")), i18n("Target"));

    // --------------------------------------------------------

    QScrollArea* const sv2   = new QScrollArea(this);
    DVBox* const vbox2       = new DVBox(sv2->viewport());
    sv2->setWidget(vbox2);
    sv2->setWidgetResizable(true);

    d->renamingButtonGroup   = new QButtonGroup(vbox2);
    d->renameOriginal        = new QRadioButton(i18n("Use original filenames"), vbox2);
    d->renameOriginal->setWhatsThis(i18n("Turn on this option to use original "
                                         "filenames without modifications."));

    d->renameManual          = new QRadioButton(i18n("Customize filenames:"), vbox2);

    d->advancedRenameWidget  = new AdvancedRenameWidget(vbox2);
    d->advancedRenameManager = new AdvancedRenameManager();
    d->advancedRenameManager->setWidget(d->advancedRenameWidget);

    QWidget* const space     = new QWidget(vbox2);

    d->renamingButtonGroup->setExclusive(true);
    d->renamingButtonGroup->addButton(d->renameOriginal, QueueSettings::USEORIGINAL);
    d->renamingButtonGroup->addButton(d->renameManual,   QueueSettings::CUSTOMIZE);

    vbox2->setStretchFactor(space, 10);
    vbox2->setContentsMargins(spacing, spacing, spacing, spacing);
    vbox2->setSpacing(spacing);

    insertTab(Private::RENAMING, sv2, QIcon::fromTheme(QLatin1String("insert-image")), i18n("File Renaming"));

    // --------------------------------------------------------

    QScrollArea* const sv     = new QScrollArea(this);
    QWidget* const panel      = new QWidget(sv->viewport());
    QVBoxLayout* const layout = new QVBoxLayout(panel);
    sv->setWidget(panel);
    sv->setWidgetResizable(true);

    // --------------------------------------------------------

    d->rawLoadingLabel           = new QLabel(i18n("Raw Files Loading:"), panel);
    QWidget* const rawLoadingBox = new QWidget(panel);
    QVBoxLayout* const vlay2     = new QVBoxLayout(rawLoadingBox);
    d->rawLoadingButtonGroup     = new QButtonGroup(rawLoadingBox);
    d->demosaicingButton         = new QRadioButton(i18n("Perform RAW demosaicing"),           rawLoadingBox);
    d->extractJPEGButton         = new QRadioButton(i18n("Extract embedded preview (faster)"), rawLoadingBox);
    d->rawLoadingButtonGroup->addButton(d->extractJPEGButton, QueueSettings::USEEMBEDEDJPEG);
    d->rawLoadingButtonGroup->addButton(d->demosaicingButton, QueueSettings::DEMOSAICING);
    d->rawLoadingButtonGroup->setExclusive(true);
    d->demosaicingButton->setChecked(true);

    vlay2->addWidget(d->demosaicingButton);
    vlay2->addWidget(d->extractJPEGButton);
    vlay2->setContentsMargins(QMargins());
    vlay2->setSpacing(0);

    // -------------

    d->conflictBox    = new FileSaveConflictBox(panel, true);

    d->asNewVersion   = new QCheckBox(i18nc("@option:check", "Save image as a newly created branch"), panel);
    d->asNewVersion->setWhatsThis(i18n("Turn on this option to save the current modifications "
                                       "to a new version of the file"));

    d->useMutiCoreCPU = new QCheckBox(i18nc("@option:check", "Uses all processors"), panel);
    d->useMutiCoreCPU->setWhatsThis(i18n("Turn on this option to use all CPU core from your computer "
                                         "to process more than one item from a queue at the same time."));
    // -------------

    layout->addWidget(d->rawLoadingLabel);
    layout->addWidget(rawLoadingBox);
    layout->addWidget(d->conflictBox);
    layout->addWidget(d->asNewVersion);
    layout->addWidget(d->useMutiCoreCPU);
    layout->setContentsMargins(spacing, spacing, spacing, spacing);
    layout->setSpacing(spacing);
    layout->addStretch();

    insertTab(Private::BEHAVIOR, sv, QIcon::fromTheme(QLatin1String("dialog-information")), i18n("Behavior"));

    // --------------------------------------------------------

    d->rawSettings = new DRawDecoderWidget(panel, DRawDecoderWidget::SIXTEENBITS | DRawDecoderWidget::COLORSPACE);
    d->rawSettings->setItemIcon(0, QIcon::fromTheme(QLatin1String("image-x-adobe-dng")));
    d->rawSettings->setItemIcon(1, QIcon::fromTheme(QLatin1String("bordertool")));
    d->rawSettings->setItemIcon(2, QIcon::fromTheme(QLatin1String("zoom-draw")));

    insertTab(Private::RAW, d->rawSettings, QIcon::fromTheme(QLatin1String("image-x-adobe-dng")), i18n("Raw Decoding"));

    // --------------------------------------------------------

    QScrollArea* const sv4       = new QScrollArea(this);
    QWidget* const spanel        = new QWidget(sv4->viewport());
    QVBoxLayout* const slay      = new QVBoxLayout(spanel);
    sv4->setWidget(spanel);
    sv4->setWidgetResizable(true);
    DPluginLoader* const ploader = DPluginLoader::instance();

    d->jpgSettings               = ploader->exportWidget(QLatin1String("JPEG"));

    if (d->jpgSettings)
    {
        QGroupBox* const  box1   = new QGroupBox;
        QVBoxLayout* const lbox1 = new QVBoxLayout;
        d->jpgSettings->setParent(this);
        lbox1->addWidget(d->jpgSettings);
        box1->setLayout(lbox1);
        slay->addWidget(box1);
    }

    d->pngSettings               = ploader->exportWidget(QLatin1String("PNG"));

    if (d->pngSettings)
    {
        QGroupBox* const  box2   = new QGroupBox;
        QVBoxLayout* const lbox2 = new QVBoxLayout;
        d->pngSettings->setParent(this);;
        lbox2->addWidget(d->pngSettings);
        box2->setLayout(lbox2);
        slay->addWidget(box2);
    }

    d->tifSettings               = ploader->exportWidget(QLatin1String("TIFF"));

    if (d->tifSettings)
    {
        QGroupBox* const  box3   = new QGroupBox;
        QVBoxLayout* const lbox3 = new QVBoxLayout;
        d->tifSettings->setParent(this);
        lbox3->addWidget(d->tifSettings);
        box3->setLayout(lbox3);
        slay->addWidget(box3);
    }

#ifdef HAVE_JASPER

    d->j2kSettings               = ploader->exportWidget(QLatin1String("JP2"));

    if (d->j2kSettings)
    {
        QGroupBox* const  box4   = new QGroupBox;
        QVBoxLayout* const lbox4 = new QVBoxLayout;
        d->j2kSettings->setParent(this);
        lbox4->addWidget(d->j2kSettings);
        box4->setLayout(lbox4);
        slay->addWidget(box4);
    }

#endif // HAVE_JASPER

    d->pgfSettings               = ploader->exportWidget(QLatin1String("PGF"));

    if (d->pgfSettings)
    {
        QGroupBox* const  box5   = new QGroupBox;
        QVBoxLayout* const lbox5 = new QVBoxLayout;
        d->pgfSettings->setParent(this);
        lbox5->addWidget(d->pgfSettings);
        box5->setLayout(lbox5);
        slay->addWidget(box5);
    }

#ifdef HAVE_X265

    d->heifSettings              = ploader->exportWidget(QLatin1String("HEIF"));

    if (d->heifSettings)
    {
        QGroupBox* const  box6   = new QGroupBox;
        QVBoxLayout* const lbox6 = new QVBoxLayout;
        d->heifSettings->setParent(this);
        lbox6->addWidget(d->heifSettings);
        box6->setLayout(lbox6);
        slay->addWidget(box6);
    }

#endif // HAVE_X265

    d->jxlSettings               = ploader->exportWidget(QLatin1String("JXL"));

    if (d->jxlSettings)
    {
        QGroupBox* const  box7   = new QGroupBox;
        QVBoxLayout* const lbox7 = new QVBoxLayout;
        d->jxlSettings->setParent(this);
        lbox7->addWidget(d->jxlSettings);
        box7->setLayout(lbox7);
        slay->addWidget(box7);
    }

    d->webpSettings              = ploader->exportWidget(QLatin1String("WEBP"));

    if (d->webpSettings)
    {
        QGroupBox* const  box8   = new QGroupBox;
        QVBoxLayout* const lbox8 = new QVBoxLayout;
        d->webpSettings->setParent(this);
        lbox8->addWidget(d->webpSettings);
        box8->setLayout(lbox8);
        slay->addWidget(box8);
    }

    d->avifSettings              = ploader->exportWidget(QLatin1String("AVIF"));

    if (d->avifSettings)
    {
        QGroupBox* const  box9   = new QGroupBox;
        QVBoxLayout* const lbox9 = new QVBoxLayout;
        d->avifSettings->setParent(this);
        lbox9->addWidget(d->avifSettings);
        box9->setLayout(lbox9);
        slay->addWidget(box9);
    }

    slay->setContentsMargins(spacing, spacing, spacing, spacing);
    slay->setSpacing(spacing);
    slay->addStretch();

    insertTab(Private::SAVE, sv4, QIcon::fromTheme(QLatin1String("document-save-all")),
              i18n("Saving Images"));

    // --------------------------------------------------------

    connect(d->useOrgAlbum, SIGNAL(toggled(bool)),
            this, SLOT(slotUseOrgAlbum()));

    connect(d->asNewVersion, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useMutiCoreCPU, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->albumSel, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSettingsChanged()));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))

    connect(d->renamingButtonGroup, SIGNAL(idClicked(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->rawLoadingButtonGroup, SIGNAL(idClicked(int)),
            this, SLOT(slotSettingsChanged()));

#else

    connect(d->renamingButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->rawLoadingButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotSettingsChanged()));

#endif

    connect(d->conflictBox, SIGNAL(signalConflictButtonChanged(int)),
            this, SLOT(slotSettingsChanged()));


    connect(d->advancedRenameWidget, SIGNAL(signalTextChanged(QString)),
            this, SLOT(slotSettingsChanged()));

    connect(d->rawSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->jpgSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->pngSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->tifSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

#ifdef HAVE_JASPER

    if (d->j2kSettings)
    {
        connect(d->j2kSettings, SIGNAL(signalSettingsChanged()),
                this, SLOT(slotSettingsChanged()));
    }

#endif // HAVE_JASPER

#ifdef HAVE_X265

    if (d->heifSettings)
    {
        connect(d->heifSettings, SIGNAL(signalSettingsChanged()),
                this, SLOT(slotSettingsChanged()));
    }

#endif // HAVE_X265

    connect(d->pgfSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->jxlSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->webpSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->avifSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    // --------------------------------------------------------

    QTimer::singleShot(0, this, SLOT(slotResetSettings()));
}

QueueSettingsView::~QueueSettingsView()
{
    delete d->advancedRenameManager;
    delete d;
}

void QueueSettingsView::setBusy(bool b)
{
    for (int i = 0 ; i < count() ; ++i)
    {
        widget(i)->setEnabled(!b);
    }
}

void QueueSettingsView::slotUseOrgAlbum()
{
    if (!d->useOrgAlbum->isChecked())
    {
        PAlbum* const album = AlbumManager::instance()->currentPAlbum();

        if (album)
        {
            blockSignals(true);
            d->albumSel->setCurrentAlbum(album);
            blockSignals(false);
        }
    }

    slotSettingsChanged();
}

void QueueSettingsView::slotResetSettings()
{
    DImgLoaderPrms set;

    blockSignals(true);
    d->useOrgAlbum->setChecked(true);
    d->asNewVersion->setChecked(true);
    d->useMutiCoreCPU->setChecked(false);

    // TODO: reset d->albumSel

    d->renamingButtonGroup->button(QueueSettings::USEORIGINAL)->setChecked(true);
    d->conflictBox->setConflictRule(FileSaveConflictBox::DIFFNAME);
    d->rawLoadingButtonGroup->button(QueueSettings::DEMOSAICING)->setChecked(true);
    d->advancedRenameWidget->clearParseString();
    d->rawSettings->resetToDefault();

    QueueSettings settings;

    if (d->jpgSettings)
    {
        set.clear();
        set.insert(QLatin1String("quality"),     settings.ioFileSettings.JPEGCompression);
        set.insert(QLatin1String("subsampling"), settings.ioFileSettings.JPEGSubSampling);
        d->jpgSettings->setSettings(set);
    }

    // ---

    if (d->pngSettings)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  settings.ioFileSettings.PNGCompression);
        d->pngSettings->setSettings(set);
    }

    // ---

    if (d->tifSettings)
    {
        set.clear();
        set.insert(QLatin1String("compress"),  settings.ioFileSettings.TIFFCompression);
        d->tifSettings->setSettings(set);
    }

#ifdef HAVE_JASPER

    if (d->j2kSettings)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  settings.ioFileSettings.JPEG2000Compression);
        set.insert(QLatin1String("lossless"), settings.ioFileSettings.JPEG2000LossLess);
        d->j2kSettings->setSettings(set);
    }

#endif // HAVE_JASPER

    if (d->pgfSettings)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  settings.ioFileSettings.PGFCompression);
        set.insert(QLatin1String("lossless"), settings.ioFileSettings.PGFLossLess);
        d->pgfSettings->setSettings(set);
    }

#ifdef HAVE_X265

    if (d->heifSettings)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  settings.ioFileSettings.HEIFCompression);
        set.insert(QLatin1String("lossless"), settings.ioFileSettings.HEIFLossLess);
        d->heifSettings->setSettings(set);
    }

#endif // HAVE_X265

    if (d->jxlSettings)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  settings.ioFileSettings.JXLCompression);
        set.insert(QLatin1String("lossless"), settings.ioFileSettings.JXLLossLess);
        d->jxlSettings->setSettings(set);
    }

    if (d->webpSettings)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  settings.ioFileSettings.WEBPCompression);
        set.insert(QLatin1String("lossless"), settings.ioFileSettings.WEBPLossLess);
        d->webpSettings->setSettings(set);
    }

    if (d->avifSettings)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  settings.ioFileSettings.AVIFCompression);
        set.insert(QLatin1String("lossless"), settings.ioFileSettings.AVIFLossLess);
        d->avifSettings->setSettings(set);
    }

    blockSignals(false);
    slotSettingsChanged();
}

void QueueSettingsView::slotQueueSelected(int, const QueueSettings& settings, const AssignedBatchTools&)
{
    DImgLoaderPrms set;

    d->useOrgAlbum->setChecked(settings.useOrgAlbum);
    d->asNewVersion->setChecked(settings.saveAsNewVersion);
    d->useMutiCoreCPU->setChecked(settings.useMultiCoreCPU);
    d->albumSel->setEnabled(!settings.useOrgAlbum);
    d->albumSel->setCurrentAlbumUrl(settings.workingUrl);

    int btn = (int)settings.renamingRule;
    d->renamingButtonGroup->button(btn)->setChecked(true);

    d->conflictBox->setConflictRule(settings.conflictRule);

    btn     = (int)settings.rawLoadingRule;
    d->rawLoadingButtonGroup->button(btn)->setChecked(true);

    d->advancedRenameWidget->setParseString(settings.renamingParser);

    d->rawSettings->setSettings(settings.rawDecodingSettings);

    // ---

    set.clear();
    set.insert(QLatin1String("quality"),     settings.ioFileSettings.JPEGCompression);
    set.insert(QLatin1String("subsampling"), settings.ioFileSettings.JPEGSubSampling);
    d->jpgSettings->setSettings(set);

    // ---

    set.clear();
    set.insert(QLatin1String("quality"),  settings.ioFileSettings.PNGCompression);
    d->pngSettings->setSettings(set);

    // ---

    set.clear();
    set.insert(QLatin1String("compress"),  settings.ioFileSettings.TIFFCompression);
    d->tifSettings->setSettings(set);

    set.clear();
    set.insert(QLatin1String("quality"),  settings.ioFileSettings.PGFCompression);
    set.insert(QLatin1String("lossless"), settings.ioFileSettings.PGFLossLess);
    d->pgfSettings->setSettings(set);

#ifdef HAVE_JASPER

    if (d->j2kSettings)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  settings.ioFileSettings.JPEG2000Compression);
        set.insert(QLatin1String("lossless"), settings.ioFileSettings.JPEG2000LossLess);
        d->j2kSettings->setSettings(set);
    }

#endif // HAVE_JASPER

#ifdef HAVE_X265

    if (d->heifSettings)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  settings.ioFileSettings.HEIFCompression);
        set.insert(QLatin1String("lossless"), settings.ioFileSettings.HEIFLossLess);
        d->heifSettings->setSettings(set);
    }

#endif // HAVE_X265

    if (d->jxlSettings)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  settings.ioFileSettings.JXLCompression);
        set.insert(QLatin1String("lossless"), settings.ioFileSettings.JXLLossLess);
        d->jxlSettings->setSettings(set);
    }

    if (d->webpSettings)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  settings.ioFileSettings.WEBPCompression);
        set.insert(QLatin1String("lossless"), settings.ioFileSettings.WEBPLossLess);
        d->webpSettings->setSettings(set);
    }

    if (d->avifSettings)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  settings.ioFileSettings.AVIFCompression);
        set.insert(QLatin1String("lossless"), settings.ioFileSettings.AVIFLossLess);
        d->avifSettings->setSettings(set);
    }
}

void QueueSettingsView::slotSettingsChanged()
{
    QueueSettings settings;

    d->albumSel->setEnabled(!d->useOrgAlbum->isChecked());
    settings.useOrgAlbum         = d->useOrgAlbum->isChecked();
    settings.saveAsNewVersion    = d->asNewVersion->isChecked();
    settings.useMultiCoreCPU     = d->useMutiCoreCPU->isChecked();
    settings.workingUrl          = d->albumSel->currentAlbumUrl();

    settings.renamingRule        = (QueueSettings::RenamingRule)d->renamingButtonGroup->checkedId();
    settings.renamingParser      = d->advancedRenameWidget->parseString();
    d->advancedRenameWidget->setEnabled(settings.renamingRule == QueueSettings::CUSTOMIZE);

    settings.conflictRule        = d->conflictBox->conflictRule();

    settings.rawLoadingRule      = (QueueSettings::RawLoadingRule)d->rawLoadingButtonGroup->checkedId();
    setTabEnabled(Private::RAW, (settings.rawLoadingRule == QueueSettings::DEMOSAICING));

    settings.rawDecodingSettings = d->rawSettings->settings();

    // ---

    if (d->jpgSettings)
    {
        settings.ioFileSettings.JPEGCompression     = d->jpgSettings->settings().value(QLatin1String("quality")).toInt();
        settings.ioFileSettings.JPEGSubSampling     = d->jpgSettings->settings().value(QLatin1String("subsampling")).toInt();
    }

    // ---

    if (d->pngSettings)
    {
        settings.ioFileSettings.PNGCompression      = d->pngSettings->settings().value(QLatin1String("quality")).toInt();
    }

    // ---

    if (d->tifSettings)
    {
        settings.ioFileSettings.TIFFCompression     = d->tifSettings->settings().value(QLatin1String("compress")).toBool();
    }

#ifdef HAVE_JASPER

    if (d->j2kSettings)
    {
        settings.ioFileSettings.JPEG2000Compression = d->j2kSettings->settings().value(QLatin1String("quality")).toInt();
        settings.ioFileSettings.JPEG2000LossLess    = d->j2kSettings->settings().value(QLatin1String("lossless")).toBool();
    }

#endif // HAVE_JASPER

    if (d->pgfSettings)
    {
        settings.ioFileSettings.PGFCompression      = d->pgfSettings->settings().value(QLatin1String("quality")).toInt();
        settings.ioFileSettings.PGFLossLess         = d->pgfSettings->settings().value(QLatin1String("lossless")).toBool();
    }

#ifdef HAVE_X265

    if (d->heifSettings)
    {
        settings.ioFileSettings.HEIFCompression     = d->heifSettings->settings().value(QLatin1String("quality")).toInt();
        settings.ioFileSettings.HEIFLossLess        = d->heifSettings->settings().value(QLatin1String("lossless")).toBool();
    }

#endif // HAVE_X265

    if (d->jxlSettings)
    {
        settings.ioFileSettings.JXLCompression  = d->jxlSettings->settings().value(QLatin1String("quality")).toInt();
        settings.ioFileSettings.JXLLossLess     = d->jxlSettings->settings().value(QLatin1String("lossless")).toBool();
    }

    if (d->webpSettings)
    {
        settings.ioFileSettings.WEBPCompression  = d->webpSettings->settings().value(QLatin1String("quality")).toInt();
        settings.ioFileSettings.WEBPLossLess     = d->webpSettings->settings().value(QLatin1String("lossless")).toBool();
    }

    if (d->avifSettings)
    {
        settings.ioFileSettings.AVIFCompression = d->avifSettings->settings().value(QLatin1String("quality")).toInt();
        settings.ioFileSettings.AVIFLossLess    = d->avifSettings->settings().value(QLatin1String("lossless")).toBool();
    }

    Q_EMIT signalSettingsChanged(settings);
}

} // namespace Digikam

#include "moc_queuesettingsview.cpp"
