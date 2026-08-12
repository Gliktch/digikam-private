/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : interface to database information for shared tools.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 * SPDX-FileCopyrightText: 2017      by Mario Frank <mario dot frank at uni minus potsdam dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbinfoiface.h"

// Qt includes

#include <QDir>
#include <QStandardPaths>

// Local includes

#include "albumselecttabs.h"
#include "album.h"
#include "albummodel.h"
#include "albummanager.h"
#include "albumfiltermodel.h"
#include "albumselectwidget.h"
#include "albumparser.h"
#include "coredb.h"
#include "collectionmanager.h"
#include "coredbnamefilter.h"
#include "collectionscanner.h"
#include "digikamapp.h"
#include "digikam_debug.h"
#include "itemattributeswatch.h"
#include "itemiconview.h"
#include "itemcomments.h"
#include "itemcopyright.h"
#include "iteminfo.h"
#include "photoinfocontainer.h"
#include "videoinfocontainer.h"
#include "itemposition.h"
#include "itemsortsettings.h"
#include "coredbsearchxml.h"
#include "itemlister.h"
#include "itemlisterreceiver.h"
#include "dio.h"
#include "disjointmetadata.h"
#include "fileactionmngr.h"
#include "tagsactionmngr.h"
#include "setup.h"
#include "applicationsettings.h"

#ifdef HAVE_GEOLOCATION
#   include "itemgps.h"
#endif

namespace Digikam
{

class Q_DECL_HIDDEN DBInfoIface::Private
{
public:

    Private() = default;

public:

    AlbumManager*        albumManager     = AlbumManager::instance();
    AlbumSelectTabs*     albumsChooser    = nullptr;
    AlbumSelectWidget*   albumSelector    = nullptr;

    QList<QUrl>          itemUrls;

    OperationType        operationType    = UnspecifiedOps;
    bool                 withGroupedIsSet = false;
    bool                 withGrouped      = false;

public:

    /**
     * get the images from the Physical album in database and return the items found.
     */
    QList<QUrl> imagesFromPAlbum(const PAlbum* const album) const
    {
        // get the images from the database and return the items found

        CoreDB::ItemSortOrder sortOrder = CoreDB::NoItemSorting;

        switch (ApplicationSettings::instance()->getImageSortOrder())
        {
            default:
            case ItemSortSettings::SortByFileName:
            {
                sortOrder = CoreDB::ByItemName;
                break;
            }

            case ItemSortSettings::SortByFilePath:
            {
                sortOrder = CoreDB::ByItemPath;
                break;
            }

            case ItemSortSettings::SortByCreationDate:
            {
                sortOrder = CoreDB::ByItemDate;
                break;
            }

            case ItemSortSettings::SortByRating:
            {
                sortOrder = CoreDB::ByItemRating;
                break;
            }

            // NOTE: ByISize not supported
        }

        QStringList list = CoreDbAccess().db()->getItemURLsInAlbum(album->id(), sortOrder);
        QList<QUrl> urlList;
        CoreDbNameFilter nameFilter(ApplicationSettings::instance()->getAllFileFilter());

        for (QStringList::const_iterator it = list.constBegin() ; it != list.constEnd() ; ++it)
        {
            if (nameFilter.matches(*it))
            {
                urlList << QUrl::fromLocalFile(*it);
            }
        }

        return urlList;
    }

    /**
     * get the images from the Tags album in database and return the items found.
     */
    QList<QUrl> imagesFromTAlbum(const TAlbum* const album) const
    {
        QStringList list = CoreDbAccess().db()->getItemURLsInTag(album->id());
        QList<QUrl> urlList;
        CoreDbNameFilter nameFilter(ApplicationSettings::instance()->getAllFileFilter());

        for (QStringList::const_iterator it = list.constBegin() ; it != list.constEnd() ; ++it)
        {
            if (nameFilter.matches(*it))
            {
                urlList << QUrl::fromLocalFile(*it);
            }
        }

        return urlList;
    }

