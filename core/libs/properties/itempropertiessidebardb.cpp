/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : item properties side bar using data from digiKam database.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Martin Klapetek <martin dot klapetek at gmail dot com>
 * SPDX-FileCopyrightText:      2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itempropertiessidebardb.h"

// Qt includes

#include <QDir>
#include <QHash>
#include <QColor>
#include <QTimer>
#include <QLocale>
#include <QSplitter>
#include <QFileInfo>
#include <QScopedPointer>
#include <QtConcurrentRun>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "applicationsettings.h"
#include "coredbinfocontainers.h"
#include "coredbwatch.h"
#include "dimg.h"
#include "itemattributeswatch.h"
#include "itemdescedittab.h"
#include "iteminfo.h"
#include "itempropertiestab.h"
#include "itemselectionpropertiestab.h"
#include "itempropertiesmetadatatab.h"
#include "itempropertiescolorstab.h"
#include "itempropertiesversionstab.h"
#include "itemposition.h"
#include "itemfiltermodel.h"
#include "tagscache.h"
#include "tagsactionmngr.h"
#include "dtrash.h"
#include "itemsgroupedview.h"

#ifdef HAVE_GEOLOCATION
#   include "itempropertiesgpstab.h"
#   include "gpsiteminfosorter.h"
#endif // HAVE_GEOLOCATION

namespace Digikam
{

class Q_DECL_HIDDEN ItemPropertiesSideBarDB::Private
{
public:

    Private() = default;

public:

    bool                        dirtyDesceditTab       = false;
    bool                        hasPrevious            = false;
    bool                        hasNext                = false;
    bool                        hasItemInfoOwnership   = false;
    bool                        showAllPropertiesMode  = false;

    QFuture<void>               selectedItemsTask;
    QTimer*                     selectedItemsTimer;

    ItemInfoList                currentInfos;                    ///< Used if multiple items are selected.
    ItemInfoList                allInfos;
    DImageHistory               currentHistory;
    ItemSelectionPropertiesTab* selectionPropertiesTab = nullptr;
    ItemDescEditTab*            desceditTab            = nullptr;
    ItemPropertiesVersionsTab*  versionsHistoryTab     = nullptr;
    ItemsGroupedView*           treeGroup              = nullptr;
};

ItemPropertiesSideBarDB::ItemPropertiesSideBarDB(QWidget* const parent, SidebarSplitter* const splitter,
                                                 Qt::Edge side, bool mimimizedDefault)
    : ItemPropertiesSideBar(parent, splitter, side, mimimizedDefault),
      d                    (new Private)
{
    d->selectionPropertiesTab = new ItemSelectionPropertiesTab(this);
    m_propertiesStackedView->addWidget(d->selectionPropertiesTab);

    d->desceditTab            = new ItemDescEditTab(parent);
    d->versionsHistoryTab     = new ItemPropertiesVersionsTab(parent);

    d->selectedItemsTimer     = new QTimer(this);
    d->selectedItemsTimer->setSingleShot(true);
    d->selectedItemsTimer->setInterval(500);

    appendTab(d->desceditTab,        QIcon::fromTheme(QLatin1String("edit-text-frame-update")), i18nc("@title: database properties", "Captions"));
    appendTab(d->versionsHistoryTab, QIcon::fromTheme(QLatin1String("view-catalog")),           i18nc("@title: database properties", "Versions"));

    m_propertiesTab->widget(ItemPropertiesTab::digiKamProperties)->setButtonIcon(QIcon::fromTheme(QLatin1String("document-edit")));
    m_propertiesTab->widget(ItemPropertiesTab::digiKamProperties)->setButtonVisible(true);
    m_propertiesTab->widget(ItemPropertiesTab::digiKamProperties)->setButtonToolTip(i18n("Open the Description tab for editing"));

    m_propertiesTab->widget(ItemPropertiesTab::TagsProperties)->setButtonIcon(QIcon::fromTheme(QLatin1String("document-edit")));
    m_propertiesTab->widget(ItemPropertiesTab::TagsProperties)->setButtonVisible(true);
    m_propertiesTab->widget(ItemPropertiesTab::TagsProperties)->setButtonToolTip(i18n("Open the Tags tab for editing"));

    m_propertiesTab->widget(ItemPropertiesTab::LocationProperties)->setButtonIcon(QIcon::fromTheme(QLatin1String("document-edit")));
    m_propertiesTab->widget(ItemPropertiesTab::LocationProperties)->setButtonVisible(true);
    m_propertiesTab->widget(ItemPropertiesTab::LocationProperties)->setButtonToolTip(i18n("Open the Information tab for editing"));

    m_propertiesTab->widget(ItemPropertiesTab::RightProperties)->setButtonIcon(QIcon::fromTheme(QLatin1String("document-edit")));
    m_propertiesTab->widget(ItemPropertiesTab::RightProperties)->setButtonVisible(true);
    m_propertiesTab->widget(ItemPropertiesTab::RightProperties)->setButtonToolTip(i18n("Open the Information tab for editing"));

    d->treeGroup = new ItemsGroupedView(this);
    m_propertiesTab->setGroupedWidget(d->treeGroup);

    // ----------------------------------------------------------

    connect(m_propertiesTab, SIGNAL(signalItemButtonPressed(int)),
            this, SLOT(slotPropertiesButtonPressed(int)));

    connect(this, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotChangedTab(QWidget*)));

