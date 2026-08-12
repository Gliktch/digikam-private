/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : digiKam album types
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "album.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dtrash.h"
#include "coredb.h"
#include "coredburl.h"
#include "coredbaccess.h"
#include "albummanager.h"
#include "collectionmanager.h"
#include "tagscache.h"

namespace Digikam
{

Album::Album(Album::Type type, int id, bool root)
    : m_root     (root),
      m_id       (id),
      m_type     (type)
{
}

Album::~Album()
{
    prepareForDeletion();

    AlbumManager::instance()->notifyAlbumDeletion(this);
}

void Album::setParent(Album* const _parent)
{
    if (_parent)
    {
        m_parent = _parent;
        m_parent->insertChild(this);
    }
}

Album* Album::parent() const
{
    return m_parent;
}

Album* Album::firstChild() const
{
    QReadLocker locker(&m_cacheLock);

    if (m_childCache.isEmpty())
    {
        return nullptr;
    }

    return m_childCache.constFirst();
}

Album* Album::lastChild() const
{
    QReadLocker locker(&m_cacheLock);

    if (m_childCache.isEmpty())
    {
        return nullptr;
    }

    return m_childCache.constLast();
}

Album* Album::next() const
{
    if (!m_parent)
    {
        return nullptr;
    }

    QReadLocker parentLocker(&m_parent->m_cacheLock);

    const int row = m_row + 1;

    if (row >= m_parent->m_childCache.size())
    {
        return nullptr;
    }

    return m_parent->m_childCache.at(row);
}

Album* Album::prev() const
{
    if (!m_parent)
    {
        return nullptr;
    }

    QReadLocker parentLocker(&m_parent->m_cacheLock);

    const int row = m_row - 1;

    if ((row < 0) || (row >= m_parent->m_childCache.size()))
    {
        return nullptr;
    }

    return m_parent->m_childCache.at(row);
}

Album* Album::childAtRow(int row) const
{
    QReadLocker locker(&m_cacheLock);

    if ((row < 0) || (row >= m_childCache.size()))
    {
        return nullptr;
    }

    return m_childCache.at(row);
}

AlbumList Album::childAlbums(bool recursive)
{
    AlbumList childList;

    QReadLocker locker(&m_cacheLock);

    QList<Album*>::const_iterator it = m_childCache.constBegin();

    while (it != m_childCache.constEnd())
    {
        childList << (*it);

        if (recursive)
        {
            childList << (*it)->childAlbums(recursive);
        }

        ++it;
    }

    return childList;
}

QList<int> Album::childAlbumIds(bool recursive)
{
    QList<int> ids;

    AlbumList childList = childAlbums(recursive);

    QListIterator<Album*> it(childList);

    while (it.hasNext())
    {
        ids << it.next()->id();
    }

    return ids;
}

// cppcheck-suppress constParameterPointer
void Album::insertChild(Album* const child)
{
    if (!child)
    {
        return;
    }

    QWriteLocker locker(&m_cacheLock);

    m_childCache.append(child);
    child->m_row = m_childCache.size() - 1;
}

void Album::removeChild(Album* const child)
{
    if (!child)
    {
        return;
    }

    QWriteLocker locker(&m_cacheLock);

    for (auto it = m_childCache.begin(); it != m_childCache.end(); ++it)
    {
        if ((*it) == child)
        {
            (*it)->m_row = 0;
            it           = m_childCache.erase(it);

            for ( ; it != m_childCache.end(); ++it)
            {
                (*it)->m_row -= 1;
            }

            break;
        }
    }
}

void Album::clear()
{
    QWriteLocker locker(&m_cacheLock);

    while (!m_childCache.isEmpty())
    {
        Album* const child = m_childCache.takeLast();
        child->m_parent    = nullptr;

        delete child;
    }
}

int Album::globalID() const
{
    return globalID(m_type, m_id);
}

int Album::globalID(Type _type, int _id)
{
    switch (_type)
    {
        // Use the upper bits to create unique ids.

        case (PHYSICAL):
        {
            return _id;
        }

        case (TAG):
        {
            return (_id | (1 << 27));
        }

        case (DATE):
        {
            return (_id | (1 << 28));
        }

        case (SEARCH):
        {
            return (_id | (1 << 29));
        }

        case (FACE):
        {
            return (_id | (1 << 30));
        }

        default:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Unknown album type";

            return -1;
        }
    }
}

int Album::id() const
{
    return m_id;
}