    /**
     * get the images from the search album in database and return the items found.
     */
    QList<QUrl> imagesFromSAlbum(const SAlbum* const album) const
    {
        QList<QUrl> urlList;
        CoreDbNameFilter nameFilter(ApplicationSettings::instance()->getAllFileFilter());

        if (album->isDuplicatesSearch())
        {
            // duplicates search album -> get the id list from the query

            SearchXmlReader reader(album->query());
            reader.readToFirstField();

            // Get the defined image ids.

            QList<int> list;
            list << reader.valueToIntList();

            for (int imageId : std::as_const(list))
            {
                ItemInfo imageInfo(imageId);

                if (imageInfo.isVisible())
                {
                    // if the image is visible, i.e. existent and not deleted,
                    // add the url if the name filter matches

                    QUrl imageUrl = imageInfo.fileUrl();
/*
                    qCDebug(DIGIKAM_GENERAL_LOG) << "Duplicates search Image url "
                                                 << imageUrl.toString();
*/
                    if (nameFilter.matches(imageUrl.toString()))
                    {
                        urlList << imageUrl;
                    }
                }
            }
        }
        else
        {
            // If we do not have a duplicates search, we use the image lister to get the images.

            ItemLister lister;
            lister.setListOnlyAvailable(true);

            ItemListerValueListReceiver receiver;

            if (album->searchType() == DatabaseSearch::HaarSearch)
            {
                lister.listHaarSearch(&receiver, album->query());
            }
            else
            {
                lister.listSearch(&receiver, album->query(), 0, -1);
            }

            if (!receiver.hasError)
            {
                // if there were no error, fetch and process the results.

                for (const ItemListerRecord& record : std::as_const(receiver.records))
                {
                    ItemInfo imageInfo(record);
                    QUrl imageUrl = imageInfo.fileUrl();
/*
                    qCDebug(DIGIKAM_GENERAL_LOG) << "Fuzzy/other search Image url "
                                                 << imageUrl.toString();
*/
                    if (nameFilter.matches(imageUrl.toString()))
                    {
                        urlList << imageUrl;
                    }
                }
            }
        }

        return urlList;
    }

    /**
     * Remove grouped images if user chose/chooses to.
     */
    QList<QUrl> resolveGroupsFromAlbums(const QList<QUrl>& urlList)
    {
        QList<QUrl> lst = urlList;

        if (!(withGroupedIsSet && withGrouped))
        {
            for (const QUrl& url : std::as_const(urlList))
            {
                ItemInfo info = ItemInfo::fromUrl(url);

                if (info.hasGroupedImages())
                {
                    if (!withGroupedIsSet)
                    {
                        withGroupedIsSet = true;
                        withGrouped      = ApplicationSettings::instance()->askGroupingOperateOnAll(operationType);

                        if (withGrouped)
                        {
                            break;
                        }
                    }

                    const auto imgs = info.groupedImages();

                    for (const ItemInfo& i : imgs)
                    {
                        lst.removeOne(i.fileUrl());
                    }
                }
            }
        }

        return lst;
    }

