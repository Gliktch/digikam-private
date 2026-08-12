/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Find Duplicates View.
 *
 * SPDX-FileCopyrightText: 2016-2017 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "findduplicatesview.h"

// Qt includes

#include <QHeaderView>
#include <QLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include <QApplication>
#include <QStyle>
#include <QTimer>
#include <QTabWidget>
#include <QElapsedTimer>
#include <QGroupBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "albumselectors.h"
#include "similaritydb.h"
#include "similaritydbaccess.h"
#include "albumselectcombobox.h"
#include "albumtreeview.h"
#include "findduplicatesalbum.h"
#include "findduplicatesalbumitem.h"
#include "duplicatesfinder.h"
#include "fingerprintsgenerator.h"
#include "applicationsettings.h"
#include "haariface.h"
#include "squeezedcombobox.h"
#include "drangebox.h"
#include "dlayoutbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN FindDuplicatesView::Private
{
public:

    Private() = default;

public:

    QLabel*              includeAlbumsLabel      = nullptr;
    QLabel*              similarityLabel         = nullptr;
    QLabel*              restrictResultsLabel    = nullptr;
    QLabel*              albumTagRelationLabel   = nullptr;

    DIntRangeBox*        similarityRange         = nullptr;
    SqueezedComboBox*    searchResultRestriction = nullptr;
    SqueezedComboBox*    albumTagRelation        = nullptr;
    SqueezedComboBox*    refImageSelMethod       = nullptr;

    QPushButton*         findDuplicatesBtn       = nullptr;
    QPushButton*         updateFingerPrtBtn      = nullptr;
    QPushButton*         removeDuplicatesBtn     = nullptr;
    QPushButton*         helpButton              = nullptr;

    FindDuplicatesAlbum* listView                = nullptr;

    ProgressItem*        progressItem            = nullptr;

    AlbumSelectors*      albumSelectors          = nullptr;
    AlbumSelectors*      refImageAlbumSelector   = nullptr;

    ApplicationSettings* settings                = nullptr;
    QTabWidget*          tab                     = nullptr;

    bool                 active                  = false;
};