    connect(d->desceditTab, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->desceditTab, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(d->desceditTab, SIGNAL(signalRightSideBarBusy(bool)),
            this, SIGNAL(signalRightSideBarBusy(bool)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotImageChangeDatabase(ImageChangeset)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(imageTagChange(ImageTagChangeset)),
            this, SLOT(slotImageTagChanged(ImageTagChangeset)));

    connect(ItemAttributesWatch::instance(), SIGNAL(signalFileMetadataChanged(QUrl)),
            this, SLOT(slotFileMetadataChanged(QUrl)));

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotLoadMetadataFilters()));

    connect(TagsActionMngr::defaultManager(), &TagsActionMngr::signalColorLabelNamesUpdated,
            this, &ItemPropertiesSideBarDB::slotColorLabelNamesUpdated);

    // ----------------------------------------------------------

    connect(d->selectedItemsTimer, &QTimer::timeout,
            this, &ItemPropertiesSideBarDB::slotImageSelectionPropertiesInformation);

    connect(this, &ItemPropertiesSideBarDB::signalSetSelectionCount,
            d->selectionPropertiesTab, &ItemSelectionPropertiesTab::slotSetSelectionCount,
            Qt::QueuedConnection);

    connect(this, &ItemPropertiesSideBarDB::signalSetSelectionSize,
            d->selectionPropertiesTab, &ItemSelectionPropertiesTab::slotSetSelectionSize,
            Qt::QueuedConnection);

    connect(this, &ItemPropertiesSideBarDB::signalSetSelectionMimes,
            d->selectionPropertiesTab, &ItemSelectionPropertiesTab::slotSetSelectionMimes,
            Qt::QueuedConnection);

    connect(this, &ItemPropertiesSideBarDB::signalSetSelectionGroups,
            d->selectionPropertiesTab, &ItemSelectionPropertiesTab::slotSetSelectionGroups,
            Qt::QueuedConnection);

    connect(this, &ItemPropertiesSideBarDB::signalSetTotalCount,
            d->selectionPropertiesTab, &ItemSelectionPropertiesTab::slotSetTotalCount,
            Qt::QueuedConnection);

    connect(this, &ItemPropertiesSideBarDB::signalSetTotalSize,
            d->selectionPropertiesTab, &ItemSelectionPropertiesTab::slotSetTotalSize,
            Qt::QueuedConnection);

    connect(this, &ItemPropertiesSideBarDB::signalSetTotalMimes,
            d->selectionPropertiesTab, &ItemSelectionPropertiesTab::slotSetTotalMimes,
            Qt::QueuedConnection);

    connect(this, &ItemPropertiesSideBarDB::signalSetTotalGroups,
            d->selectionPropertiesTab, &ItemSelectionPropertiesTab::slotSetTotalGroups,
            Qt::QueuedConnection);
}

ItemPropertiesSideBarDB::~ItemPropertiesSideBarDB()
{
    d->selectedItemsTimer->stop();

    if (d->selectedItemsTask.isRunning())
    {
        d->selectedItemsTask.cancel();
        d->selectedItemsTask.waitForFinished();
    }

    delete d;
}

ItemInfoList ItemPropertiesSideBarDB::currentSelection() const
{
    return d->currentInfos;
}

void ItemPropertiesSideBarDB::setItemFilterModel(ItemFilterModel* const model)
{
    d->treeGroup->setItemFilterModel(model);
    d->selectionPropertiesTab->setItemFilterModel(model);
}

void ItemPropertiesSideBarDB::setShowAllPropertiesMode(bool b)
{
    d->showAllPropertiesMode = b;
}

void ItemPropertiesSideBarDB::itemChanged(const ItemInfo& info, const QRect& rect,
                                          DImg* const img, const DImageHistory& history)
{
    itemChanged(info.fileUrl(), info, rect, img, history);
}

void ItemPropertiesSideBarDB::itemChanged(const QUrl& url, const QRect& rect, DImg* const img)
{
    itemChanged(url, ItemInfo(), rect, img, DImageHistory());
}