    bool includeGroupedFromSelected()
    {
        if (withGroupedIsSet)
        {
            return withGrouped;
        }

        withGroupedIsSet = true;
        withGrouped      = DigikamApp::instance()->view()->selectedNeedGroupResolving(operationType);

        return withGrouped;
    }
};

DBInfoIface::DBInfoIface(QObject* const parent, const QList<QUrl>& lst,
                         const OperationType type)
    : DInfoInterface(parent),
      d             (new Private)
{
    setObjectName(QLatin1String("DBInfoIface"));
    d->itemUrls      = lst;
    d->operationType = type;

    // Forward signal to DPluginAction of Digikam

    connect(TagsActionMngr::defaultManager(), &TagsActionMngr::signalShortcutPressed,
            this, &DBInfoIface::signalShortcutPressed);

    connect(TagsActionMngr::defaultManager(), &TagsActionMngr::signalColorLabelNamesUpdated,
            this, [this]()
        {
            Q_EMIT signalColorLabelNamesUpdated(TagsActionMngr::defaultManager()->colorLabelNames());
        }
    );
}

DBInfoIface::~DBInfoIface()
{
    delete d;
}

void DBInfoIface::slotDateTimeForUrl(const QUrl& url, const QDateTime& dt, bool updModDate)
{
    ItemInfo info = ItemInfo::fromUrl(url);

    if (!info.isNull())
    {
        info.setDateTime(dt);

        if (updModDate)
        {
            info.setModDateTime(dt);
        }
        else
        {
            QFileInfo finfo(url.toLocalFile());
            info.setModDateTime(finfo.lastModified());
        }
    }

    ItemAttributesWatch::instance()->fileMetadataChanged(url);
}

void DBInfoIface::slotMetadataChangedForUrl(const QUrl& url)
{
    // Refresh Database with new metadata from file.

    CollectionScanner scanner;

    scanner.scanFile(url.toLocalFile(), CollectionScanner::Rescan);
    ItemAttributesWatch::instance()->fileMetadataChanged(url);
}

void DBInfoIface::parseAlbumItemsRecursive()
{
    Album* const currAlbum = d->albumManager->currentAlbums().constFirst();

    if (currAlbum)
    {
        AlbumParser* const parser = new AlbumParser(currAlbum);

        connect(parser, SIGNAL(signalComplete(QList<QUrl>)),
                this, SIGNAL(signalAlbumItemsRecursiveCompleted(QList<QUrl>)));

        parser->run();
    }
}

QList<QUrl> DBInfoIface::currentAlbumItems() const
{
    if (!d->itemUrls.isEmpty())
    {
        return d->itemUrls;
    }

    if (d->albumManager->currentAlbums().isEmpty())
    {
        return QList<QUrl>();
    }

    QList<QUrl> imageList = DigikamApp::instance()->view()->allUrls(d->includeGroupedFromSelected());

    if (imageList.isEmpty())
    {
        Album* const currAlbum = d->albumManager->currentAlbums().constFirst();
        imageList              = d->resolveGroupsFromAlbums(albumItems(currAlbum));
    }

    return imageList;
}

QList<QUrl> DBInfoIface::currentSelectedItems() const
{
    if (!d->itemUrls.isEmpty())
    {
        return d->itemUrls;
    }

    return DigikamApp::instance()->view()->selectedUrls(d->includeGroupedFromSelected());
}

QList<QUrl> DBInfoIface::allAlbumItems() const
{
    QList<QUrl> imageList;

    const AlbumList palbumList = d->albumManager->allPAlbums();

    for (AlbumList::ConstIterator it = palbumList.constBegin() ;
         it != palbumList.constEnd() ; ++it)
    {
        // don't add the root album

        if ((*it)->isRoot())
        {
            continue;
        }

        PAlbum* const p = dynamic_cast<PAlbum*>(*it);

        if (p)
        {
            imageList.append(d->imagesFromPAlbum(p));
        }
    }

    return imageList;
}

DBInfoIface::DInfoMap DBInfoIface::albumInfo(int gid) const
{
    Album* const a = d->albumManager->findAlbum(gid);

    if (!a)
    {
        return DInfoMap();
    }

    DInfoMap map;
    map.insert(QLatin1String("title"), a->title());

    const PAlbum* const p = dynamic_cast<PAlbum*>(a);

    if (p)
    {
        map.insert(QLatin1String("caption"),   p->caption());
        map.insert(QLatin1String("date"),      p->date());
        map.insert(QLatin1String("path"),      p->folderPath());
        map.insert(QLatin1String("albumpath"), p->albumPath());
    }

    return map;
}

DBInfoIface::DInfoMap DBInfoIface::itemInfo(const QUrl& url) const
{
    DInfoMap map;

    ItemInfo info = ItemInfo::fromUrl(url);

    if (!info.isNull())
    {
        map.insert(QLatin1String("name"),               info.name());
        map.insert(QLatin1String("title"),              info.title());
        map.insert(QLatin1String("albumid"),            info.albumId());
        map.insert(QLatin1String("comment"),            info.comment());
        map.insert(QLatin1String("orientation"),        info.orientation());
        map.insert(QLatin1String("datetime"),           info.dateTime());
        map.insert(QLatin1String("rating"),             info.rating());
        map.insert(QLatin1String("colorlabel"),         info.colorLabel());
        map.insert(QLatin1String("picklabel"),          info.pickLabel());
        map.insert(QLatin1String("filesize"),           info.fileSize());
        map.insert(QLatin1String("dimensions"),         info.dimensions());

        // Get digiKam Tags Path list of picture from database.
        // Ex.: "City/Paris/Monuments/Notre Dame"

        QList<int> tagIds            = info.tagIds();
        QStringList tagspath         = AlbumManager::instance()->tagPaths(tagIds, false);
        map.insert(QLatin1String("tagspath"),           tagspath);

        // Get digiKam Tags name (keywords) list of picture from database.
        // Ex.: "Notre Dame"

        QStringList tags             = AlbumManager::instance()->tagNames(tagIds);
        map.insert(QLatin1String("keywords"),           tags);

        // Get GPS location of picture from database.

        ItemPosition pos             = info.imagePosition();

        if (!pos.isEmpty())
        {
            map.insert(QLatin1String("latitude"),       pos.latitudeNumber());
            map.insert(QLatin1String("longitude"),      pos.longitudeNumber());
            map.insert(QLatin1String("altitude"),       pos.altitude());
        }

        // Get Copyright information of picture from database.

        ItemCopyright rights         = info.imageCopyright();
        map.insert(QLatin1String("creators"),           rights.creator());
        map.insert(QLatin1String("credit"),             rights.credit());
        map.insert(QLatin1String("rights"),             rights.rights());
        map.insert(QLatin1String("source"),             rights.source());

        PhotoInfoContainer photoInfo = info.photoInfoContainer();
        map.insert(QLatin1String("lens"),               photoInfo.lens);
        map.insert(QLatin1String("make"),               photoInfo.make);
        map.insert(QLatin1String("model"),              photoInfo.model);
        map.insert(QLatin1String("exposuretime"),       photoInfo.exposureTime);
        map.insert(QLatin1String("sensitivity"),        photoInfo.sensitivity);
        map.insert(QLatin1String("aperture"),           photoInfo.aperture);
        map.insert(QLatin1String("focallength"),        photoInfo.focalLength);
        map.insert(QLatin1String("focalLength35mm"),    photoInfo.focalLength35mm);

        // TODO: add more video metadata as needed

        VideoInfoContainer videoInfo = info.videoInfoContainer();
        map.insert(QLatin1String("videocodec"),         videoInfo.videoCodec);

        // Get complex text containers.

        ItemComments comments = info.imageComments(CoreDbAccess());
        CaptionsMap titles    = comments.toCaptionsMap(DatabaseComment::Title);
        map.insert(QLatin1String("titles"),             QVariant::fromValue(titles));
        CaptionsMap captions  = comments.toCaptionsMap(DatabaseComment::Comment);
        map.insert(QLatin1String("captions"),           QVariant::fromValue(captions));

        Template tpl                      = info.metadataTemplate();
        MetaEngine::AltLangMap copyrights = tpl.copyright();
        map.insert(QLatin1String("copyrights"),         QVariant::fromValue(copyrights));
        MetaEngine::AltLangMap notices    = tpl.rightUsageTerms();
        map.insert(QLatin1String("copyrightnotices"),   QVariant::fromValue(notices));

        qCDebug(DIGIKAM_GENERAL_LOG) << "Database Info populated for" << url;
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Database Info is null for" << url;
    }

    return map;
}

void DBInfoIface::setItemInfo(const QUrl& url, const DInfoMap& map)
{
    ItemInfo info    = ItemInfo::fromUrl(url);
    QStringList keys = map.keys();
    DisjointMetadata hub;
    hub.load(info);

    qCDebug(DIGIKAM_GENERAL_LOG) << "DBInfoIface::setItemInfo() keys:" << keys;

    if (map.contains(QLatin1String("orientation")))
    {
        FileActionMngr::instance()->setExifOrientation(QList<ItemInfo>() << info,
                                                       map[QLatin1String("orientation")].toInt());
        keys.removeAll(QLatin1String("orientation"));
    }

    if (map.contains(QLatin1String("rating")))
    {
        hub.setRating(map[QLatin1String("rating")].toInt());
        keys.removeAll(QLatin1String("rating"));
    }

    if (map.contains(QLatin1String("colorlabel")))
    {
        hub.setColorLabel(map[QLatin1String("colorlabel")].toInt());
        keys.removeAll(QLatin1String("colorlabel"));
    }

    if (map.contains(QLatin1String("picklabel")))
    {
        hub.setPickLabel(map[QLatin1String("picklabel")].toInt());
        keys.removeAll(QLatin1String("picklabel"));
    }

    // NOTE: For now tag doesn't really exist anywhere else apart from digikam
    // therefore it's not really necessary to implement accessor method in InfoIface.

    if (map.contains(QLatin1String("tag")))
    {
        hub.setTag(map[QLatin1String("tag")].toInt());
        keys.removeAll(QLatin1String("tag"));
    }

    if (map.contains(QLatin1String("titles")))
    {
        hub.setTitles(qvariant_cast<CaptionsMap>(map[QLatin1String("titles")]));
        keys.removeAll(QLatin1String("titles"));
    }

    if (map.contains(QLatin1String("captions")))
    {
        hub.setComments(qvariant_cast<CaptionsMap>(map[QLatin1String("captions")]));
        keys.removeAll(QLatin1String("captions"));
    }

    Template tpl = hub.metadataTemplate();

    if (map.contains(QLatin1String("copyrights")))
    {
        tpl.setCopyright(qvariant_cast<MetaEngine::AltLangMap>(map[QLatin1String("copyrights")]));
        hub.setMetadataTemplate(tpl);
        keys.removeAll(QLatin1String("copyrights"));
    }

    if (map.contains(QLatin1String("copyrightnotices")))
    {
        tpl.setRightUsageTerms(qvariant_cast<MetaEngine::AltLangMap>(map[QLatin1String("copyrightnotices")]));
        hub.setMetadataTemplate(tpl);
        keys.removeAll(QLatin1String("copyrightnotices"));
    }

    if (!keys.isEmpty())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Keys not yet supported in DBInfoIface::setItemInfo():" << keys;
    }

