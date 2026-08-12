/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Fuzzy search sidebar tab contents.
 *
 * SPDX-FileCopyrightText: 2016-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fuzzysearchview_p.h"

namespace Digikam
{

FuzzySearchView::FuzzySearchView(SearchModel* const searchModel,
                                 SearchModificationHelper* const searchModificationHelper,
                                 QWidget* const parent)
    : QScrollArea      (parent),
      StateSavingObject(this),
      d                (new Private)
{
    d->settings                 = ApplicationSettings::instance();

    const int spacing           = layoutSpacing();


    d->thumbLoadThread          = ThumbnailLoadThread::defaultThread();
    d->searchModel              = searchModel;
    d->searchModificationHelper = searchModificationHelper;

    setWidgetResizable(true);
    setAttribute(Qt::WA_DeleteOnClose);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    // ---------------------------------------------------------------

    QWidget* const imagePanel  = setupFindSimilarPanel();
    QWidget* const sketchPanel = setupSketchPanel();
    d->findDuplicatesPanel     = new FindDuplicatesView();

    d->tabWidget               = new QTabWidget();
    d->tabWidget->insertTab(Private::DUPLICATES, d->findDuplicatesPanel, i18nc("Find Duplicates Tab", "Duplicates"));
    d->tabWidget->insertTab(Private::SIMILARS,   imagePanel,             i18nc("Similar Image Tab", "Image"));
    d->tabWidget->insertTab(Private::SKETCH,     sketchPanel,            i18nc("Sketch Search Tab", "Sketch"));

    // ---------------------------------------------------------------

    d->folderView     = new DVBox();
    d->searchTreeView = new EditableSearchTreeView(d->folderView, searchModel, searchModificationHelper);
    d->searchTreeView->filteredModel()->listHaarSearches();
    d->searchTreeView->filteredModel()->setListTemporarySearches(true);
    d->searchTreeView->setAlbumManagerCurrentAlbum(true);

    d->searchFuzzyBar = new SearchTextBarDb(d->folderView, QLatin1String("FuzzySearchViewSearchFuzzyBar"));
    d->searchFuzzyBar->setModel(d->searchTreeView->filteredModel(),
                                AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->searchFuzzyBar->setFilterModel(d->searchTreeView->albumFilterModel());
    d->folderView->setContentsMargins(QMargins());
    d->folderView->setSpacing(spacing);

    // ---------------------------------------------------------------

    QWidget* const mainWidget     = new QWidget(this);
    QVBoxLayout* const mainLayout = new QVBoxLayout();
    mainLayout->addWidget(d->tabWidget);
    mainLayout->addWidget(d->folderView);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->setSpacing(0);
    mainWidget->setLayout(mainLayout);

    setWidget(mainWidget);

    // ---------------------------------------------------------------

    d->timerSketch = new QTimer(this);
    d->timerSketch->setSingleShot(true);
    d->timerSketch->setInterval(500);

    d->timerImage  = new QTimer(this);
    d->timerImage->setSingleShot(true);
    d->timerImage->setInterval(500);

    // ---------------------------------------------------------------

    setupConnections();

    // ---------------------------------------------------------------

    slotCheckNameEditSketchConditions();
    slotCheckNameEditImageConditions();
}

FuzzySearchView::~FuzzySearchView()
{
    delete d;
}

void FuzzySearchView::setupConnections()
{
    connect(d->findDuplicatesPanel, SIGNAL(signalScanNotification(QString,int)),
            this, SIGNAL(signalNotificationError(QString,int)));

    connect(d->settings, SIGNAL(setupChanged()),
            this, SLOT(slotApplicationSettingsChanged()));

    connect(d->tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(slotTabChanged(int)));

    connect(d->searchTreeView, SIGNAL(currentAlbumChanged(Album*)),
            this, SLOT(slotAlbumSelected(Album*)));

    connect(d->hsSelector, SIGNAL(valueChanged(int,int)),
            this, SLOT(slotHSChanged(int,int)));

    connect(d->vSelector, SIGNAL(valueChanged(int)),
            this, SLOT(slotVChanged(int)));

    connect(d->penSize, SIGNAL(valueChanged(int)),
            d->sketchWidget, SLOT(setPenWidth(int)));

    connect(d->resultsSketch, SIGNAL(valueChanged(int)),
            this, SLOT(slotDirtySketch()));

    connect(d->sketchSearchAlbumSelectors, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotDirtySketch()));

    connect(d->fuzzySearchAlbumSelectors, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotFuzzyAlbumsChanged()));

    connect(d->similarityRange, SIGNAL(minChanged(int)),
            this, SLOT(slotMinLevelImageChanged(int)));