int Album::childCount() const
{
    QReadLocker locker(&m_cacheLock);

    return m_childCache.size();
}

int Album::rowFromAlbum() const
{
    return m_row;
}

void Album::setTitle(const QString& _title)
{
    m_title = _title;
}

QString Album::title() const
{
    return m_title;
}

Album::Type Album::type() const
{
    return m_type;
}

// cppcheck-suppress constParameterPointer
void Album::setExtraData(const void* const key, void* const value)
{
    m_extraMap.insert(key, value);
}

void Album::removeExtraData(const void* const key)
{
    m_extraMap.remove(key);
}

void* Album::extraData(const void* const key) const
{
    return m_extraMap.value(key, nullptr);
}

bool Album::isRoot() const
{
    return m_root;
}

bool Album::isAncestorOf(const Album* const album) const
{
    bool val       = false;
    const Album* a = album;

    while (a && !a->isRoot())
    {
        if (a == this)
        {
            val = true;
            break;
        }

        a = a->parent();
    }

    return val;
}

bool Album::isUsedByLabelsTree() const
{
    return m_usedByLabelsTree;
}

bool Album::isTrashAlbum() const
{
    if ((m_id < -1) && (m_type == PHYSICAL))
    {
        return true;
    }

    return false;
}

void Album::setUsedByLabelsTree(bool isUsed)
{
    m_usedByLabelsTree = isUsed;
}

void Album::prepareForDeletion()
{
    if (m_parent)
    {
        m_parent->removeChild(this);
    }

    clear();
}

// ------------------------------------------------------------------------------

int PAlbum::m_uniqueTrashId = -2;

PAlbum::PAlbum(const QString& title)
    : Album(Album::PHYSICAL, 0, true)
{
    setTitle(title);
    m_path.clear();
}

PAlbum::PAlbum(int albumRoot, const QString& label)
    : Album             (Album::PHYSICAL, -1, false),
      m_isAlbumRootAlbum(true),
      m_albumRootId     (albumRoot)
{
    // set the id to -1 (line above). AlbumManager may change that later.

    setTitle(label);
    m_path.clear();
}

PAlbum::PAlbum(int albumRoot, const QString& parentPath, const QString& title, int id)
    : Album             (Album::PHYSICAL, id, false),
      m_albumRootId     (albumRoot),
      m_path            (title),
      m_parentPath      (parentPath + QLatin1Char('/')),
      m_date            (QDate::currentDate())
{
    // If path is /holidays/2007, title is only "2007", path is "/holidays"

    setTitle(title);
}

PAlbum::PAlbum(const QString& parentPath, int albumRoot)
    : Album             (Album::PHYSICAL, m_uniqueTrashId--, false),
      m_albumRootId     (albumRoot),
      m_path            (DTrash::TRASH_FOLDER),
      m_parentPath      (parentPath + QLatin1Char('/'))
{
    setTitle(i18n("Trash"));
}

bool PAlbum::isAlbumRoot() const
{
    return m_isAlbumRootAlbum;
}

void PAlbum::setCaption(const QString& _caption)
{
    m_caption = _caption;

    CoreDbAccess access;
    access.db()->setAlbumCaption(id(), m_caption);
}

void PAlbum::setCategory(const QString& _category)
{
    m_category = _category;

    CoreDbAccess access;
    access.db()->setAlbumCategory(id(), m_category);
}

void PAlbum::setDate(const QDate& _date)
{
    m_date = _date;

    CoreDbAccess access;
    access.db()->setAlbumDate(id(), m_date);
}

QString PAlbum::albumRootPath() const
{
    return CollectionManager::instance()->albumRootPath(m_albumRootId);
}

QString PAlbum::albumRootLabel() const
{
    return CollectionManager::instance()->albumRootLabel(m_albumRootId);
}

int PAlbum::albumRootId() const
{
    return m_albumRootId;
}

QString PAlbum::caption() const
{
    return m_caption;
}

QString PAlbum::category() const
{
    return m_category;
}

QDate PAlbum::date() const
{
    return m_date;
}

QString PAlbum::albumPath() const
{
    return (m_parentPath + m_path);
}

CoreDbUrl PAlbum::databaseUrl() const
{
    return CoreDbUrl::fromAlbumAndName(QString(), albumPath(),
                                       QUrl::fromLocalFile(albumRootPath()), m_albumRootId);
}