void ItemPropertiesSideBarDB::itemChanged(const QUrl& url,
                                          const ItemInfo& info,
                                          const QRect& rect,
                                          DImg* const img,
                                          const DImageHistory& history)
{
    if (!url.isValid())
    {
        return;
    }

    m_currentUrl = url;

    // Special case if item come from a trash view. ItemInfo will be empty as item is removed from the database.
    // We need to use the DMetadata based method to populated the properties view.

    if (url.path().contains(DTrash::TRASH_FOLDER))
    {
        ItemPropertiesSideBar::itemChanged(url);

        return;
    }

    ItemInfoList list;

    if (!info.isNull())
    {
        list << info;
    }

    ItemInfoList allInfos;

    itemChanged(list, allInfos, rect, img, history);
}

void ItemPropertiesSideBarDB::itemChanged(const ItemInfoList& infos, const ItemInfoList& allInfos)
{
    if (infos.isEmpty())
    {
        return;
    }

    m_currentUrl = infos.first().fileUrl();

    itemChanged(infos, allInfos, QRect(), nullptr, DImageHistory());
}

void ItemPropertiesSideBarDB::itemChanged(const ItemInfoList& infos,
                                          const ItemInfoList& allInfos,
                                          const QRect& rect,
                                          DImg* const img,
                                          const DImageHistory& history)
{
    d->selectedItemsTimer->stop();

    if (d->selectedItemsTask.isRunning())
    {
        d->selectedItemsTask.cancel();
        d->selectedItemsTask.waitForFinished();
    }

    bool changedInfos    = (d->currentInfos == infos);
    m_dirtyPropertiesTab = (changedInfos && (d->allInfos == allInfos));
    m_dirtyMetadataTab   = changedInfos;
    m_dirtyColorTab      = (changedInfos && (m_image == img) && (m_currentRect == rect));
    m_dirtyGpsTab        = changedInfos;
    m_dirtyHistoryTab    = (changedInfos && (d->currentHistory == history));
    d->dirtyDesceditTab  = changedInfos;

    m_currentRect        = rect;
    m_image              = img;
    d->currentHistory    = history;
    d->currentInfos      = infos;
    d->allInfos          = allInfos;

    // changedTab only handles the active tab.
    // Any tab that holds information reset above shall be reset here,
    // unless it is the active tab.

    if (getActiveTab() != d->desceditTab)
    {
        d->desceditTab->setItem();
    }

    changedTab(getActiveTab());
}

void ItemPropertiesSideBarDB::slotNoCurrentItem()
{
    ItemPropertiesSideBar::slotNoCurrentItem();
    d->treeGroup->setVisible(false);
    m_dirtyPropertiesTab = false;

    // All tabs that store the ItemInfo list and access it after selection change
    // must release the image info here. changedTab only handles the active tab!

    d->desceditTab->setItem();
    d->currentInfos.clear();
    d->dirtyDesceditTab  = false;

    changedTab(getActiveTab());
}

void ItemPropertiesSideBarDB::populateTags()
{
    d->desceditTab->populateTags();
}

void ItemPropertiesSideBarDB::slotChangedTab(QWidget* tab)
{
    m_dirtyPropertiesTab = false;
    m_dirtyMetadataTab   = false;
    m_dirtyColorTab      = false;
    m_dirtyGpsTab        = false;
    m_dirtyHistoryTab    = false;
    d->dirtyDesceditTab  = false;

    changedTab(tab);
}