    // We write to the database immediately because
    // the FileActionMngr writes with a delay.

    if (hub.write(info, DisjointMetadata::PartialWrite))
    {
        FileActionMngr::instance()->applyMetadata(QList<ItemInfo>() << info, hub);
    }
}

QList<QUrl> DBInfoIface::albumItems(Album* const album) const
{
    if (!album)
    {
        return QList<QUrl>();
    }

    QList<QUrl> imageList;

    switch (album->type())
    {
        case Album::PHYSICAL:
        {
            PAlbum* const p = dynamic_cast<PAlbum*>(album);

            if (p)
            {
                imageList = d->imagesFromPAlbum(p);
            }

            break;
        }

        case Album::TAG:
        {
            TAlbum* const p = dynamic_cast<TAlbum*>(album);

            if (p)
            {
                imageList = d->imagesFromTAlbum(p);
            }

            break;
        }

        case Album::SEARCH:
        {
            SAlbum* const s = dynamic_cast<SAlbum*>(album);

            if (s)
            {
                imageList = d->imagesFromSAlbum(s);
            }

            break;
        }

        default:
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown album type";
            break;
        }
    }

    return d->resolveGroupsFromAlbums(imageList);
}

QList<QUrl> DBInfoIface::albumItems(int id) const
{
    return albumItems(d->albumManager->findAlbum(id));
}