QString PAlbum::prettyUrl() const
{
    QString u = i18n("Albums") + QLatin1Char('/') +
                                 albumRootLabel() +
                                 albumPath();

    if (u.endsWith(QLatin1Char('/')))
    {
        u.chop(1);
    }

    return u;
}

qlonglong PAlbum::iconId() const
{
    return m_iconId;
}

QUrl PAlbum::fileUrl() const
{
    return databaseUrl().fileUrl();
}

QString PAlbum::folderPath() const
{
    return databaseUrl().fileUrl().toLocalFile();
}

// --------------------------------------------------------------------------

TAlbum::TAlbum(const QString& title, int id, bool root)
    : Album(Album::TAG, id, root)
{
    setTitle(title);
}

QString TAlbum::tagPath(bool leadingSlash) const
{
    if (isRoot())
    {
        return (leadingSlash ? QLatin1String("/") : QLatin1String(""));
    }

    QString u;

    if (parent())
    {
        u = (static_cast<TAlbum*>(parent()))->tagPath(leadingSlash);

        if (!parent()->isRoot())
        {
            u += QLatin1Char('/');
        }
    }

    u += title();

    return u;
}

QString TAlbum::standardIconName() const
{
    return (hasProperty(TagPropertyName::person()) ? QLatin1String("smiley")
                                                   : QLatin1String("tag"));
}

QString TAlbum::prettyUrl() const
{
    return (i18n("Tags") + tagPath(true));
}

CoreDbUrl TAlbum::databaseUrl() const
{
    return CoreDbUrl::fromTagIds(tagIDs());
}

QList<int> TAlbum::tagIDs() const
{
    if      (isRoot())
    {
        return QList<int>();
    }
    else if (parent())
    {
        return (static_cast<TAlbum*>(parent())->tagIDs() << id());
    }
    else
    {
        return (QList<int>() << id());
    }
}

QString TAlbum::icon() const
{
    return m_icon;
}

bool TAlbum::isInternalTag() const
{
    return TagsCache::instance()->isInternalTag(id());
}

qlonglong TAlbum::iconId() const
{
    return m_iconId;
}

bool TAlbum::hasProperty(const QString& key) const
{
    return TagsCache::instance()->hasProperty(id(), key);
}

QString TAlbum::property(const QString& key) const
{
    return TagsCache::instance()->propertyValue(id(), key);
}

QMap<QString, QString> TAlbum::properties() const
{
    return TagsCache::instance()->properties(id());
}

// --------------------------------------------------------------------------

int DAlbum::m_uniqueID = 0;

DAlbum::DAlbum(const QDate& date, bool root, Range range)
    : Album  (Album::DATE, root ? 0 : ++m_uniqueID, root),
      m_date (date),
      m_range(range)
{
    // Set the name of the date album

    QString dateTitle;

    if (m_range == Month)
    {
        dateTitle = m_date.toString(QLatin1String("MMMM yyyy"));
    }
    else
    {
        dateTitle = m_date.toString(QLatin1String("yyyy"));
    }

    setTitle(dateTitle);
}

QDate DAlbum::date() const
{
    return m_date;
}

DAlbum::Range DAlbum::range() const
{
    return m_range;
}

CoreDbUrl DAlbum::databaseUrl() const
{
    if (m_range == Month)
    {
        return CoreDbUrl::fromDateForMonth(m_date);
    }

    return CoreDbUrl::fromDateForYear(m_date);
}

// --------------------------------------------------------------------------

SAlbum::SAlbum(const QString& title, int id, bool root)
    : Album(Album::SEARCH, id, root)
{
    setTitle(title);
}

void SAlbum::setSearch(DatabaseSearch::Type _type, const QString& _query)
{
    m_searchType = _type;
    m_query      = _query;
}

CoreDbUrl SAlbum::databaseUrl() const
{
    return CoreDbUrl::searchUrl(id());
}

QString SAlbum::query() const
{
    return m_query;
}

DatabaseSearch::Type SAlbum::searchType() const
{
    return m_searchType;
}

bool SAlbum::isNormalSearch() const
{
    switch (m_searchType)
    {
        case DatabaseSearch::KeywordSearch:
        case DatabaseSearch::AdvancedSearch:
        case DatabaseSearch::LegacyUrlSearch:
        {
            return true;
        }

        default:
        {
            return false;
        }
    }
}

bool SAlbum::isAdvancedSearch() const
{
    return (m_searchType == DatabaseSearch::AdvancedSearch);
}

bool SAlbum::isKeywordSearch() const
{
    return (m_searchType == DatabaseSearch::KeywordSearch);
}