void ItemPropertiesSideBarDB::changedTab(QWidget* const tab)
{
    setCursor(Qt::WaitCursor);

    if      ((tab == m_propertiesStackedView) && !m_dirtyPropertiesTab)
    {
        m_propertiesTab->setCurrentUrl(m_currentUrl);

        if      (d->currentInfos.isEmpty())
        {
            if (!d->showAllPropertiesMode)
            {
                ItemPropertiesSideBar::setImagePropertiesInformation(m_currentUrl);
            }
            else
            {
                d->selectedItemsTimer->start();
                m_propertiesStackedView->setCurrentWidget(d->selectionPropertiesTab);
            }
        }
        else if (d->currentInfos.count() == 1)
        {
            setImagePropertiesInformation(m_currentUrl);
            m_propertiesStackedView->setCurrentWidget(m_propertiesTab);
        }
        else
        {
            d->selectedItemsTimer->start();
            m_propertiesStackedView->setCurrentWidget(d->selectionPropertiesTab);
        }

        m_dirtyPropertiesTab = true;
    }
    else if ((tab == m_metadataTab) && !m_dirtyMetadataTab)
    {
        if (d->currentInfos.count() > 1)
        {
            // No multiple selection supported. Only if all items belong to
            // the same group display metadata of main item.

            ItemInfo mainItem = d->currentInfos.singleGroupMainItem();

            if (!mainItem.isNull())
            {
                m_metadataTab->setCurrentUrl(mainItem.fileUrl());
            }
            else
            {
                m_metadataTab->setCurrentUrl();
            }
        }
        else if (m_image)
        {
            QScopedPointer<DMetadata> data(new DMetadata(m_image->getMetadata()));
            m_metadataTab->setCurrentData(data.data(), m_currentUrl);
        }
        else
        {
            m_metadataTab->setCurrentUrl(m_currentUrl);
        }

        m_dirtyMetadataTab = true;
    }
    else if ((tab == m_colorTab) && !m_dirtyColorTab)
    {
        if (d->currentInfos.count() > 1)
        {
            // No multiple selection supported. Only if all items belong to
            // the same group display metadata of main item.

            ItemInfo mainItem = d->currentInfos.singleGroupMainItem();

            if (!mainItem.isNull())
            {
                m_colorTab->setData(mainItem.fileUrl());
            }
            else
            {
                m_colorTab->setData();
            }
        }
        else
        {
            m_colorTab->setData(m_currentUrl, m_currentRect, m_image);
        }

        m_dirtyColorTab = true;
    }
    else if ((tab == d->desceditTab) && !d->dirtyDesceditTab)
    {
        if      (d->currentInfos.count() == 0)
        {
            // Do nothing here. We cannot get data from database !

            d->desceditTab->setItem();
        }
        else if (d->currentInfos.count() == 1)
        {
            d->desceditTab->setItem(d->currentInfos.first());
        }
        else
        {
            d->desceditTab->setItems(d->currentInfos);
        }

        d->dirtyDesceditTab = true;
    }

#ifdef HAVE_GEOLOCATION

    else if ((tab == m_gpsTab) && !m_dirtyGpsTab)
    {
        if (d->currentInfos.count() == 0)
        {
            m_gpsTab->setCurrentUrl(m_currentUrl);
        }
        else
        {
            GPSItemInfo::List list;

            for (ItemInfoList::const_iterator it = d->currentInfos.constBegin() ;
                 it != d->currentInfos.constEnd() ; ++it)
            {
                GPSItemInfo info;

                if (GPSItemInfofromItemInfo(*it, &info))
                {
                    list << info;
                }
            }

            if (list.isEmpty())
            {
                m_gpsTab->setCurrentUrl();
            }
            else
            {
                m_gpsTab->setGPSInfoList(list);
            }
        }

        m_dirtyGpsTab = true;
    }

#endif // HAVE_GEOLOCATION

    else if ((tab == d->versionsHistoryTab) && !m_dirtyHistoryTab)
    {
        // TODO: Make a database-less parent class with only the filters tab

        if ((d->currentInfos.count() == 0) || (d->currentInfos.count() > 1))
        {
            // FIXME: Any sensible multi-selection functionality? Must scale for large n!

            d->versionsHistoryTab->clear();
        }
        else
        {
            d->versionsHistoryTab->setItem(d->currentInfos.first(), d->currentHistory);
        }

        m_dirtyHistoryTab = true;
    }

#ifdef HAVE_GEOLOCATION

    m_gpsTab->setActive(tab == m_gpsTab);

#endif // HAVE_GEOLOCATION

    unsetCursor();
}

void ItemPropertiesSideBarDB::slotFileMetadataChanged(const QUrl& url)
{
    if (url == m_currentUrl)
    {
        // trigger an update

        m_dirtyMetadataTab = false;

        if (getActiveTab() == m_metadataTab)
        {
            // update now - reuse code form changedTab

            changedTab(getActiveTab());
        }
    }
}

void ItemPropertiesSideBarDB::slotImageChangeDatabase(const ImageChangeset& changeset)
{
    if (!d->currentInfos.isEmpty())
    {
        QWidget* const tab = getActiveTab();

        if (!tab)
        {
            return;
        }

        if      ((tab == m_propertiesStackedView)

#ifdef HAVE_GEOLOCATION

            || (tab == m_gpsTab)

#endif // HAVE_GEOLOCATION

           )
        {
            const ItemInfo& info = d->currentInfos.first();

            if (changeset.ids().contains(info.id()))
            {
                // trigger an update, if changes touch the tab's information

                DatabaseFields::Set set = changeset.changes();

                if      ((set & DatabaseFields::ImagesAll)           ||
                         (set & DatabaseFields::ItemInformationAll)  ||
                         (set & DatabaseFields::ImageMetadataAll)    ||
                         (set & DatabaseFields::VideoMetadataAll)    ||
                         (set & DatabaseFields::ImageHistoryInfoAll) ||
                         (set & DatabaseFields::ItemCommentsAll))
                {
                    m_dirtyPropertiesTab = false;
                }
                else if (set & DatabaseFields::ItemPositionsAll)
                {
                    m_dirtyGpsTab = false;
                }

                if ((tab == m_propertiesStackedView)

#ifdef HAVE_GEOLOCATION

                    || (tab == m_gpsTab)

#endif // HAVE_GEOLOCATION

                   )
                {
                    // update now - reuse code form changedTab

                    changedTab(tab);
                }
            }
        }
        else if ((tab == d->desceditTab) && d->desceditTab->isModified())
        {
            const ItemInfo& info = d->currentInfos.first();

            if (changeset.ids().contains(info.id()))
            {
                DatabaseFields::Set set = changeset.changes();

                if (set & DatabaseFields::Rating)
                {
                    d->desceditTab->replaceRating(info.rating());
                }
            }
        }
        else if (tab == m_metadataTab)
        {
            const ItemInfo& info = d->currentInfos.first();

            if (changeset.ids().contains(info.id()))
            {
                // trigger an update, if changes touch creation date

                DatabaseFields::Set set = changeset.changes();

                if (set & DatabaseFields::CreationDate)
                {
                    // update now - reuse code form changedTab

                    m_dirtyMetadataTab = false;

                    changedTab(tab);
                }
            }
        }
    }
}

