/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-24
 * Description : Qt Model for Albums - filter model
 *
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009      by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albumfiltermodel_p.h"

namespace Digikam
{

AlbumFilterModel::AlbumFilterModel(QObject* const parent)
    : QSortFilterProxyModel(parent),
      m_parent             (parent)
{
    setSortRole(AbstractAlbumModel::AlbumSortRole);
    setSortCaseSensitivity(Qt::CaseInsensitive);

    // sorting may have changed when the string comparison is different

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(invalidate()));

    // dynamicSortFilter does not work well for us: a dataChange may, because of our way of filtering,
    // also affect parents and children of the changed index, which is not handled by QSortFilterProxyModel.

    setDynamicSortFilter(false);

    // Instead, we listen directly to AlbumManager's relevant change signals

    connect(AlbumManager::instance(), SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsUpdated(int)),
            this, SLOT(slotAlbumsHaveBeenUpdated(int)));
}

void AlbumFilterModel::setFilterBehavior(FilterBehavior behavior)
{
    if (behavior == m_filterBehavior)
    {
        return;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    beginFilterChange();

#endif

    m_filterBehavior = behavior;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    endFilterChange(QSortFilterProxyModel::Direction::Rows);

#else

    invalidateFilter();

#endif

}

void AlbumFilterModel::setSearchTextSettings(const SearchTextSettings& settings)
{
    if (!sourceModel())
    {
        return;
    }

    // Don't use isFiltering here because it may be reimplemented

    bool wasSearching = settingsFilter(m_settings);
    bool willSearch   = settingsFilter(settings);

    Q_EMIT searchTextSettingsAboutToChange(wasSearching, willSearch);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    beginFilterChange();

#endif

    m_settings = settings;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    endFilterChange(QSortFilterProxyModel::Direction::Rows);

#else

    invalidateFilter();

#endif

    Q_EMIT signalFilterChanged();

    Q_EMIT searchTextSettingsChanged(wasSearching, willSearch);

    if (sourceAlbumModel()->albumType() == Album::PHYSICAL)
    {
        // find out if this setting has some results or not

        int validRows = 0;

        // for every collection we got

        for (int i = 0 ; i < rowCount(rootAlbumIndex()) ; ++i)
        {
            QModelIndex collectionIndex = index(i, 0, rootAlbumIndex());

            // count the number of rows

            validRows                  += rowCount(collectionIndex);
        }

        bool hasResult = (validRows > 0);
        qCDebug(DIGIKAM_GENERAL_LOG) << "new search text settings:" << settings.text
                                     << ": hasResult =" << hasResult
                                     << ", validRows =" << validRows;

        Q_EMIT hasSearchResult(hasResult);
    }
    else
    {
        QModelIndex head = rootAlbumIndex(); // either root, or invalid, thus toplevel

        Q_EMIT hasSearchResult(rowCount(head));
    }

    if (settings.text.isEmpty())
    {
        sort(0, sortOrder());
    }
}

bool AlbumFilterModel::settingsFilter(const SearchTextSettings& settings) const
{
    return (!settings.text.isEmpty());
}

bool AlbumFilterModel::isFiltering() const
{
    return (settingsFilter(m_settings));
}

void AlbumFilterModel::updateFilter()
{
    if (isFiltering())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

        beginFilterChange();
        endFilterChange(QSortFilterProxyModel::Direction::Rows);

#else

        invalidateFilter();

#endif

    }
}

SearchTextSettings AlbumFilterModel::searchTextSettings() const
{
    return m_settings;
}

void AlbumFilterModel::setSourceAlbumModel(AbstractAlbumModel* const source)
{
    if (m_chainedModel)
    {
        m_chainedModel->setSourceAlbumModel(source);
    }
    else
    {
        if (source != sourceModel())
        {
            setSourceModel(source);
        }
    }
}

void AlbumFilterModel::setSourceFilterModel(AlbumFilterModel* const source)
{
    if (source)
    {
        AbstractAlbumModel* const model = sourceAlbumModel();

        if (model)
        {
            source->setSourceAlbumModel(model);
        }
    }

    if ((m_chainedModel != source) || (sourceModel() != source))
    {
        m_chainedModel = source;
        setSourceModel(source);
    }
}

void AlbumFilterModel::setSourceModel(QAbstractItemModel* const model)
{
    // made it protected, only setSourceAlbumModel is public

    QSortFilterProxyModel::setSourceModel(model);
}

AbstractAlbumModel* AlbumFilterModel::sourceAlbumModel() const
{
    if (m_chainedModel)
    {
        return m_chainedModel->sourceAlbumModel();
    }

    return static_cast<AbstractAlbumModel*>(sourceModel());
}

AlbumFilterModel* AlbumFilterModel::sourceFilterModel() const
{
    return m_chainedModel;
}

