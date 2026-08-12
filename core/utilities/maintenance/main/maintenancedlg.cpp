/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-30
 * Description : maintenance dialog
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2025      by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "maintenancedlg_p.h"

// Qt includes

#include <QMessageBox>

// local includes

#include "autotagsscansettings.h"
#include "systemsettings.h"

namespace Digikam
{

MaintenanceDlg::MaintenanceDlg(QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    setWindowFlags((windowFlags() & ~Qt::Dialog) |
                   Qt::Window                    |
                   Qt::WindowCloseButtonHint     |
                   Qt::WindowMinMaxButtonsHint);

    setWindowTitle(i18nc("@title:window", "Maintenance"));
    setModal(true);

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Cancel)->setDefault(true);

    QScrollArea* const main    = new QScrollArea(this);
    QWidget* const page        = new QWidget(main->viewport());
    main->setWidget(page);
    main->setWidgetResizable(true);

    QGridLayout* const grid    = new QGridLayout(page);

    d->logo                    = new QLabel(page);
    d->logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48)));
    d->title                   = new QLabel(i18n("<qt><b>Select Maintenance Operations to Process</b></qt>"), page);
    d->expanderBox             = new DExpanderBox(page);

    // --------------------------------------------------------------------------------------

    DVBox* const options       = new DVBox;
    d->albumSelectors          = new AlbumSelectors(i18nc("@label", "Process items from:"),
                                                    d->configGroupName, options, AlbumSelectors::AlbumType::All, true);
    d->useLastSettings         = new QCheckBox(i18nc("@option:check", "Use the last saved active tools and settings"), options);
    d->useMutiCoreCPU          = new QCheckBox(i18nc("@option:check", "Work on all processor cores (when it is possible)"), options);

    d->expanderBox->insertItem(
                               Private::Options,
                               options,
                               QIcon::fromTheme(QLatin1String("configure")),
                               i18n("Common Options"),
                               QLatin1String("Options"),
                               true
                              );

    // --------------------------------------------------------------------------------------

    d->expanderBox->insertItem(
                               Private::NewItems,
                               new QLabel(i18n("<qt>No option<br>"
                                               "<i>Note: only Albums Collection are processed by this tool.</i></qt>")),
                               QIcon::fromTheme(QLatin1String("view-refresh")),
                               i18n("Scan for new items"),
                               QLatin1String("NewItems"),
                               false
                              );

    d->expanderBox->setCheckBoxVisible(Private::NewItems, true);

    // --------------------------------------------------------------------------------------

    d->vbox3                   = new DVBox;
    new QLabel(i18n("<qt><i>Note: If activated, the core database is always cleaned. "
                    "You can select additional databases for cleaning.<br>"
                    "If you select one of the options below, the process may take "
                    "much time and can freeze digiKam temporarily "
                    "in order to make sure that no database corruption occurs.</i></qt>"), d->vbox3);
    d->cleanThumbsDb           = new QCheckBox(i18n("Also clean up the thumbnail database."), d->vbox3);
    d->cleanFacesDb            = new QCheckBox(i18n("Also clean up the faces database."), d->vbox3);
    d->cleanSimilarityDb       = new QCheckBox(i18n("Also clean up the similarity database."), d->vbox3);
    d->shrinkDatabases         = new QCheckBox(i18n("Extended clean up and shrink all databases."), d->vbox3);
    d->shrinkDatabases->setToolTip(i18n("This option leads to the vacuuming (shrinking) of the databases. "
                                        "Vacuuming is supported both for SQLite and MySQL."));

    d->expanderBox->insertItem(
                               Private::DbCleanup,
                               d->vbox3,
                               QIcon::fromTheme(QLatin1String("run-build")),
                               i18n("Perform database cleaning"),
                               QLatin1String("DbCleanup"),
                               false
                              );

    d->expanderBox->setCheckBoxVisible(Private::DbCleanup, true);

    // --------------------------------------------------------------------------------------

    d->scanThumbs              = new QCheckBox(i18n("Scan for changed or non-cataloged items (faster)"));

    d->expanderBox->insertItem(
                               Private::Thumbnails,
                               d->scanThumbs,
                               QIcon::fromTheme(QLatin1String("photo")),
                               i18n("Rebuild Thumbnails"),
                               QLatin1String("Thumbnails"),
                               false
                              );

    d->expanderBox->setCheckBoxVisible(Private::Thumbnails, true);

    // --------------------------------------------------------------------------------------

    d->scanFingerPrints        = new QCheckBox(i18n("Scan for changed or non-cataloged items (faster)"));

    d->expanderBox->insertItem(
                               Private::FingerPrints,
                               d->scanFingerPrints,
                               QIcon::fromTheme(QLatin1String("fingerprint")),
                               i18n("Rebuild Finger-prints"),
                               QLatin1String("Fingerprints"),
                               false
                              );

    d->expanderBox->setCheckBoxVisible(Private::FingerPrints, true);

    // --------------------------------------------------------------------------------------

    const ApplicationSettings* const config   = ApplicationSettings::instance();

    d->duplicatesBox                          = new DVBox;
    d->similarityRangeBox                     = new DHBox(d->duplicatesBox);
    new QLabel(i18n("Similarity range (in percents): "), d->similarityRangeBox);
    QWidget* const space                      = new QWidget(d->similarityRangeBox);
    d->similarityRangeBox->setStretchFactor(space, 10);

    d->similarityRange                        = new DIntRangeBox(d->similarityRangeBox);
    d->similarityRange->setSuffix(QLatin1String("%"));

    if (config)
    {
        d->similarityRange->setRange(config->getMinimumSimilarityBound(), 100);
        d->similarityRange->setInterval(config->getDuplicatesSearchLastMinSimilarity(),
                                        config->getDuplicatesSearchLastMaxSimilarity());
    }
    else
    {
        d->similarityRange->setRange(40, 100);
        d->similarityRange->setInterval(90, 100);
    }

    d->dupeRestrictionBox      = new DHBox(d->duplicatesBox);
    new QLabel(i18n("Restriction on duplicates:"), d->dupeRestrictionBox);
    QWidget* const space4      = new QWidget(d->dupeRestrictionBox);
    d->dupeRestrictionBox->setStretchFactor(space4, 10);
    d->searchResultRestriction = new QComboBox(d->dupeRestrictionBox);
    d->searchResultRestriction->addItem(i18n("No restriction"),
                                        HaarIface::DuplicatesSearchRestrictions::None);
    d->searchResultRestriction->addItem(i18n("Restrict to album of reference image"),
                                        HaarIface::DuplicatesSearchRestrictions::SameAlbum);
    d->searchResultRestriction->addItem(i18n("Exclude album of reference image"),
                                        HaarIface::DuplicatesSearchRestrictions::DifferentAlbum);

    // Load the last choice from application settings.

    HaarIface::DuplicatesSearchRestrictions restrictions = HaarIface::DuplicatesSearchRestrictions::None;

    if (config)
    {
        restrictions = (HaarIface::DuplicatesSearchRestrictions) config->getDuplicatesSearchRestrictions();
    }

    d->searchResultRestriction->setCurrentIndex(d->searchResultRestriction->findData(restrictions));

    d->expanderBox->insertItem(
                               Private::Duplicates,
                               d->duplicatesBox,
                               QIcon::fromTheme(QLatin1String("tools-wizard")),
                               i18n("Find Duplicate Items"),
                               QLatin1String("Duplicates"),
                               false
                              );

    d->expanderBox->setCheckBoxVisible(Private::Duplicates, true);

    // --------------------------------------------------------------------------------------

    d->vbox4               = new DVBox;
    d->retrainAllFaces     = new QCheckBox(d->vbox4);
    d->retrainAllFaces->setText(i18nc("@option:check", "Rebuild all training data"));
    d->retrainAllFaces->setToolTip(i18nc("@info:tooltip",
                                         "This will re-train the face recognizer with tagged faces in your library."));

    d->resetFaceDb         = new QCheckBox(d->vbox4);
    d->resetFaceDb->setText(i18nc("@option:check", "Reset and clear all faces and training"));
    d->resetFaceDb->setToolTip(i18nc("@info:tooltip",
                                         "This will clear all detected and tagged faces."));

    d->clearRejectedFaces  = new QCheckBox(d->vbox4);
    d->clearRejectedFaces->setText(i18nc("@option:check", "Clear rejected face matches"));
    d->clearRejectedFaces->setToolTip(i18nc("@info:tooltip",
                                         "This will delete all rejected face matches allowing the "
                                         "face recognizer to present the suggested matches again."));

    d->expanderBox->insertItem(
                               Private::FaceManagement,
                               d->vbox4,
                               QIcon::fromTheme(QLatin1String("edit-image-face-detect")),
                               i18n("Face engine management"),
                               QLatin1String("FaceManagement"),
                               false
                              );

    d->expanderBox->setCheckBoxVisible(Private::FaceManagement, true);

    // disable face management if the setting is not enabled

    if (!SystemSettings(qApp->applicationName()).enableFaceEngine)
    {
        d->expanderBox->setItemExpanded(Private::FaceManagement, false);
        d->expanderBox->setItemEnabled(Private::FaceManagement, false);
    }

    // --------------------------------------------------------------------------------------

    d->vbox5               = new DVBox;
    d->autotagsWidget      = new AutotagsScanWidget(AutotagsScanWidget::Maintenance, d->vbox5);
    d->expanderBox->insertItem(
                               Private::AutotagsAssignment,
                               d->vbox5,
                               QIcon::fromTheme(QLatin1String("tag")),
                               i18n("Auto-tags Assignment"),
                               QLatin1String("AutotagsAssignment"),
                               false
                              );

    d->expanderBox->setCheckBoxVisible(Private::AutotagsAssignment, true);

    // disable auto-tags assignment if the setting is not enabled

    if (!SystemSettings(qApp->applicationName()).enableAutoTags)
    {
        d->expanderBox->setItemExpanded(Private::AutotagsAssignment, false);
        d->expanderBox->setItemEnabled(Private::AutotagsAssignment, false);
    }

    // --------------------------------------------------------------------------------------

    d->vbox               = new DVBox;
    d->qualityWidget      = new ImageQualityWidget(ImageQualityWidget::Maintenance, d->vbox);

    d->expanderBox->insertItem(
                               Private::ImageQualitySorter,
                               d->vbox,
                               QIcon::fromTheme(QLatin1String("flag-green")),
                               i18n("Image Quality Scanner"),
                               QLatin1String("ImageQualitySorter"),
                               false
                              );

    d->expanderBox->setCheckBoxVisible(Private::ImageQualitySorter, true);

    // --------------------------------------------------------------------------------------

    d->vbox2              = new DVBox;
    DHBox* const hbox21   = new DHBox(d->vbox2);
    new QLabel(i18n("Sync Direction: "), hbox21);
    QWidget* const space5 = new QWidget(hbox21);
    hbox21->setStretchFactor(space5, 10);
    d->syncDirection      = new QComboBox(hbox21);
    d->syncDirection->addItem(i18n("From database to image metadata"),
                              MetadataSynchronizer::WriteFromDatabaseToFile);
    d->syncDirection->addItem(i18n("From image metadata to database"),
                              MetadataSynchronizer::ReadFromFileToDatabase);

    DHBox* const hbox22   = new DHBox(d->vbox2);
    new QLabel(i18n("Check metadata setup panel for details: "), hbox22);
    QWidget* const space6 = new QWidget(hbox22);
    hbox22->setStretchFactor(space6, 10);
    d->metadataSetup      = new QPushButton(i18n("Settings..."), hbox22);

    d->expanderBox->insertItem(
                               Private::MetadataSync,
                               d->vbox2,
                               QIcon::fromTheme(QLatin1String("run-build-file")),
                               i18n("Sync Metadata and Database"),
                               QLatin1String("MetadataSync"),
                               false
                              );

    d->expanderBox->setCheckBoxVisible(Private::MetadataSync, true);

    d->expanderBox->insertStretch(Private::Stretch);

    // --------------------------------------------------------------------------------------

    grid->addWidget(d->logo,        0, 0, 1, 1);
    grid->addWidget(d->title,       0, 1, 1, 1);
    grid->addWidget(d->expanderBox, 5, 0, 3, 2);
    grid->setSpacing(layoutSpacing());
    grid->setContentsMargins(QMargins());
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(5, 10);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(main);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    // --------------------------------------------------------------------------------------

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotOk()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    connect(d->expanderBox, SIGNAL(signalItemToggled(int,bool)),
            this, SLOT(slotItemToggled(int,bool)));

    connect(d->useLastSettings, SIGNAL(toggled(bool)),
            this, SLOT(slotUseLastSettings(bool)));

    connect(d->metadataSetup, SIGNAL(clicked()),
            this, SLOT(slotMetadataSetup()));

    connect(d->resetFaceDb, &QCheckBox::toggled,
            this, [this] (bool on)
        {
            if (on)
            {
                int mbResult = QMessageBox(
                                           QMessageBox::Warning,
                                           i18nc("@title: window reset face database", "Warning"),
                                           i18n("Are you sure you want to delete all confirmed, "
                                                "unconfirmed, and unknown faces? "
                                                "You’ll need to rescan images and start tagging "
                                                "unknown faces again before any faces can be recognized."),
                                           QMessageBox::Ok | QMessageBox::Cancel,
                                           this
                                           ).exec();

                if (QMessageBox::Cancel == mbResult)
                {
                    d->resetFaceDb->setChecked(false);
                }
            }

            d->retrainAllFaces->setEnabled(!(Qt::Checked == d->resetFaceDb->checkState()));
            d->clearRejectedFaces->setEnabled(!(Qt::Checked == d->resetFaceDb->checkState()));
        }
    );

    // --------------------------------------------------------------------------------------

    readSettings();
}