FindDuplicatesView::FindDuplicatesView(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    d->settings        = ApplicationSettings::instance();
    const int spacing  = layoutSpacing();

    // --- Duplicates List View

    d->listView        = new FindDuplicatesAlbum();
    d->tab             = new QTabWidget();

    // --- Albums Selection

    d->albumSelectors  = new AlbumSelectors(QString(), QLatin1String("Find Duplicates View"),
                                            nullptr, AlbumSelectors::AlbumType::All, true);
    d->tab->addTab(d->albumSelectors, i18nc("@title:tab", "Search in"));

    // --- Reference Selection

    auto* const refImageselection      = new QWidget();
    refImageselection->setToolTip(i18nc("@info", "Reference Image Selection Method"));
    auto* const layout                 = new QVBoxLayout();

    const auto refSelMethod            = ApplicationSettings::instance()->getDuplicatesRefImageSelMethod();
    d->refImageSelMethod               = new SqueezedComboBox();
    d->refImageSelMethod->addSqueezedItem(i18nc("@label:listbox similarity reference selection method",
                                                "Older or Larger"),
                                          (int)HaarIface::RefImageSelMethod::OlderOrLarger);
    d->refImageSelMethod->addSqueezedItem(i18nc("@label:listbox similarity reference selection method",
                                                "Prefer selected Album as reference"),
                                          (int)HaarIface::RefImageSelMethod::PreferFolder);
    d->refImageSelMethod->addSqueezedItem(i18nc("@label:listbox similarity reference selection method",
                                                "Prefer other than selected Album as reference"),
                                          (int)HaarIface::RefImageSelMethod::ExcludeFolder);
    d->refImageSelMethod->addSqueezedItem(i18nc("@label:listbox similarity reference selection method",
                                                "Prefer Newer Creation Date"),
                                          (int)HaarIface::RefImageSelMethod::NewerCreationDate);
    d->refImageSelMethod->addSqueezedItem(i18nc("@label:listbox similarity reference selection method",
                                                "Prefer Newer Modification Date"),
                                          (int)HaarIface::RefImageSelMethod::NewerModificationDate);
    d->refImageSelMethod->setCurrentIndex(d->refImageSelMethod->findData((int)refSelMethod));
    d->refImageSelMethod->setWhatsThis(i18nc("@label:listbox similarity reference selection method",
                                             "Select method for reference image selection"));

    d->refImageAlbumSelector           = new AlbumSelectors(i18nc("@label", "Reference Album:"),
                                                            QLatin1String("Select Reference"),
                                                            nullptr, AlbumSelectors::AlbumType::PhysAlbum, true);
    d->refImageAlbumSelector->setEnabled(
                                         (refSelMethod == HaarIface::RefImageSelMethod::ExcludeFolder) ||
                                         (refSelMethod == HaarIface::RefImageSelMethod::PreferFolder)
                                        );

    layout->addWidget(d->refImageSelMethod);
    layout->addWidget(d->refImageAlbumSelector);
    layout->addStretch();
    refImageselection->setLayout(layout);

    d->tab->addTab(refImageselection, i18nc("@title:tab", "Reference"));

    // --- Settings View

    auto* const settingsView = new QWidget();
    QGridLayout* const grid  = new QGridLayout();

    // ---

    d->albumTagRelationLabel = new QLabel(i18nc("@label", "Restrict Search to:"));
    d->albumTagRelationLabel->setBuddy(d->albumTagRelation);

    /*
     * only selected tab   => search duplicates in albums or tags, use the viewed tab and ignore the other tab.
     * one of              => search duplicates being in the selected albums or having the selected tags, use checkbox selection.
     * both                => search duplicates that are both in one of the selected albums and have at least one of the selected tags.
     * albums but not tags => search only duplicates in the selected albums that do not have the selected tags.
     * tags but not albums => vice versa.
     */

    d->albumTagRelation      = new SqueezedComboBox();
    d->albumTagRelation->addSqueezedItem(i18nc("@label:listbox similarity album tag relation",
                                               "Only selected tab"),
                                         (int)HaarIface::AlbumTagRelation::NoMix);
    d->albumTagRelation->addSqueezedItem(i18nc("@label:listbox similarity album tag relation",
                                               "One of"),
                                         (int)HaarIface::AlbumTagRelation::Union);
    d->albumTagRelation->addSqueezedItem(i18nc("@label:listbox similarity album tag relation",
                                               "Both"),
                                         (int)HaarIface::AlbumTagRelation::Intersection);
    d->albumTagRelation->addSqueezedItem(i18nc("@label:listbox similarity album tag relation",
                                               "Albums but not tags"),
                                         (int)HaarIface::AlbumTagRelation::AlbumExclusive);
    d->albumTagRelation->addSqueezedItem(i18nc("@label:listbox similarity album tag relation",
                                               "Tags but not albums"),
                                         (int)HaarIface::AlbumTagRelation::TagExclusive);
    d->albumTagRelation->setCurrentIndex(ApplicationSettings::instance()->getDuplicatesAlbumTagRelation());

    QString tip = i18nc("@info",
                        "Use this option to decide about the relation of the selected search-in albums and tags.\n"
                        "\"Only selected tab\": only the Search In selected tab is used.\n"
                        "\"One of\": the images are either in the selected albums or tags.\n"
                        "\"Both\": the images are both in the selected albums and tags.\n"
                        "\"Albums but not tags\": images must be in the selected albums but not tags.\n"
                        "\"Tags but not albums\": images must be in the selected tags but not albums.");
    d->albumTagRelation->setToolTip(tip);

    // Load the last choice from application settings.

    HaarIface::AlbumTagRelation relation
        = d->settings ? (HaarIface::AlbumTagRelation)(d->settings->getDuplicatesAlbumTagRelation())
                      : HaarIface::AlbumTagRelation::NoMix;

    d->albumTagRelation->setCurrentIndex(d->albumTagRelation->findData(relation));

    // ---

    d->similarityLabel       = new QLabel(i18nc("@label", "Similarity range:"));
    d->similarityLabel->setBuddy(d->similarityRange);

    d->similarityRange       = new DIntRangeBox();
    d->similarityRange->setSuffix(QLatin1String("%"));

    if (d->settings)
    {
        d->similarityRange->setRange(d->settings->getMinimumSimilarityBound(), 100);
        updateSimilarityRangeInterval();
    }
    else
    {
        d->similarityRange->setRange(40, 100);
        d->similarityRange->setInterval(40, 100);
    }

    // ---

    d->restrictResultsLabel    = new QLabel(i18nc("@label", "Limit Search to:"));
    d->restrictResultsLabel->setBuddy(d->searchResultRestriction);

    d->searchResultRestriction = new SqueezedComboBox();
    d->searchResultRestriction->addSqueezedItem(i18nc("@label:listbox similarity restriction", "None"),                    HaarIface::DuplicatesSearchRestrictions::None);
    d->searchResultRestriction->addSqueezedItem(i18nc("@label:listbox similarity restriction", "Reference Album only"),    HaarIface::DuplicatesSearchRestrictions::SameAlbum);
    d->searchResultRestriction->addSqueezedItem(i18nc("@label:listbox similarity restriction", "Exclude reference Album"), HaarIface::DuplicatesSearchRestrictions::DifferentAlbum);
    d->searchResultRestriction->setToolTip(i18nc("@info", "Use this option to restrict the duplicate search "
                                                          "with some criteria, as to limit search to the Album\n"
                                                          "of reference image, or to exclude the Album of "
                                                          "reference image of the search."));

    // Load the last choice from application settings.

    HaarIface::DuplicatesSearchRestrictions restrictions
        = d->settings ? (HaarIface::DuplicatesSearchRestrictions)(d->settings->getDuplicatesSearchRestrictions())
                      : HaarIface::DuplicatesSearchRestrictions::None;

    d->searchResultRestriction->setCurrentIndex(d->searchResultRestriction->findData(restrictions));

    // ---

    grid->addWidget(d->albumTagRelationLabel,   0, 0, 1,  2);
    grid->addWidget(d->albumTagRelation,        0, 2, 1, -1);
    grid->addWidget(d->similarityLabel,         1, 0, 1,  1);
    grid->addWidget(d->similarityRange,         1, 2, 1,  1);
    grid->addWidget(d->restrictResultsLabel,    2, 0, 1,  2);
    grid->addWidget(d->searchResultRestriction, 2, 2, 1, -1);
    grid->setRowStretch(3, 10);
    settingsView->setLayout(grid);

    d->tab->addTab(settingsView, i18nc("@title:tab", "Settings"));

    // --- Bottom View Buttons

    DHBox* const hbox     = new DHBox();
    d->updateFingerPrtBtn = new QPushButton(i18nc("@action", "Update all fingerprints"), hbox);
    d->updateFingerPrtBtn->setIcon(QIcon::fromTheme(QLatin1String("run-build")));
    d->updateFingerPrtBtn->setWhatsThis(i18nc("@info", "Use this button to update all image fingerprints."));

    d->helpButton         = new QPushButton(hbox);
    d->helpButton->setToolTip(i18nc("@info", "Help"));
    d->helpButton->setIcon(QIcon::fromTheme(QLatin1String("help-browser")));
    hbox->setStretchFactor(d->updateFingerPrtBtn, 10);

    QFontMetrics fmt      = d->updateFingerPrtBtn->fontMetrics();
    d->helpButton->setIconSize(QSize(fmt.height(), fmt.height()));

    connect(d->helpButton, &QPushButton::clicked,
            this, []()
        {
            openOnlineDocumentation(QLatin1String("left_sidebar"),
                                    QLatin1String("similarity_view"));
        }
    );

    d->findDuplicatesBtn  = new QPushButton(i18nc("@action", "Find duplicates"));
    d->findDuplicatesBtn->setIcon(QIcon::fromTheme(QLatin1String("edit-find")));
    d->findDuplicatesBtn->setWhatsThis(i18nc("@info", "Use this button to scan the selected albums for "
                                            "duplicate items."));

    d->removeDuplicatesBtn = new QPushButton(i18nc("@action", "Remove Duplicates"));
    d->removeDuplicatesBtn->setIcon(QIcon::fromTheme(QLatin1String("user-trash")));
    d->removeDuplicatesBtn->setWhatsThis(i18nc("@info", "Use this button to delete all duplicate images."));

    d->removeDuplicatesBtn->setEnabled(false);

    // ---

    QGridLayout* const mainLayout = new QGridLayout();
    mainLayout->addWidget(d->listView,                0, 0, 1, 1);
    mainLayout->addWidget(d->tab,                     1, 0, 1, 1);
    mainLayout->addWidget(hbox,                       2, 0, 1, 1);
    mainLayout->addWidget(d->findDuplicatesBtn,       3, 0, 1, 1);
    mainLayout->addWidget(d->removeDuplicatesBtn,     4, 0, 1, 1);

    mainLayout->setRowStretch(0, 10);
    mainLayout->setColumnStretch(0, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);
    setLayout(mainLayout);

    // ---

    connect(d->updateFingerPrtBtn, SIGNAL(clicked()),
            this, SLOT(slotUpdateFingerPrints()));

    connect(d->findDuplicatesBtn, SIGNAL(clicked()),
            this, SLOT(slotFindDuplicates()));

    connect(d->removeDuplicatesBtn, SIGNAL(clicked()),
            this, SLOT(slotRemoveDuplicates()));

    connect(d->listView, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotDuplicatesAlbumActived()));

    connect(d->albumSelectors, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotCheckForValidSettings()));

    connect(AlbumManager::instance(), SIGNAL(signalAllAlbumsLoaded()),
            this, SLOT(initAlbumUpdateConnections()));

    connect(d->settings, SIGNAL(setupChanged()),
            this, SLOT(slotApplicationSettingsChanged()));

    connect(d->refImageSelMethod, QOverload<int>::of(&SqueezedComboBox::currentIndexChanged),
            this, &FindDuplicatesView::slotReferenceSelectionMethodChanged);
}