QModelIndex AlbumFilterModel::mapToSourceAlbumModel(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    if (m_chainedModel)
    {
        return m_chainedModel->mapToSourceAlbumModel(mapToSource(index));
    }

    return mapToSource(index);
}

QModelIndex AlbumFilterModel::mapFromSourceAlbumModel(const QModelIndex& albummodel_index) const
{
    if (!albummodel_index.isValid())
    {
        return QModelIndex();
    }

    if (m_chainedModel)
    {
        return mapFromSource(m_chainedModel->mapFromSourceAlbumModel(albummodel_index));
    }

    return mapFromSource(albummodel_index);
}

Album* AlbumFilterModel::albumForIndex(const QModelIndex& index) const
{
    return AbstractAlbumModel::retrieveAlbum(index);
}

QModelIndex AlbumFilterModel::indexForAlbum(Album* album) const
{
    const AbstractAlbumModel* const model = sourceAlbumModel();

    if (!model)
    {
        return QModelIndex();
    }

    return mapFromSourceAlbumModel(model->indexForAlbum(album));
}

QModelIndex AlbumFilterModel::rootAlbumIndex() const
{
    const AbstractAlbumModel* const model = sourceAlbumModel();

    if (!model)
    {
        return QModelIndex();
    }

    return mapFromSourceAlbumModel(model->rootAlbumIndex());
}

QVariant AlbumFilterModel::dataForCurrentSortRole(Album* album) const
{
    if (album)
    {
        if      (album->type() == Album::PHYSICAL)
        {
            const PAlbum* const a = static_cast<PAlbum*>(album);

            ApplicationSettings::AlbumSortRole sortRole = ApplicationSettings::instance()->getAlbumSortRole();

            switch (sortRole)
            {
                case ApplicationSettings::ByFolder:
                {
                    return a->title();
                }

                case ApplicationSettings::ByDate:
                {
                    return a->date();
                }

                default:
                {
                    return a->category();
                }
            }
        }
        else if (album->type() == Album::TAG)
        {
            return static_cast<TAlbum*>(album)->title();
        }
        else if (album->type() == Album::DATE)
        {
            return static_cast<DAlbum*>(album)->date();
        }
        else if (album->type() == Album::SEARCH)
        {
            return static_cast<SAlbum*>(album)->title();
        }
    }

    return QVariant();
}

bool AlbumFilterModel::matches(Album* album) const
{
    // We want to work on the visual representation, so we use model data with AlbumTitleRole,
    // not a direct Album method.
    // We use direct source's index, not our index,
    // because if the item is currently filtered out, we won't have an index for this album.

    QModelIndex source_index = sourceAlbumModel()->indexForAlbum(album);

    if (m_chainedModel)
    {
        source_index = m_chainedModel->mapFromSourceAlbumModel(source_index);
    }

    QString displayTitle = source_index.data(AbstractAlbumModel::AlbumTitleRole).toString();

    return displayTitle.contains(m_settings.text, m_settings.caseSensitive);
}

AlbumFilterModel::MatchResult AlbumFilterModel::matchResult(const QModelIndex& index) const
{
    return matchResult(albumForIndex(index));
}

AlbumFilterModel::MatchResult AlbumFilterModel::matchResult(Album* album) const
{
    if (!album)
    {
        return NoMatch;
    }

    const PAlbum* const palbum = dynamic_cast<PAlbum*>(album);

    if (album->isRoot() || (palbum && palbum->isAlbumRoot()))
    {
        return SpecialMatch;
    }

    const TAlbum* const talbum = dynamic_cast<TAlbum*>(album);

    if (talbum && talbum->isInternalTag())
    {
        return NoMatch;
    }

    if (matches(album))
    {
        return DirectMatch;
    }

    if (m_filterBehavior == SimpleFiltering)
    {
        return NoMatch;
    }

    if (m_filterBehavior == FullFiltering)
    {
        // check if any of the parents match the search

        Album* parent               = album->parent();
        const PAlbum* const pparent = palbum ? static_cast<PAlbum*>(parent) : nullptr;

        while (parent && !(parent->isRoot() || (pparent && pparent->isAlbumRoot())))
        {
            if (matches(parent))
            {
                return ParentMatch;
            }

            parent = parent->parent();
        }
    }

    // check if any of the children match the search

    AlbumIterator it(album);

    while (it.current())
    {
        if (matches(*it))
        {
            return ChildMatch;
        }

        ++it;
    }

    return NoMatch;
}

bool AlbumFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index  = sourceModel()->index(source_row, 0, source_parent);
    Album* const album = AbstractAlbumModel::retrieveAlbum(index);
    MatchResult result = matchResult(album);

    return result;
}

