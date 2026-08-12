/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2014      by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "abstractcheckablealbumtreeview.h"

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "abstractalbumtreeview_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN AbstractCheckableAlbumTreeView::Private
{
public:

    Private() = default;

public:

    const QString configCheckedAlbumsEntry          = QLatin1String("Checked");
    const QString configPartiallyCheckedAlbumsEntry = QLatin1String("PartiallyChecked");
    const QString configRestoreCheckedEntry         = QLatin1String("RestoreChecked");

    QList<int>           checkedAlbumIds;
    QList<int>           partiallyCheckedAlbumIds;
};

// --------------------------------------------------------

AbstractCheckableAlbumTreeView::AbstractCheckableAlbumTreeView(QWidget* const parent, Flags flags)
    : AbstractCountingAlbumTreeView(parent, flags & ~CreateDefaultFilterModel),
      d                            (new Private)
{
    m_checkOnMiddleClick = true;
    m_restoreCheckState  = false;

    if (flags & CreateDefaultFilterModel)
    {
        setAlbumFilterModel(new CheckableAlbumFilterModel(this));
    }
}

AbstractCheckableAlbumTreeView::~AbstractCheckableAlbumTreeView()
{
    delete d;
}

AbstractCheckableAlbumModel* AbstractCheckableAlbumTreeView::checkableAlbumModel() const
{
    return dynamic_cast<AbstractCheckableAlbumModel*>(m_albumModel);
}

CheckableAlbumFilterModel* AbstractCheckableAlbumTreeView::checkableAlbumFilterModel() const
{
    return dynamic_cast<CheckableAlbumFilterModel*> (m_albumFilterModel);
}

void AbstractCheckableAlbumTreeView::setAlbumModel(AbstractCheckableAlbumModel* const model)
{
    AbstractCountingAlbumTreeView::setAlbumModel(model);
}

void AbstractCheckableAlbumTreeView::setCheckableAlbumFilterModel(CheckableAlbumFilterModel* const filterModel)
{
    AbstractCountingAlbumTreeView::setAlbumFilterModel(filterModel);
}

void AbstractCheckableAlbumTreeView::setCheckOnMiddleClick(bool doThat)
{
    m_checkOnMiddleClick = doThat;
}

void AbstractCheckableAlbumTreeView::middleButtonPressed(Album* a)
{
    AbstractCheckableAlbumModel* const model = static_cast<AbstractCheckableAlbumModel*>(m_albumModel);

    if (!model)
    {
        return;
    }

    if (model->isCheckable())
    {
        if (model->isTristate())
        {
            switch (model->checkState(a))
            {
                case Qt::Unchecked:
                {
                    model->setCheckState(a, Qt::PartiallyChecked);
                    break;
                }

                case Qt::PartiallyChecked:
                {
                    model->setCheckState(a, Qt::Checked);
                    break;
                }

                case Qt::Checked:
                {
                    model->setCheckState(a, Qt::Unchecked);
                    break;
                }
            }
        }
        else
        {
            model->toggleChecked(a);
        }
    }
}

bool AbstractCheckableAlbumTreeView::isRestoreCheckState() const
{
    return m_restoreCheckState;
}

void AbstractCheckableAlbumTreeView::setRestoreCheckState(bool restore)
{
    m_restoreCheckState = restore;
}

void AbstractCheckableAlbumTreeView::doLoadState()
{
    AbstractCountingAlbumTreeView::doLoadState();

    KConfigGroup group = getConfigGroup();

    if (!m_restoreCheckState)
    {
        m_restoreCheckState = group.readEntry(entryName(d->configRestoreCheckedEntry), false);
    }

    if (!m_restoreCheckState || !checkableAlbumModel()->isCheckable())
    {
        return;
    }

    const QStringList checkedAlbums = group.readEntry(entryName(d->configCheckedAlbumsEntry), QStringList());

    d->checkedAlbumIds.clear();

    for (const QString& albumId : std::as_const(checkedAlbums))
    {
        bool ok;
        const int id = albumId.toInt(&ok);

        if (ok)
        {
            d->checkedAlbumIds << id;
        }
    }

    const QStringList partiallyCheckedAlbums = group.readEntry(entryName(d->configPartiallyCheckedAlbumsEntry), QStringList());
    d->partiallyCheckedAlbumIds.clear();

    for (const QString& albumId : std::as_const(partiallyCheckedAlbums))
    {
        bool ok;
        const int id = albumId.toInt(&ok);

        if (ok)
        {
            d->partiallyCheckedAlbumIds << id;
        }
    }

    // initially sync with the albums that are already in the model

    restoreCheckStateForHierarchy(QModelIndex());
    horizontalScrollBar()->setValue(0);
}

void AbstractCheckableAlbumTreeView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    AbstractCountingAlbumTreeView::rowsInserted(parent, start, end);

    if (!d->checkedAlbumIds.isEmpty())
    {
        for (int i = start ; i <= end ; ++i)
        {
            const QModelIndex child = checkableAlbumModel()->index(i, 0, parent);
            restoreCheckState(child);
        }
    }
}

void AbstractCheckableAlbumTreeView::restoreCheckStateForHierarchy(const QModelIndex& index)
{
    // recurse children

    for (int i = 0 ; i < checkableAlbumModel()->rowCount(index) ; ++i)
    {
        const QModelIndex child = checkableAlbumModel()->index(i, 0, index);
        restoreCheckState(child);
        restoreCheckStateForHierarchy(child);
    }
}

void AbstractCheckableAlbumTreeView::restoreCheckState(const QModelIndex& index)
{
    Album* const album = checkableAlbumModel()->albumForIndex(index);

    if (!album || !(album->id()))
    {
        return;
    }

    if (d->checkedAlbumIds.contains(album->id()))
    {
        checkableAlbumModel()->setCheckState(album, Qt::Checked);
        d->checkedAlbumIds.removeOne(album->id());
    }

    if (d->partiallyCheckedAlbumIds.contains(album->id()))
    {
        checkableAlbumModel()->setCheckState(album, Qt::PartiallyChecked);
        d->partiallyCheckedAlbumIds.removeOne(album->id());
    }
}

void AbstractCheckableAlbumTreeView::doSaveState()
{
    AbstractCountingAlbumTreeView::doSaveState();

    KConfigGroup group = getConfigGroup();

    group.writeEntry(entryName(d->configRestoreCheckedEntry), m_restoreCheckState);

    if (!m_restoreCheckState || !checkableAlbumModel()->isCheckable())
    {
        return;
    }

    const QList<Album*> checkedAlbums = checkableAlbumModel()->checkedAlbums();
    QStringList checkedIds;

    for (const Album* const album : std::as_const(checkedAlbums))
    {
        checkedIds << QString::number(album->id());
    }

    group.writeEntry(entryName(d->configCheckedAlbumsEntry), checkedIds);

    if (!checkableAlbumModel()->isTristate())
    {
        return;
    }

    const QList<Album*> partiallyCheckedAlbums = checkableAlbumModel()->partiallyCheckedAlbums();
    QStringList partiallyCheckedIds;

    for (const Album* const album : std::as_const(partiallyCheckedAlbums))
    {
        partiallyCheckedIds << QString::number(album->id());
    }

    group.writeEntry(entryName(d->configPartiallyCheckedAlbumsEntry), partiallyCheckedIds);
}

} // namespace Digikam

#include "moc_abstractcheckablealbumtreeview.cpp"