MaintenanceDlg::~MaintenanceDlg()
{
    delete d;
}

void MaintenanceDlg::slotOk()
{
    writeSettings();
    accept();
}

void MaintenanceDlg::slotItemToggled(int index, bool b)
{
    switch (index)
    {
        case Private::Thumbnails:
        {
            d->scanThumbs->setEnabled(b);
            break;
        }

        case Private::FingerPrints:
        {
            d->scanFingerPrints->setEnabled(b);
            break;
        }

        case Private::Duplicates:
        {
            d->duplicatesBox->setEnabled(b);
            break;
        }

        case Private::FaceManagement:
        {
            d->vbox4->setEnabled(b);
            break;
        }

        case Private::ImageQualitySorter:
        {
            d->vbox->setEnabled(b);
            break;
        }

        case Private::AutotagsAssignment:
        {
            d->vbox5->setEnabled(b);
            break;
        }

        case Private::MetadataSync:
        {
            d->vbox2->setEnabled(b);
            break;
        }

        case Private::DbCleanup:
        {
            d->vbox3->setEnabled(b);
            break;
        }

        default :  // NewItems
        {
            break;
        }
    }
}

void MaintenanceDlg::slotMetadataSetup()
{
    Setup::execSinglePage(this, Setup::MetadataPage);
}

void MaintenanceDlg::slotHelp()
{
    openOnlineDocumentation(QLatin1String("maintenance_tools"));
}

void MaintenanceDlg::showEvent(QShowEvent* e)
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    DXmlGuiWindow::setGoodDefaultWindowSize(windowHandle());
    DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());

    QDialog::showEvent(e);
}

} // namespace Digikam

#include "moc_maintenancedlg.cpp"