bool SAlbum::isTimelineSearch() const
{
    return (m_searchType == DatabaseSearch::TimeLineSearch);
}

bool SAlbum::isHaarSearch() const
{
    return (m_searchType == DatabaseSearch::HaarSearch);
}

bool SAlbum::isMapSearch() const
{
    return (m_searchType == DatabaseSearch::MapSearch);
}

bool SAlbum::isDuplicatesSearch() const
{
    return (m_searchType == DatabaseSearch::DuplicatesSearch);
}

bool SAlbum::isTemporarySearch() const
{
    if (isHaarSearch())
    {
        return (
                (title() == getTemporaryHaarTitle(DatabaseSearch::HaarImageSearch))) ||
                (title() == getTemporaryHaarTitle(DatabaseSearch::HaarSketchSearch)
               );
    }

    return (title() == getTemporaryTitle(m_searchType));
}

QString SAlbum::displayTitle() const
{
    if (isTemporarySearch())
    {
        switch (m_searchType)
        {
            case DatabaseSearch::TimeLineSearch:
            {
                return i18n("Current Timeline Search");
            }

            case DatabaseSearch::HaarSearch:
            {
                if      (title() == getTemporaryHaarTitle(DatabaseSearch::HaarImageSearch))
                {
                    return i18n("Current Fuzzy Image Search");
                }
                else if (title() == getTemporaryHaarTitle(DatabaseSearch::HaarSketchSearch))
                {
                    return i18n("Current Fuzzy Sketch Search");
                }

                break;
            }

            case DatabaseSearch::MapSearch:
            {
                return i18n("Current Map Search");
            }

            case DatabaseSearch::KeywordSearch:
            case DatabaseSearch::AdvancedSearch:
            case DatabaseSearch::LegacyUrlSearch:
            {
                return i18n("Current Search");
            }

            case DatabaseSearch::DuplicatesSearch:
            {
                return i18n("Current Duplicates Search");
            }

            case DatabaseSearch::UndefinedType:
            {
                break;
            }
        }
    }

    return title();
}

QString SAlbum::getTemporaryTitle(DatabaseSearch::Type type, DatabaseSearch::HaarSearchType haarType)
{
    switch (type)
    {
        case DatabaseSearch::TimeLineSearch:
        {
            return QLatin1String("_Current_Time_Line_Search_");
        }

        case DatabaseSearch::HaarSearch:
        {
            return getTemporaryHaarTitle(haarType);
        }

        case DatabaseSearch::MapSearch:
        {
            return QLatin1String("_Current_Map_Search_");
        }

        case DatabaseSearch::KeywordSearch:
        case DatabaseSearch::AdvancedSearch:
        case DatabaseSearch::LegacyUrlSearch:
        {
            return QLatin1String("_Current_Search_View_Search_");
        }

        case DatabaseSearch::DuplicatesSearch:
        {
            return QLatin1String("_Current_Duplicates_Search_");
        }

        default:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Untreated temporary search type " << type;

            return QLatin1String("_Current_Unknown_Search_");
        }
    }
}

QString SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarSearchType haarType)
{
    switch (haarType)
    {
        case DatabaseSearch::HaarImageSearch:
        {
            return QLatin1String("_Current_Fuzzy_Image_Search_");
        }

        case DatabaseSearch::HaarSketchSearch:
        {
            return QLatin1String("_Current_Fuzzy_Sketch_Search_");
        }

        default:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Untreated temporary haar search type " << haarType;

            return QLatin1String("_Current_Unknown_Haar_Search_");
        }
    }
}

// --------------------------------------------------------------------------

AlbumIterator::AlbumIterator(Album* const album)
    : m_current(album ? album->firstChild() : nullptr),
      m_root   (album)
{
}

AlbumIterator& AlbumIterator::operator++()
{
    if (!m_current)
    {
        return *this;
    }

    Album* album = m_current->firstChild();

    if (!album)
    {
        while ((album = m_current->next()) == nullptr)
        {
            m_current = m_current->parent();

            if (m_current == m_root)
            {
                // we have reached the root.
                // that means no more children

                m_current = nullptr;
                break;
            }

            if (m_current == nullptr)
            {
                break;
            }
        }
    }

    m_current = album;

    return *this;
}

Album* AlbumIterator::operator*()
{
    return m_current;
}

Album* AlbumIterator::current() const
{
    return m_current;
}

} // namespace Digikam