    connect(d->similarityRange, SIGNAL(maxChanged(int)),
            this, SLOT(slotMaxLevelImageChanged(int)));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotClearSketch()));

    connect(d->sketchWidget, SIGNAL(signalPenSizeChanged(int)),
            d->penSize, SLOT(setValue(int)));

    connect(d->sketchWidget, SIGNAL(signalPenColorChanged(QColor)),
            this, SLOT(slotPenColorChanged(QColor)));

    connect(d->sketchWidget, SIGNAL(signalSketchChanged(QImage)),
            this, SLOT(slotDirtySketch()));

    connect(d->sketchWidget, SIGNAL(signalUndoRedoStateChanged(bool,bool)),
            this, SLOT(slotUndoRedoStateChanged(bool,bool)));

    connect(d->undoBtnSketch, SIGNAL(clicked()),
            d->sketchWidget, SLOT(slotUndo()));

    connect(d->redoBtnSketch, SIGNAL(clicked()),
            d->sketchWidget, SLOT(slotRedo()));

    connect(d->saveBtnSketch, SIGNAL(clicked()),
            this, SLOT(slotSaveSketchSAlbum()));

    connect(d->saveBtnImage, SIGNAL(clicked()),
            this, SLOT(slotSaveImageSAlbum()));

    connect(d->nameEditSketch, SIGNAL(textChanged()),
            this, SLOT(slotCheckNameEditSketchConditions()));

    connect(d->nameEditSketch, SIGNAL(returnPressed()),
            d->saveBtnSketch, SLOT(animateClick()));

    connect(d->nameEditImage, SIGNAL(textChanged()),
            this, SLOT(slotCheckNameEditImageConditions()));

    connect(d->nameEditImage, SIGNAL(returnPressed()),
            d->saveBtnImage, SLOT(animateClick()));

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));

    connect(d->timerSketch, SIGNAL(timeout()),
            this, SLOT(slotTimerSketchDone()));

    connect(d->timerImage, SIGNAL(timeout()),
            this, SLOT(slotTimerImageDone()));
}

SAlbum* FuzzySearchView::currentAlbum() const
{
    return d->searchTreeView->currentAlbum();
}

void FuzzySearchView::setCurrentAlbum(SAlbum* const album)
{
    d->searchTreeView->setCurrentAlbums(QList<Album*>() << album);
}

void FuzzySearchView::newDuplicatesSearch(const QList<PAlbum*>& albums)
{
    if (!albums.isEmpty())
    {
        d->findDuplicatesPanel->slotSetSelectedAlbums(albums);
    }

    d->tabWidget->setCurrentIndex(Private::DUPLICATES);
}

void FuzzySearchView::newDuplicatesSearch(const QList<TAlbum*>& albums)
{
    if (!albums.isEmpty())
    {
        d->findDuplicatesPanel->slotSetSelectedAlbums(albums);
    }

    d->tabWidget->setCurrentIndex(Private::DUPLICATES);
}

void FuzzySearchView::setConfigGroup(const KConfigGroup& group)
{
    StateSavingObject::setConfigGroup(group);
    d->searchTreeView->setConfigGroup(group);
}

void FuzzySearchView::doLoadState()
{
    KConfigGroup group = getConfigGroup();

    d->tabWidget->setCurrentIndex(group.readEntry(entryName(d->configTabEntry),                    (int)Private::DUPLICATES));
    d->penSize->setValue(group.readEntry(entryName(d->configPenSketchSizeEntry),                   10));
    d->resultsSketch->setValue(group.readEntry(entryName(d->configResultSketchItemsEntry),         10));
    d->hsSelector->setHue(group.readEntry(entryName(d->configPenSketchHueEntry),                   180));
    d->hsSelector->setSaturation(group.readEntry(entryName(d->configPenSketchSaturationEntry),     128));
    d->vSelector->setValue(group.readEntry(entryName(d->configPenSkethValueEntry),                 255));
    d->similarityRange->setInterval(group.readEntry(entryName(d->configSimilarsThresholdEntry),    90),
                                    group.readEntry(entryName(d->configSimilarsMaxThresholdEntry), 100));
    d->hsSelector->updateContents();

    QColor col;
    col.setHsv(d->hsSelector->hue(),
               d->hsSelector->saturation(),
               d->vSelector->value());
    setColor(col);

    d->sketchWidget->setPenWidth(d->penSize->value());

    d->searchTreeView->loadState();
    d->fuzzySearchAlbumSelectors->loadState();
    d->sketchSearchAlbumSelectors->loadState();
}