FindDuplicatesView::~FindDuplicatesView()
{
    d->albumSelectors->saveState();
    d->refImageAlbumSelector->saveState();

    delete d;
}

void FindDuplicatesView::initAlbumUpdateConnections()
{

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalSearchUpdated(SAlbum*)),
            this, SLOT(slotSearchUpdated(SAlbum*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotClear()));

    connect(AlbumManager::instance(), SIGNAL(signalUpdateDuplicatesAlbums(QList<SAlbum*>,QList<qlonglong>)),
            this, SLOT(slotUpdateDuplicates(QList<SAlbum*>,QList<qlonglong>)));
}

void FindDuplicatesView::setActive(bool val)
{
    d->active = val;
    slotCheckForValidSettings();
    d->removeDuplicatesBtn->setEnabled(false);

    QTimer::singleShot(250, this, SLOT(slotSelectItemsTimer()));
}

void FindDuplicatesView::populateTreeView()
{
    if (d->listView->topLevelItemCount() > 0)
    {
        return;
    }

    d->listView->setSortingEnabled(false);
    QElapsedTimer waitCursorTimer;
    waitCursorTimer.start();

    bool waitCursor = false;

    const AlbumList& aList = AlbumManager::instance()->allSAlbums();

    for (AlbumList::const_iterator it = aList.constBegin() ; it != aList.constEnd() ; ++it)
    {
        SAlbum* const salbum = dynamic_cast<SAlbum*>(*it);

        if (salbum && salbum->isDuplicatesSearch() && !salbum->extraData(this))
        {
            // Adding item to listView by creating an item and passing listView as parent

            FindDuplicatesAlbumItem* const item = new FindDuplicatesAlbumItem(d->listView, salbum);
            salbum->setExtraData(this, item);
        }

        if (!waitCursor && (waitCursorTimer.elapsed() > 2000))
        {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            waitCursor = true;
        }
    }

    d->listView->setSortingEnabled(true);

    d->albumSelectors->loadState();
    d->refImageAlbumSelector->loadState();

    if (waitCursor)
    {
        QApplication::restoreOverrideCursor();
    }

    QTimer::singleShot(250, this, SLOT(slotSelectItemsTimer()));
}

