/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : albums history manager.
 *
 * SPDX-FileCopyrightText: 2004      by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albumhistory.h"

// Qt includes

#include <QMutexLocker>
#include <QString>
#include <QWidget>
#include <QHash>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "albummanager.h"

// NOTE: must be declared outside namespace.

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

inline size_t qHash(const QList<Digikam::Album*>& key)

#else

inline uint qHash(const QList<Digikam::Album*>& key)

#endif

{
    if (key.isEmpty())
    {
        return 0;
    }

    const Digikam::Album* const temp = key.first();
    quint64 myint                    = (unsigned long long)temp;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    size_t value                 = ::qHash(myint);

#else

    uint value                   = ::qHash(myint);

#endif

    for (int it = 1 ; it < key.size() ; ++it)
    {
        const Digikam::Album* const al = key.at(it);
        quint64 myint2                 = (unsigned long long)al;   // clazy:exclude=qt6-qhash-signature

        value                         ^= ::qHash(myint2);
    }

    return value;
}

namespace Digikam
{

/**
 * Stores an album along with the sidebar view, where the album
 * is selected
 */
class Q_DECL_HIDDEN HistoryItem
{
public:

    HistoryItem() = default;

    HistoryItem(const QList<Album*>& a, QWidget* const w)
        : widget(w)
    {
        albums.append(a);
    }

    HistoryItem(const QList<Album*>& a, QWidget* const w,
                const QHash<LabelsTreeView::Labels,
                QList<int> >& selectedLabels)
        : widget(w),
          labels(selectedLabels)
    {
        albums.append(a);
    }

    bool operator==(const HistoryItem& item) const
    {
        if (widget != item.widget)
        {
            return false;
        }

        return (albums == item.albums);
    }

public:

    QList<Album*>                              albums;
    QWidget*                                   widget  = nullptr;
    QHash<LabelsTreeView::Labels, QList<int> > labels;
};

// ---------------------------------------------------------------------

class Q_DECL_HIDDEN HistoryPosition
{
public:

    HistoryPosition() = default;

    HistoryPosition(const ItemInfo& c, const QList<ItemInfo>& s)
        : current(c),
          select (s)
    {
    }

    bool operator==(const HistoryPosition& item)
    {
        return ((current == item.current) && (select == item.select));
    }

public:

    ItemInfo        current;
    QList<ItemInfo> select;
};

// ---------------------------------------------------------------------

class Q_DECL_HIDDEN AlbumHistory::Private
{
public:

    Private() = default;

    void forward(unsigned int steps = 1);

public:

    bool                                       moving           = false;
    bool                                       blockSelection   = false;

    QMutex                                     mutex;