QList<QUrl> DBInfoIface::albumsItems(const DAlbumIDs& lst) const
{
    QList<QUrl> imageList;

    for (int gid : std::as_const(lst))
    {
        imageList << albumItems(gid);
    }

    return d->resolveGroupsFromAlbums(imageList);
}

QWidget* DBInfoIface::albumChooser(QWidget* const parent) const
{
    if (!d->albumsChooser)
    {
        d->albumsChooser = new AlbumSelectTabs(objectName(), parent);
    }

    connect(d->albumsChooser, SIGNAL(signalAlbumSelectionChanged()),
            this, SIGNAL(signalAlbumChooserSelectionChanged()));

    return d->albumsChooser;
}

DBInfoIface::DAlbumIDs DBInfoIface::albumChooserItems() const
{
    if (!d->albumsChooser)
    {
        return DAlbumIDs();
    }

    AlbumList lst = d->albumsChooser->selectedAlbums();
    DAlbumIDs ids;

    for (const Album* const a : std::as_const(lst))
    {
        if (a)
        {
            ids << a->globalID();
        }
    }

    return ids;
}

bool DBInfoIface::supportAlbums() const
{
    return true;
}

QWidget* DBInfoIface::uploadWidget(QWidget* const parent) const
{
    if (!d->albumSelector)
    {
        d->albumSelector = new AlbumSelectWidget(parent);

        connect(d->albumSelector, SIGNAL(itemSelectionChanged()),
                this, SIGNAL(signalUploadUrlChanged()));
    }

    return d->albumSelector;
}