QList<SAlbum*> FindDuplicatesView::currentFindDuplicatesAlbums() const
{
    QList<QTreeWidgetItem*> selectedItems = d->listView->selectedItems();

    if (selectedItems.isEmpty())
    {
        QTreeWidgetItem* const item = d->listView->firstItem();         // cppcheck-suppress constVariablePointer

        if (item)
        {
            selectedItems << item;
        }
    }

    QList<SAlbum*> albumList;

    for (QTreeWidgetItem* const item : std::as_const(selectedItems))
    {
        const FindDuplicatesAlbumItem* const albumItem = dynamic_cast<FindDuplicatesAlbumItem*>(item);

        if (albumItem)
        {
            albumList << albumItem->album();
        }
    }

    return albumList;
}

void FindDuplicatesView::slotAlbumAdded(Album* a)
{
    if (!a || (a->type() != Album::SEARCH))
    {
        return;
    }

    SAlbum* const salbum = static_cast<SAlbum*>(a);

    if (!salbum->isDuplicatesSearch())
    {
        return;
    }

    if (!d->active)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Duplicates view is not active, returning";
        return;
    }

    if (!salbum->extraData(this))
    {
        FindDuplicatesAlbumItem* const item = new FindDuplicatesAlbumItem(d->listView, salbum);
        salbum->setExtraData(this, item);
    }
}