void ItemPropertiesSideBarDB::slotImageTagChanged(const ImageTagChangeset& changeset)
{
    if (!d->currentInfos.isEmpty())
    {
        QWidget* const tab = getActiveTab();

        if (!tab)
        {
            return;
        }

        if      (tab == m_propertiesStackedView)
        {
            const ItemInfo& info = d->currentInfos.first();

            if (changeset.ids().contains(info.id()))
            {
                m_dirtyPropertiesTab = false;
                changedTab(tab);
            }
        }
        else if ((tab == d->desceditTab) && d->desceditTab->isModified())
        {
            const ItemInfo& info = d->currentInfos.first();

            if (changeset.ids().contains(info.id()))
            {
                QVector<int> colorLabelTags = TagsCache::instance()->colorLabelTags();
                QVector<int> pickLabelTags  = TagsCache::instance()->pickLabelTags();
                const auto ctags            = changeset.tags();

                for (int ctag : ctags)
                {
                    if (colorLabelTags.contains(ctag))
                    {
                        d->desceditTab->replaceColorLabel(info.colorLabel());
                        break;
                    }
                }

                const auto ptags            = changeset.tags();

                for (int ptag : ptags)
                {
                    if (pickLabelTags.contains(ptag))
                    {
                        d->desceditTab->replacePickLabel(info.pickLabel());
                        break;
                    }
                }
            }
        }
    }
}

void ItemPropertiesSideBarDB::slotAssignRating(int rating)
{
    d->desceditTab->assignRating(rating);
}

void ItemPropertiesSideBarDB::slotAssignRatingNoStar()
{
    d->desceditTab->assignRating(0);
}

void ItemPropertiesSideBarDB::slotAssignRatingOneStar()
{
    d->desceditTab->assignRating(1);
}

void ItemPropertiesSideBarDB::slotAssignRatingTwoStar()
{
    d->desceditTab->assignRating(2);
}

void ItemPropertiesSideBarDB::slotAssignRatingThreeStar()
{
    d->desceditTab->assignRating(3);
}

void ItemPropertiesSideBarDB::slotAssignRatingFourStar()
{
    d->desceditTab->assignRating(4);
}

void ItemPropertiesSideBarDB::slotAssignRatingFiveStar()
{
    d->desceditTab->assignRating(5);
}

void ItemPropertiesSideBarDB::refreshTagsView()
{
    // TODO update, do we still need this method?
/*
    d->desceditTab->refreshTagsView();
*/
}

void ItemPropertiesSideBarDB::slotColorLabelNamesUpdated()
{
    setImagePropertiesInformation(m_currentUrl);
}