    QList<HistoryItem>                         backwardStack;
    QList<HistoryItem>                         forwardStack;
    QHash<QList<Album*>, HistoryPosition>      historyPos;
    QHash<LabelsTreeView::Labels, QList<int> > neededLabels;
};

void AlbumHistory::Private::forward(unsigned int steps)
{
    if (forwardStack.isEmpty() || ((int)steps > forwardStack.count()))
    {
        return;
    }

    while (steps)
    {
        backwardStack << forwardStack.takeFirst();
        --steps;
    }

    moving = true;
}

AlbumHistory::AlbumHistory(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
}

AlbumHistory::~AlbumHistory()
{
    clearHistory();
    delete d;
}

void AlbumHistory::clearHistory()
{
    QMutexLocker locker(&d->mutex);

    d->backwardStack.clear();
    d->forwardStack.clear();
    d->historyPos.clear();

    d->moving = false;
}

void AlbumHistory::addAlbums(const QList<Album*>& albums, QWidget* const widget)
{
    QMutexLocker locker(&d->mutex);

    if (albums.isEmpty() || !widget || d->moving)
    {
        d->moving = false;

        return;
    }

    // Same album as before in the history

    if (!d->backwardStack.isEmpty() && (d->backwardStack.last().albums == albums))
    {
        d->backwardStack.last().widget = widget;

        return;
    }

    d->backwardStack << HistoryItem(albums, widget);

    // The forward stack has to be cleared, if backward stack was changed

    d->forwardStack.clear();
}

/**
 * @brief AlbumHistory::addAlbums
 *        A special overloaded function for handling AlbumHistory
 *        for the Labels tree-view
 *
 * @author Mohamed_Anwer
 */
void AlbumHistory::addAlbums(const QList<Album*>& albums,
                             QWidget* const widget,
                             const QHash<LabelsTreeView::Labels, QList<int> >& selectedLabels)
{
    QMutexLocker locker(&d->mutex);

    if (albums.isEmpty() || !widget || d->moving)
    {
        d->moving = false;
        return;
    }

    if (!d->backwardStack.isEmpty() && d->backwardStack.last().albums.first()->isUsedByLabelsTree())
    {
        d->backwardStack.last().widget = widget;
        d->backwardStack.last().labels = selectedLabels;
        return;
    }

    d->backwardStack << HistoryItem(albums, widget, selectedLabels);

    // The forward stack has to be cleared, if backward stack was changed

    d->forwardStack.clear();
}

void AlbumHistory::deleteAlbum(Album* const album)
{
    QMutexLocker locker(&d->mutex);

    if (!album || d->backwardStack.isEmpty())
    {
        return;
    }

    QList<Album*> albums;
    albums << album;

    //  Search all HistoryItems, with album and delete them

    QList<HistoryItem>::iterator it = d->backwardStack.begin();

    while (it != d->backwardStack.end())
    {
        if (it->albums == albums)
        {
            it = d->backwardStack.erase(it);
        }
        else
        {
            ++it;
        }
    }

    it = d->forwardStack.begin();

    while (it != d->forwardStack.end())
    {
        if (it->albums == albums)
        {
            it = d->forwardStack.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (d->backwardStack.isEmpty() && d->forwardStack.isEmpty())
    {
        return;
    }

    // If backwardStack is empty, then there is no current album.
    // So make the first album of the forwardStack the current one.

    if (d->backwardStack.isEmpty())
    {
        d->forward();
    }

    // After the album is deleted from the history it has to be ensured,
    // that neighboring albums are different

    QList<HistoryItem>::iterator lhs = d->backwardStack.begin();
    QList<HistoryItem>::iterator rhs = lhs;
    ++rhs;

    while (rhs != d->backwardStack.end())
    {
        if (*lhs == *rhs)
        {
            rhs = d->backwardStack.erase(rhs);
        }
        else
        {
            ++lhs;
            rhs = lhs;
            ++rhs;
        }
    }

    rhs = d->forwardStack.begin();

    while (rhs != d->forwardStack.end())
    {
        if (*lhs == *rhs)
        {
            rhs = d->forwardStack.erase(rhs);
        }
        else
        {
            if (lhs == (d->backwardStack.isEmpty() ?   d->backwardStack.end()
                                                   : --d->backwardStack.end()))
            {
                lhs = d->forwardStack.begin();
            }
            else
            {
                ++lhs;
                rhs = lhs;
            }

            ++rhs;
        }
    }

    if (d->backwardStack.isEmpty() && !d->forwardStack.isEmpty())
    {
        d->forward();
    }
}

void AlbumHistory::getBackwardHistory(QStringList& list) const
{
    QMutexLocker locker(&d->mutex);

    if (d->backwardStack.isEmpty())
    {
        return;
    }

    QList<HistoryItem>::const_iterator it = d->backwardStack.constBegin();

    for ( ; it != (d->backwardStack.isEmpty() ?   d->backwardStack.constEnd()
                                              : --d->backwardStack.constEnd())
          ; ++it)
    {
        if (!(it->albums.isEmpty()))
        {
            QString name;

            for (int iter = 0 ; iter < it->albums.size() ; ++iter)
            {
                name.append(it->albums.at(iter)->title());

                if (iter+1 < it->albums.size())
                {
                    name.append(QLatin1Char('/'));
                }
            }

            list.prepend(name);
        }
    }
}

void AlbumHistory::getForwardHistory(QStringList& list) const
{
    QMutexLocker locker(&d->mutex);

    if (d->forwardStack.isEmpty())
    {
        return;
    }

    QList<HistoryItem>::const_iterator it;

    for (it = d->forwardStack.constBegin() ; it != d->forwardStack.constEnd() ; ++it)
    {
        if (!(it->albums.isEmpty()))
        {
            QString name;

            for (int iter = 0 ; iter < it->albums.size() ; ++iter)
            {
                name.append(it->albums.at(iter)->title());

                if (iter+1 < it->albums.size())
                {
                    name.append(QLatin1Char('/'));
                }
            }

            list.append(name);
        }
    }
}

void AlbumHistory::back(QList<Album*>& album, QWidget** const widget, unsigned int steps)
{
    QMutexLocker locker(&d->mutex);

    *widget = nullptr;

    if ((d->backwardStack.count() <= 1) || ((int)steps > d->backwardStack.count()))
    {
        return;    // Only the current album available
    }

    while (steps)
    {
        d->forwardStack.prepend(d->backwardStack.takeLast());
        --steps;
    }

    d->moving = true;

    if (d->backwardStack.isEmpty())
    {
        return;
    }

    album.append(d->backwardStack.last().albums);
    *widget         = d->backwardStack.last().widget;
    d->neededLabels = d->backwardStack.last().labels;
}

void AlbumHistory::forward(QList<Album*>& album, QWidget** const widget, unsigned int steps)
{
    QMutexLocker locker(&d->mutex);

    *widget = nullptr;

    if (d->forwardStack.isEmpty() || ((int)steps > d->forwardStack.count()))
    {
        return;
    }

    d->forward(steps);

    if (d->backwardStack.isEmpty())
    {
        return;
    }

    album.append(d->backwardStack.last().albums);
    *widget         = d->backwardStack.last().widget;
    d->neededLabels = d->backwardStack.last().labels;
}

void AlbumHistory::getCurrentAlbum(Album** const album, QWidget** const widget) const
{
    QMutexLocker locker(&d->mutex);

    *album  = nullptr;
    *widget = nullptr;

    if (d->backwardStack.isEmpty())
    {
        return;
    }

    if (!(d->backwardStack.last().albums.isEmpty()))
    {
        *album  = d->backwardStack.last().albums.first();
    }

    *widget = d->backwardStack.last().widget;
}

bool AlbumHistory::isForwardEmpty() const
{
    QMutexLocker locker(&d->mutex);

    return d->forwardStack.isEmpty();
}

bool AlbumHistory::isBackwardEmpty() const
{
    QMutexLocker locker(&d->mutex);

    // the last album of the backwardStack is the currently shown
    // album, and therefore not really a previous album

    return ((d->backwardStack.count() <= 1) ? true : false);
}

QHash<LabelsTreeView::Labels, QList<int> > AlbumHistory::neededLabels()
{
    return d->neededLabels;
}

void AlbumHistory::slotAlbumSelected()
{
    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();

    if (d->historyPos.contains(albumList))
    {
        d->blockSelection = true;

        Q_EMIT signalSetCurrent(d->historyPos[albumList].current.id());
    }
}

void AlbumHistory::slotAlbumCurrentChanged()
{
    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();

    if (!(albumList.isEmpty()) && d->historyPos.contains(albumList))
    {
        if (d->historyPos[albumList].select.size())
        {
            Q_EMIT signalSetSelectedInfos(d->historyPos[albumList].select);
        }
    }

    d->blockSelection = false;
}

void AlbumHistory::slotCurrentChange(const ItemInfo& info)
{
    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();

    if (albumList.isEmpty())
    {
        return;
    }

    d->historyPos[albumList].current = info;
}

void AlbumHistory::slotImageSelected(const ItemInfoList& selectedImages)
{
    if (d->blockSelection)
    {
        return;
    }

    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();

    if (d->historyPos.contains(albumList))
    {
        d->historyPos[albumList].select = selectedImages;
    }
}

void AlbumHistory::slotClearSelectPAlbum(const ItemInfo& imageInfo)
{
    Album* const album = dynamic_cast<Album*>(AlbumManager::instance()->findPAlbum(imageInfo.albumId()));   // cppcheck-suppress constVariablePointer
    QList<Album*> albums;
    albums << album;

    if (d->historyPos.contains(albums))
    {
        d->historyPos[albums].select.clear();
    }
}

void AlbumHistory::slotClearSelectTAlbum(int id)
{
    Album* const album = dynamic_cast<Album*>(AlbumManager::instance()->findTAlbum(id));    // cppcheck-suppress constVariablePointer
    QList<Album*> albums;
    albums << album;

    if (d->historyPos.contains(albums))
    {
        d->historyPos[albums].select.clear();
    }
}

void AlbumHistory::slotAlbumDeleted(Album* album)
{
    deleteAlbum(album);
    QList<Album*> albums;
    albums << album;

    if (d->historyPos.contains(albums))
    {
        d->historyPos.remove(albums);
    }
}

void AlbumHistory::slotAlbumsCleared()
{
    clearHistory();
}

} // namespace Digikam

#include "moc_albumhistory.cpp"