void FindDuplicatesView::slotAlbumDeleted(Album* a)
{
    if (!a || (a->type() != Album::SEARCH))
    {
        return;
    }

    const SAlbum* const album = static_cast<SAlbum*>(a);

    FindDuplicatesAlbumItem* const item = static_cast<FindDuplicatesAlbumItem*>(album->extraData(this));

    if (item)
    {
        a->removeExtraData(this);
        delete item;
    }
}

void FindDuplicatesView::slotSearchUpdated(SAlbum* a)
{
    if (!a->isDuplicatesSearch())
    {
        return;
    }

    slotAlbumDeleted(a);
    slotAlbumAdded(a);
}

void FindDuplicatesView::slotSelectItemsTimer()
{
    d->listView->resizeColumnToContents(0);
    d->listView->sortByColumn(1, Qt::DescendingOrder);

    if (d->listView->selectedItems().isEmpty())
    {
        d->listView->selectFirstItem();
    }

    slotDuplicatesAlbumActived();
}

void FindDuplicatesView::slotClear()
{
/*
    for (QTreeWidgetItemIterator it(d->listView) ; *it ; ++it)
    {
        SAlbum* const salbum = static_cast<FindDuplicatesAlbumItem*>(*it)->album();

        if (salbum)
        {
            salbum->removeExtraData(this);
        }
    }

    d->listView->clear();
*/
    AlbumList::const_iterator it;
    const AlbumList& aList = AlbumManager::instance()->allSAlbums();

    for (it = aList.constBegin() ; it != aList.constEnd() ; ++it)
    {
        SAlbum* const salbum = dynamic_cast<SAlbum*>(*it);

        if (salbum && salbum->isDuplicatesSearch())
        {
            AlbumManager::instance()->deleteSAlbum(salbum);
        }
    }
}