void ItemPropertiesSideBarDB::setImagePropertiesInformation(const QUrl& url)
{
    for (const ItemInfo& info : std::as_const(d->currentInfos))
    {
        if (info.fileUrl() == url)
        {   // cppcheck-suppress useStlAlgorithm
            QString str;
            QString unavailable(QString::fromUtf8("<i>%1</i>").arg(i18nc("@info: item properties", "unavailable")));
            QFileInfo fileInfo(url.toLocalFile());

            // -- File system information -----------------------------------------

            ImageCommonContainer commonInfo  = info.imageCommonContainer();
            ImageMetadataContainer photoInfo = info.imageMetadataContainer();
            VideoMetadataContainer videoInfo = info.videoMetadataContainer();

            str = asUserDateTime(commonInfo.fileModificationDate);
            m_propertiesTab->setFileModifiedDate(str);

            str = QString::fromUtf8("%1 (%2)").arg(ItemPropertiesTab::humanReadableBytesCount(commonInfo.fileSize))
                                    .arg(QLocale().toString(commonInfo.fileSize));
            m_propertiesTab->setFileSize(str);

            // If the collection is offline, url is empty.

            if (url.isEmpty())
            {
                m_propertiesTab->setFileName(info.name());
                QString path(QLatin1String("???") + info.relativePath());
                m_propertiesTab->setFileFolder(QDir::toNativeSeparators(path));
            }

            //  These infos are not stored in DB

            m_propertiesTab->setFileOwner(QString::fromUtf8("%1 - %2").arg(fileInfo.owner()).arg(fileInfo.group()));
            m_propertiesTab->setFilePermissions(ItemPropertiesTab::permissionsString(fileInfo));

            // -- Image Properties --------------------------------------------------

            if ((commonInfo.width == 0) || (commonInfo.height == 0))
            {
                str = i18nc("@info: item properties", "Unknown");
            }
            else
            {
                QString mpixels = QLocale().toString(commonInfo.width * commonInfo.height / 1000000.0, 'f', 1);
                str = i18nc("@info: width x height (megapixels Mpx)", "%1x%2 (%3Mpx)",
                            commonInfo.width, commonInfo.height, mpixels);
            }

            m_propertiesTab->setItemDimensions(str);

            if ((commonInfo.width == 0) || (commonInfo.height == 0))
            {
                str = i18nc("@info: item properties", "Unknown");
            }
            else
            {
                m_propertiesTab->aspectRatioToString(commonInfo.width, commonInfo.height, str);
            }

            m_propertiesTab->setImageRatio(str);
            m_propertiesTab->setImageMime(commonInfo.format);
            m_propertiesTab->setImageBitDepth(i18nc("@info: item properties", "%1 bpp", commonInfo.colorDepth));
            m_propertiesTab->setHasSidecar(DMetadata::hasSidecar(url.toLocalFile()) ? i18nc("@info: item properties", "Yes")
                                                                                    : i18nc("@info: item properties", "No"));
            m_propertiesTab->setHasGPSInfo(info.hasCoordinates() ? i18nc("@info: item properties", "Yes")
                                                                 : i18nc("@info: item properties", "No"));
            m_propertiesTab->setImageColorMode(commonInfo.colorModel.isEmpty() ? unavailable : commonInfo.colorModel);

            // -- Photograph information ------------------------------------------

            m_propertiesTab->setPhotoInfoDisable(photoInfo.allFieldsNull);
            ItemPropertiesTab::shortenedMakeInfo(photoInfo.make);
            ItemPropertiesTab::shortenedModelInfo(photoInfo.model);
            m_propertiesTab->setPhotoMake(photoInfo.make.isEmpty()   ? unavailable : photoInfo.make);
            m_propertiesTab->setPhotoModel(photoInfo.model.isEmpty() ? unavailable : photoInfo.model);

            if (commonInfo.creationDate.isValid())
            {
                str = asUserDateTime(commonInfo.creationDate);
                m_propertiesTab->setPhotoDateTime(str);
            }
            else
            {
                m_propertiesTab->setPhotoDateTime(unavailable);
            }

            m_propertiesTab->setPhotoLens(photoInfo.lens.isEmpty()         ? unavailable : photoInfo.lens);
            m_propertiesTab->setPhotoAperture(photoInfo.aperture.isEmpty() ? unavailable : photoInfo.aperture);

            if (photoInfo.focalLength35.isEmpty())
            {
                m_propertiesTab->setPhotoFocalLength(photoInfo.focalLength.isEmpty() ? unavailable : photoInfo.focalLength);
            }
            else
            {
                str = i18nc("@info: item properties", "%1 (%2)", photoInfo.focalLength, photoInfo.focalLength35);
                m_propertiesTab->setPhotoFocalLength(str);
            }

            m_propertiesTab->setPhotoExposureTime(photoInfo.exposureTime.isEmpty() ? unavailable : photoInfo.exposureTime);
            m_propertiesTab->setPhotoSensitivity(photoInfo.sensitivity.isEmpty()   ? unavailable : i18nc("@info: item properties", "%1 ISO", photoInfo.sensitivity));

            if      (photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
            {
                m_propertiesTab->setPhotoExposureMode(unavailable);
            }
            else if (!photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
            {
                m_propertiesTab->setPhotoExposureMode(photoInfo.exposureMode);
            }
            else if (photoInfo.exposureMode.isEmpty() && !photoInfo.exposureProgram.isEmpty())
            {
                m_propertiesTab->setPhotoExposureMode(photoInfo.exposureProgram);
            }
            else
            {
                str = QString::fromUtf8("%1 / %2").arg(photoInfo.exposureMode).arg(photoInfo.exposureProgram);
                m_propertiesTab->setPhotoExposureMode(str);
            }

            m_propertiesTab->setPhotoFlash(photoInfo.flashMode.isEmpty() ? unavailable : photoInfo.flashMode);
            m_propertiesTab->setPhotoWhiteBalance(photoInfo.whiteBalance.isEmpty() ? unavailable : photoInfo.whiteBalance);

            // -- Audio/Video Metadata information ------------------------

            m_propertiesTab->setVideoInfoDisable(videoInfo.allFieldsNull);

            m_propertiesTab->setVideoAspectRatio(videoInfo.aspectRatio.isEmpty()           ? unavailable : videoInfo.aspectRatio);
            m_propertiesTab->setVideoDuration(videoInfo.duration.isEmpty()                 ? unavailable : videoInfo.duration);
            m_propertiesTab->setVideoFrameRate(videoInfo.frameRate.isEmpty()               ? unavailable : videoInfo.frameRate);
            m_propertiesTab->setVideoVideoCodec(videoInfo.videoCodec.isEmpty()             ? unavailable : videoInfo.videoCodec);
            m_propertiesTab->setVideoAudioBitRate(videoInfo.audioBitRate.isEmpty()         ? unavailable : videoInfo.audioBitRate);
            m_propertiesTab->setVideoAudioChannelType(videoInfo.audioChannelType.isEmpty() ? unavailable : videoInfo.audioChannelType);
            m_propertiesTab->setVideoAudioCodec(videoInfo.audioCodec.isEmpty()             ? unavailable : videoInfo.audioCodec);

            // -- Caption / Tags ------------------------------------------

            m_propertiesTab->setTitle(info.title());
            m_propertiesTab->setCaption(info.comment());
            m_propertiesTab->setPickLabel(info.pickLabel());
            m_propertiesTab->setColorLabel(info.colorLabel());
            m_propertiesTab->setRating(info.rating());
            QList<int> allTagIds = info.tagIds();
            QList<int> regularTagIds;
            QList<int> peopleTagIds;

            for (int t : std::as_const(allTagIds))
            {
                if (TagsCache::instance()->properties(t).contains(QLatin1String("person")))
                {
                    peopleTagIds << t;
                }
                else
                {
                    regularTagIds << t;
                }
            }

            m_propertiesTab->setTags(
                                     TagsCache::instance()->tagPaths(regularTagIds, TagsCache::NoLeadingSlash, TagsCache::NoHiddenTags),
                                     TagsCache::instance()->tagNames(regularTagIds, TagsCache::NoHiddenTags),
                                     TagsCache::instance()->tagPaths(peopleTagIds,  TagsCache::NoLeadingSlash, TagsCache::NoHiddenTags),
                                     TagsCache::instance()->tagNames(peopleTagIds,  TagsCache::NoHiddenTags)
                                    );

            m_propertiesTab->setTemplate(info.metadataTemplate());

            m_propertiesTab->setVersionedInfo((info.hasDerivedImages() || info.hasAncestorImages()) ? i18nc("@info: item properties", "Yes")
                                                                                                    : i18nc("@info: item properties", "No"));

            d->treeGroup->setVisible(false);

            if      (info.isGrouped())
            {
                m_propertiesTab->setGroupedInfo(i18nc("@info: item properties", "Yes"));
            }
            else if (info.hasGroupedImages())
            {
                m_propertiesTab->setGroupedInfo(i18nc("@info: item properties", "Yes (stack)"));
                ItemInfoList list;
                list.append(info);
                d->treeGroup->setGroups(list);
                d->treeGroup->setVisible(true);
            }
            else
            {
                m_propertiesTab->setGroupedInfo(i18nc("@info: item properties", "No"));
            }

            m_propertiesTab->showOrHideCachedProperties();

            return;
        }
    }
}

void ItemPropertiesSideBarDB::slotImageSelectionPropertiesInformation()
{
    if (d->selectedItemsTask.isRunning())
    {
        d->selectedItemsTask.cancel();
        d->selectedItemsTask.waitForFinished();
    }

    d->selectionPropertiesTab->clear();

    d->selectedItemsTask =
        QtConcurrent::run(

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                          &ItemPropertiesSideBarDB::calculateItemsMultithreaded, this

#else

                          this, &ItemPropertiesSideBarDB::calculateItemsMultithreaded

#endif

                         );
}

void ItemPropertiesSideBarDB::calculateItemsMultithreaded()
{
    // -- Items Selection Properties

    Q_EMIT signalSetSelectionCount(d->currentInfos.count());

    qint64 selectionFileSize   = 0;
    auto restInfos             = d->allInfos;
    ItemInfoList selectionGroups;
    QHash<QString, int> selectionMimes;

    for (const ItemInfo& info : std::as_const(d->currentInfos))
    {
        if (d->selectedItemsTask.isCanceled())
        {
            return;
        }

        // cppcheck-suppress useStlAlgorithm
        selectionFileSize += info.fileSize();
        selectionMimes.insert(info.format(), selectionMimes.value(info.format()) + 1);

        if (info.hasGroupedImages())
        {
            selectionGroups.append(info);
        }

        restInfos.removeAll(info);
    }

    Q_EMIT signalSetSelectionSize(ItemPropertiesTab::humanReadableBytesCount(selectionFileSize));
    Q_EMIT signalSetSelectionMimes(selectionMimes);
    Q_EMIT signalSetSelectionGroups(selectionGroups);

    // -- Total Album Items Properties

    Q_EMIT signalSetTotalCount(d->allInfos.count());

    qint64 totalFileSize           = selectionFileSize;
    ItemInfoList totalGroups       = selectionGroups;
    QHash<QString, int> totalMimes = selectionMimes;

    for (const ItemInfo& info : std::as_const(restInfos))
    {
        if (d->selectedItemsTask.isCanceled())
        {
            return;
        }

        // cppcheck-suppress useStlAlgorithm
        totalFileSize += info.fileSize();
        totalMimes.insert(info.format(), totalMimes.value(info.format()) + 1);

        if (info.hasGroupedImages())
        {
            totalGroups.append(info);
        }
    }

    Q_EMIT signalSetTotalSize(ItemPropertiesTab::humanReadableBytesCount(totalFileSize));
    Q_EMIT signalSetTotalMimes(totalMimes);

    Q_EMIT signalSetTotalGroups(totalGroups);
}

ItemPropertiesVersionsTab* ItemPropertiesSideBarDB::getFiltersHistoryTab() const
{
    return d->versionsHistoryTab;
}

ItemDescEditTab* ItemPropertiesSideBarDB::imageDescEditTab() const
{
    return d->desceditTab;
}

void ItemPropertiesSideBarDB::doLoadState()
{
    ItemPropertiesSideBar::doLoadState();

    KConfigGroup group                = getConfigGroup();
    KConfigGroup groupVersionTab      = KConfigGroup(&group, entryName(QLatin1String("Version Properties Tab")));
    d->versionsHistoryTab->readSettings(groupVersionTab);

    KConfigGroup groupCaptionsTagsTab = KConfigGroup(&group, entryName(QLatin1String("Captions Tags Properties Tab")));
    d->desceditTab->readSettings(groupCaptionsTagsTab);
}

void ItemPropertiesSideBarDB::doSaveState()
{
    ItemPropertiesSideBar::doSaveState();

    KConfigGroup group           = getConfigGroup();
    KConfigGroup groupVersionTab = KConfigGroup(&group, entryName(QLatin1String("Version Properties Tab")));
    d->versionsHistoryTab->writeSettings(groupVersionTab);

    KConfigGroup groupCaptionsTagsTab = KConfigGroup(&group, entryName(QLatin1String("Captions Tags Properties Tab")));
    d->desceditTab->writeSettings(groupCaptionsTagsTab);
}

void ItemPropertiesSideBarDB::slotPopupTagsView()
{
    setActiveTab(d->desceditTab);
    d->desceditTab->setFocusToTagsView();
}

#ifdef HAVE_GEOLOCATION

bool ItemPropertiesSideBarDB::GPSItemInfofromItemInfo(const ItemInfo& imageInfo, GPSItemInfo* const gpsItemInfo)
{
    const ItemPosition pos = imageInfo.imagePosition();

    if (pos.isEmpty() || !pos.hasCoordinates())
    {
        return false;
    }

    gpsItemInfo->coordinates.setLatLon(pos.latitudeNumber(), pos.longitudeNumber());

    if (pos.hasAltitude())
    {
        gpsItemInfo->coordinates.setAlt(pos.altitude());
    }

    gpsItemInfo->dateTime  = imageInfo.dateTime();
    gpsItemInfo->rating    = imageInfo.rating();
    gpsItemInfo->url       = imageInfo.fileUrl();
    gpsItemInfo->id        = imageInfo.id();

    return true;
}

#endif // HAVE_GEOLOCATION

void ItemPropertiesSideBarDB::slotPropertiesButtonPressed(int index)
{
    switch (index)
    {
        case ItemPropertiesTab::digiKamProperties:
        {
            setActiveTab(d->desceditTab);
            d->desceditTab->setCurrentTab(ItemDescEditTab::DESCRIPTIONS);
            break;
        }

        case ItemPropertiesTab::TagsProperties:
        {
            setActiveTab(d->desceditTab);
            d->desceditTab->setCurrentTab(ItemDescEditTab::TAGS);
            break;
        }

        case ItemPropertiesTab::LocationProperties:
        case ItemPropertiesTab::RightProperties:
        {
            setActiveTab(d->desceditTab);
            d->desceditTab->setCurrentTab(ItemDescEditTab::INFOS);
            break;
        }

        default:
        {
            break;
        }
    }
}

void ItemPropertiesSideBarDB::slotOpenGeolocationMap()
{

#ifdef HAVE_GEOLOCATION

    setActiveTab(m_gpsTab);

#endif // HAVE_GEOLOCATION

}

} // namespace Digikam

#include "moc_itempropertiessidebardb.cpp"