void FuzzySearchView::doSaveState()
{
    KConfigGroup group = getConfigGroup();
    group.writeEntry(entryName(d->configTabEntry),                  d->tabWidget->currentIndex());
    group.writeEntry(entryName(d->configPenSketchSizeEntry),        d->penSize->value());
    group.writeEntry(entryName(d->configResultSketchItemsEntry),    d->resultsSketch->value());
    group.writeEntry(entryName(d->configPenSketchHueEntry),         d->hsSelector->hue());
    group.writeEntry(entryName(d->configPenSketchSaturationEntry),  d->hsSelector->saturation());
    group.writeEntry(entryName(d->configPenSkethValueEntry),        d->vSelector->value());
    group.writeEntry(entryName(d->configSimilarsThresholdEntry),    d->similarityRange->minValue());
    group.writeEntry(entryName(d->configSimilarsMaxThresholdEntry), d->similarityRange->maxValue());
    d->searchTreeView->saveState();
    group.sync();
    d->fuzzySearchAlbumSelectors->saveState();
    d->sketchSearchAlbumSelectors->saveState();
}

void FuzzySearchView::setActive(bool val)
{
    d->active = val;

    // at first occasion, warn if no fingerprints are available

    if (val && !d->fingerprintsChecked && isVisible())
    {
        if (!SimilarityDbAccess().db()->hasFingerprints())
        {
            QString msg = i18n("Image fingerprints have not yet been generated for your collection. "
                               "The Similarity Search Tools will not be operational "
                               "without pre-generated fingerprints. Please generate "
                               "the fingerprints first.");

            Q_EMIT signalNotificationError(msg, DNotificationWidget::Warning);
        }

        d->fingerprintsChecked = true;
    }

    int tab = d->tabWidget->currentIndex();

    if (val)
    {
        slotTabChanged(tab);
    }
}

void FuzzySearchView::slotTabChanged(int tab)
{
    /**
     * Set a list with only one element, albummanager can set only multiple albums.
     */
    QList<Album*> albums;

    switch (tab)
    {
        case Private::SIMILARS:
        {
            albums << d->imageSAlbum;
            AlbumManager::instance()->setCurrentAlbums(albums);
            d->folderView->setVisible(true);

            break;
        }

        case Private::SKETCH:
        {
            albums << d->sketchSAlbum;
            AlbumManager::instance()->setCurrentAlbums(albums);
            d->folderView->setVisible(true);

            break;
        }

        default:  // DUPLICATES
        {
            QList<SAlbum*> sAlbums = d->findDuplicatesPanel->currentFindDuplicatesAlbums();

            for (SAlbum* const album : std::as_const(sAlbums))      // cppcheck-suppress constVariablePointer
            {
                albums << album;
            }

            AlbumManager::instance()->setCurrentAlbums(albums);
            d->findDuplicatesPanel->populateTreeView();
            d->findDuplicatesPanel->setActive(true);
            d->folderView->setVisible(false);

            break;
        }
    }
}

void FuzzySearchView::slotAlbumSelected(Album* album)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Selected new album" << album;

    SAlbum* const salbum = dynamic_cast<SAlbum*>(album);

    if (!salbum || !salbum->isHaarSearch())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Not a haar search, returning";

        return;
    }

    if (!d->active)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Not active, returning";

        return;
    }

    SearchXmlReader reader(salbum->query());
    reader.readToFirstField();
    QStringView type               = reader.attributes().value(QLatin1String("type"));
/*
    TODO: apply this settings if necessary.

    QStringView numResultsString   = reader.attributes().value(QLatin1String("numberofresults"));
    QStringView thresholdString    = reader.attributes().value(QLatin1String("threshold"));
    QStringView maxThresholdString = reader.attributes().value(QLatin1String("maxthreshold"));
    QStringView sketchTypeString   = reader.attributes().value(QLatin1String("sketchtype"));
*/
    if      (type == QLatin1String("imageid"))
    {
        setCurrentImage(reader.valueToLongLong());
        d->imageSAlbum = salbum;
        d->tabWidget->setCurrentIndex((int)Private::SIMILARS);
    }
    else if (type == QLatin1String("signature"))
    {
        d->sketchSAlbum = salbum;
        d->tabWidget->setCurrentIndex((int)Private::SKETCH);

        if (reader.readToStartOfElement(QLatin1String("SketchImage")))
        {
            d->sketchWidget->setSketchImageFromXML(reader);
        }
    }
}

void FuzzySearchView::slotApplicationSettingsChanged()
{
    d->similarityRange->setRange(d->settings->getMinimumSimilarityBound(), 100);
}

} // namespace Digikam

#include "moc_fuzzysearchview.cpp"