QUrl DBInfoIface::uploadUrl() const
{
    QUrl url;

    if (d->albumSelector)
    {
        const PAlbum* const p = d->albumSelector->currentAlbum();

        if (p)
        {
            url = QUrl::fromLocalFile(p->folderPath());
        }
    }

    return url;
}

QUrl DBInfoIface::defaultUploadUrl() const
{
    QUrl place       = QUrl::fromLocalFile(QDir::homePath());
    QStringList pics = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

    if (!pics.isEmpty())
    {
        place = QUrl::fromLocalFile(pics.first());
    }

    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();
    Album* album            = nullptr;

    if (!albumList.isEmpty())
    {
        album = albumList.first();
    }

    if (album && (album->type() == Album::PHYSICAL))
    {
        const PAlbum* const p = dynamic_cast<PAlbum*>(album);

        if (p)
        {
            place = QUrl::fromLocalFile(p->folderPath());
        }
    }
    else
    {
        QStringList cols = CollectionManager::instance()->allAvailableAlbumRootPaths();

        if (!cols.isEmpty())
        {
            place = QUrl::fromLocalFile(cols.first());
        }
    }

    return place;
}

QAbstractItemModel* DBInfoIface::tagFilterModel()
{
    TagModel* const tagModel                    = new TagModel(AbstractAlbumModel::IgnoreRootAlbum, this);
    TagPropertiesFilterModel* const filterModel = new TagPropertiesFilterModel(this);
    filterModel->setSourceAlbumModel(tagModel);
    filterModel->sort(0);

    return filterModel;
}

#ifdef HAVE_GEOLOCATION

QList<GPSItemContainer*> DBInfoIface::currentGPSItems() const
{
    QList<GPSItemContainer*> items;
    const auto urls = currentSelectedItems();

    for (const QUrl& url : urls)
    {
        ItemInfo info = ItemInfo::fromUrl(url);
        items << new ItemGPS(info);
    }

    return items;
}

#endif

QMap<QString, QString> DBInfoIface::passShortcutActionsToWidget(QWidget* const wdg) const
{
    TagsActionMngr::defaultManager()->registerActionsToWidget(wdg);

    QMap<QString, QString> shortcutPrefixes;
    shortcutPrefixes.insert(QLatin1String("rating"),     TagsActionMngr::defaultManager()->ratingShortcutPrefix());
    shortcutPrefixes.insert(QLatin1String("tag"),        TagsActionMngr::defaultManager()->tagShortcutPrefix());
    shortcutPrefixes.insert(QLatin1String("picklabel"),  TagsActionMngr::defaultManager()->pickShortcutPrefix());
    shortcutPrefixes.insert(QLatin1String("colorlabel"), TagsActionMngr::defaultManager()->colorShortcutPrefix());

    return shortcutPrefixes;
}

void DBInfoIface::deleteImage(const QUrl& url)
{
    ItemInfo info = ItemInfo::fromUrl(url);

    DIO::del(info, true);
}

void DBInfoIface::openSetupPage(SetupPage page)
{
    switch (page)
    {
        case ExifToolPage:
        {
            if (Setup::execExifTool(nullptr))
            {
                Q_EMIT signalSetupChanged();
            }

            break;
        }

        default:
        {
            break;
        }
    }
}

} // namespace Digikam

#include "moc_dbinfoiface.cpp"