bool AlbumFilterModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    Album* leftAlbum  = albumForIndex(left);
    Album* rightAlbum = albumForIndex(right);

    if (!leftAlbum || !rightAlbum)
    {
        return QSortFilterProxyModel::lessThan(left, right);
    }

    if ((leftAlbum->type() == Album::TAG) && (rightAlbum->type() == Album::TAG))
    {
        if (
            (leftAlbum->id()  == FaceTags::unconfirmedPersonTagId()) !=
            (rightAlbum->id() == FaceTags::unconfirmedPersonTagId())
           )
        {
            // unconfirmed tag album go to the top, regardless of sort role

            return (sortOrder() == Qt::AscendingOrder) ? (leftAlbum->id() == FaceTags::unconfirmedPersonTagId())
                                                       : (leftAlbum->id() != FaceTags::unconfirmedPersonTagId());
        }

        if (
            (leftAlbum->id() == FaceTags::unknownPersonTagId()) !=
            (rightAlbum->id() == FaceTags::unknownPersonTagId())
           )
        {
            // unknown tag albums go to the top, regardless of sort role

            return (sortOrder() == Qt::AscendingOrder) ? (leftAlbum->id() == FaceTags::unknownPersonTagId())
                                                       : (leftAlbum->id() != FaceTags::unknownPersonTagId());
        }

        // Verify this, to prevent auto-creation of Ignored Tag.

        if (
            (leftAlbum->id() == FaceTags::ignoredPersonTagId()) !=
            (rightAlbum->id() == FaceTags::ignoredPersonTagId())
           )
        {
            // ignored tag albums go to the top, regardless of sort role

            return (sortOrder() == Qt::AscendingOrder) ? (leftAlbum->id() == FaceTags::ignoredPersonTagId())
                                                       : (leftAlbum->id() != FaceTags::ignoredPersonTagId());
        }

        /**
         * Implementation to sort Tags that contain
         * Unconfirmed Faces, according to the Unconfirmed
         * Face Count.
         */

        if (sourceAlbumModel() && sourceAlbumModel()->isFaceTagModel())
        {
            QHash<int, int> unconfirmedFaceCount = AlbumManager::instance()->getUnconfirmedFaceCount();
            int leftFaceCount                    = unconfirmedFaceCount.value(leftAlbum->id(), 0);
            int rightFaceCount                   = unconfirmedFaceCount.value(rightAlbum->id(), 0);

            if ((leftFaceCount != 0) || (rightFaceCount != 0))
            {
                return (sortOrder() == Qt::AscendingOrder) ? (leftFaceCount > rightFaceCount)
                                                           : (leftFaceCount < rightFaceCount);
            }
        }
    }

    if (leftAlbum->isTrashAlbum() != rightAlbum->isTrashAlbum())
    {
        // trash albums go to the bottom, regardless of sort role

        return (
                (sortOrder() == Qt::AscendingOrder) ? !leftAlbum->isTrashAlbum()
                                                    : leftAlbum->isTrashAlbum()
               );
    }

    QVariant valLeft                        = dataForCurrentSortRole(leftAlbum);
    QVariant valRight                       = dataForCurrentSortRole(rightAlbum);
    ApplicationSettings::AlbumSortRole role = ApplicationSettings::instance()->getAlbumSortRole();

    if (
        ((role == ApplicationSettings::ByDate) || (role == ApplicationSettings::ByCategory)) &&
        (valLeft == valRight)
       )
    {
        return QSortFilterProxyModel::lessThan(left, right);
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    if ((valLeft.typeId() == QMetaType::QString) && (valRight.typeId() == QMetaType::QString))

#else

    if ((valLeft.type() == QVariant::String) && (valRight.type() == QVariant::String))

#endif

    {
        const ItemSortCollator* const sorter = ItemSortCollator::instance();
        bool natural                         = ApplicationSettings::instance()->isStringTypeNatural();

        return (sorter->albumCompare(valLeft.toString(), valRight.toString(), sortCaseSensitivity(), natural) < 0);
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    else if ((valLeft.typeId() == QMetaType::QDate) && (valRight.typeId() == QMetaType::QDate))

#else

    else if ((valLeft.type() == QVariant::Date) && (valRight.type() == QVariant::Date))

#endif

    {
        return (compareByOrder(valLeft.toDate(), valRight.toDate(), Qt::AscendingOrder) < 0);
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

void AlbumFilterModel::slotAlbumRenamed(Album* album)
{
    if (album)
    {
        slotAlbumsHaveBeenUpdated(album->type());
    }
}

void AlbumFilterModel::slotAlbumsHaveBeenUpdated(int type)
{
    if (sourceAlbumModel() && (sourceAlbumModel()->albumType() == type))
    {
        invalidate();
    }
}

} // namespace Digikam

#include "moc_albumfiltermodel.cpp"