void FindDuplicatesView::enableControlWidgets(bool val)
{
    d->searchResultRestriction->setEnabled(val);
    d->removeDuplicatesBtn->setEnabled(val);
    d->updateFingerPrtBtn->setEnabled(val);
    d->findDuplicatesBtn->setEnabled(val);
    d->refImageSelMethod->setEnabled(val);
    d->albumTagRelation->setEnabled(val);
    d->similarityRange->setEnabled(val);
    d->albumSelectors->setEnabled(val);

    d->albumTagRelationLabel->setEnabled(val);
    d->restrictResultsLabel->setEnabled(val);
    d->similarityLabel->setEnabled(val);
}

void FindDuplicatesView::slotFindDuplicates()
{
    d->refImageAlbumSelector->saveState();
    d->albumSelectors->saveState();
    enableControlWidgets(false);
    slotClear();

    AlbumList tags;
    AlbumList albums;
    AlbumList referenceImageSelector;

    if (d->albumTagRelation->itemData(d->albumTagRelation->currentIndex()).toInt() == HaarIface::AlbumTagRelation::NoMix)
    {
        if      (d->albumSelectors->typeSelection() == AlbumSelectors::AlbumType::PhysAlbum)
        {
            albums = d->albumSelectors->selectedAlbums();
        }
        else if (d->albumSelectors->typeSelection() == AlbumSelectors::AlbumType::TagsAlbum)
        {
            tags  = d->albumSelectors->selectedTags();
        }
    }
    else
    {
        albums = d->albumSelectors->selectedAlbums();
        tags   = d->albumSelectors->selectedTags();
    }

    const auto referenceImageSelectionMethod = (HaarIface::RefImageSelMethod)d->refImageSelMethod->
                                                itemData(d->refImageSelMethod->currentIndex()).toInt();

    if (referenceImageSelectionMethod != HaarIface::RefImageSelMethod::OlderOrLarger)
    {
        referenceImageSelector = d->refImageAlbumSelector->selectedAlbums();
    }

    AlbumManager::instance()->clearCurrentAlbums();
    SimilarityDbAccess().db()->clearImageSimilarity();

    DuplicatesFinder* const finder = new DuplicatesFinder(albums, tags,
                                                          d->albumTagRelation->itemData(d->albumTagRelation->currentIndex()).toInt(),
                                                          d->similarityRange->minValue(), d->similarityRange->maxValue(),
                                                          d->searchResultRestriction->itemData(d->searchResultRestriction->currentIndex()).toInt(),
                                                          referenceImageSelectionMethod, referenceImageSelector);

    connect(finder, SIGNAL(signalComplete()),
            this, SLOT(slotComplete()));

    connect(finder, &DuplicatesFinder::signalCanceled,
            this, [this]()
            {
                d->removeDuplicatesBtn->setEnabled(false);
            }
    );

    connect(finder, SIGNAL(signalScanNotification(QString,int)),
            this, SIGNAL(signalScanNotification(QString,int)));

    finder->start();
}

void FindDuplicatesView::slotUpdateDuplicates(const QList<SAlbum*>& sAlbumsToRebuild,
                                              const QList<qlonglong>& deletedImages)
{
    d->listView->updateDuplicatesAlbumItems(sAlbumsToRebuild, deletedImages);
}

void FindDuplicatesView::slotApplicationSettingsChanged()
{
    d->similarityRange->setRange(d->settings->getMinimumSimilarityBound(), 100);
}

void FindDuplicatesView::slotReferenceSelectionMethodChanged(int index)
{
   auto method = static_cast<HaarIface::RefImageSelMethod>(d->refImageSelMethod->itemData(index).toInt());

   d->refImageAlbumSelector->setEnabled(
                                        (method == HaarIface::RefImageSelMethod::ExcludeFolder) ||
                                        (method == HaarIface::RefImageSelMethod::PreferFolder)
                                       );
}

void FindDuplicatesView::slotComplete()
{
    enableControlWidgets(true);
    slotCheckForValidSettings();

    QTimer::singleShot(250, this, SLOT(slotSelectItemsTimer()));
}

void FindDuplicatesView::slotDuplicatesAlbumActived()
{
    if (!d->active)
    {
        return;
    }

    QList<Album*> albums;
    const auto items = d->listView->selectedItems();

    for (QTreeWidgetItem* const item : items)
    {
        const FindDuplicatesAlbumItem* const albumItem = dynamic_cast<FindDuplicatesAlbumItem*>(item);

        if (albumItem)
        {
            albums << albumItem->album();
        }
    }

    if (!albums.isEmpty())
    {
        AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << albums);
    }
}

void FindDuplicatesView::slotCheckForValidSettings()
{
    bool valid = (
                  d->albumSelectors->selectedAlbums().count() ||
                  d->albumSelectors->selectedTags().count()
                 );

    d->findDuplicatesBtn->setEnabled(valid);
}

void FindDuplicatesView::slotUpdateFingerPrints()
{
    FingerPrintsGenerator* const tool = new FingerPrintsGenerator(false);

    connect(tool, SIGNAL(signalScanNotification(QString,int)),
            this, SIGNAL(signalScanNotification(QString,int)));

    tool->start();
}

void FindDuplicatesView::slotSetSelectedAlbums(const QList<PAlbum*>& albums)
{
    // @ODD: Why is singleton set to true? resetAlbumsAndTags already clears the selection.

    resetAlbumsAndTags();

    const AlbumSelectors::SelectionType type = albums.size() == 1 ? AlbumSelectors::SingleSelection
                                                                  : AlbumSelectors::MultipleSelection;

    for (PAlbum* const album : std::as_const(albums))
    {
        d->albumSelectors->setAlbumSelected(album, type);
    }

    d->albumSelectors->setTypeSelection(AlbumSelectors::AlbumType::PhysAlbum);
    d->albumTagRelation->setCurrentIndex(d->albumTagRelation->findData(HaarIface::AlbumTagRelation::NoMix));
    d->searchResultRestriction->setCurrentIndex(d->searchResultRestriction->findData(HaarIface::DuplicatesSearchRestrictions::None));
    slotCheckForValidSettings();

    if (d->findDuplicatesBtn->isEnabled())
    {
        slotFindDuplicates();
    }
}

void FindDuplicatesView::slotSetSelectedAlbums(const QList<TAlbum*>& albums)
{
    resetAlbumsAndTags();

    const AlbumSelectors::SelectionType type = albums.size() == 1 ? AlbumSelectors::SingleSelection
                                                                  : AlbumSelectors::MultipleSelection;

    for (TAlbum* const album : std::as_const(albums))
    {
        d->albumSelectors->setTagSelected(album, type);
    }

    d->albumSelectors->setTypeSelection(AlbumSelectors::AlbumType::TagsAlbum);
    d->albumTagRelation->setCurrentIndex(d->albumTagRelation->findData(HaarIface::AlbumTagRelation::NoMix));
    d->searchResultRestriction->setCurrentIndex(d->searchResultRestriction->findData(HaarIface::DuplicatesSearchRestrictions::None));
    slotCheckForValidSettings();

    if (d->findDuplicatesBtn->isEnabled())
    {
        slotFindDuplicates();
    }
}

void FindDuplicatesView::updateSimilarityRangeInterval()
{
    d->similarityRange->setInterval(d->settings->getDuplicatesSearchLastMinSimilarity(),
                                    d->settings->getDuplicatesSearchLastMaxSimilarity());
}

void FindDuplicatesView::resetAlbumsAndTags()
{
    d->albumSelectors->resetSelection();
    slotCheckForValidSettings();
}

void FindDuplicatesView::slotRemoveDuplicates()
{
    d->listView->removeDuplicates();
}

} // namespace Digikam

#include "moc_findduplicatesview.cpp"
