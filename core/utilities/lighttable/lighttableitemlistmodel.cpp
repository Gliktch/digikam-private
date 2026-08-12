/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-11
 * Description : model for the light table thumbs bar.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lighttableitemlistmodel.h"

// Qt includes

#include <QList>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

template <typename T, class Container>
void removeAnyInInterval(Container& list, const T& begin, const T& end)
{
    typename Container::iterator it;

    for (it = list.begin() ; it != list.end() ; )
    {
        if (((*it) >= begin) && ((*it) <= end))
        {
            it = list.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

class Q_DECL_HIDDEN LightTableItemListModel::Private
{
public:

    Private() = default;

public:

    QSet<int> leftIndexes;
    QSet<int> rightIndexes;
    bool      exclusive    = false;
};

LightTableItemListModel::LightTableItemListModel(QWidget* const parent)
    : ItemListModel(parent),
      d            (new Private)
{
}

LightTableItemListModel::~LightTableItemListModel()
{
    delete d;
}

void LightTableItemListModel::clearLightTableState()
{
    d->leftIndexes.clear();
    d->rightIndexes.clear();
}

void LightTableItemListModel::setExclusiveLightTableState(bool exclusive)
{
    d->exclusive = exclusive;
}

QVariant LightTableItemListModel::data(const QModelIndex& index, int role) const
{
    if      (role == LTLeftPanelRole)
    {
        return d->leftIndexes.contains(index.row());
    }
    else if (role == LTRightPanelRole)
    {
        return d->rightIndexes.contains(index.row());
    }

    return ItemListModel::data(index, role);
}

bool LightTableItemListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
    {
        return false;
    }

    if      (role == LTLeftPanelRole)
    {
        if (d->exclusive)
        {
            d->leftIndexes.clear();
        }

        d->leftIndexes << index.row();

        return true;
    }
    else if (role == LTRightPanelRole)
    {
        if (d->exclusive)
        {
            d->rightIndexes.clear();
        }

        d->rightIndexes << index.row();

        return true;
    }

    return ItemListModel::setData(index, value, role);
}

void LightTableItemListModel::prepareImageInfosAboutToBeRemoved(int begin, int end)
{
    removeAnyInInterval(d->leftIndexes,  begin, end);
    removeAnyInInterval(d->rightIndexes, begin, end);
}

void LightTableItemListModel::imageInfosCleared()
{
    clearLightTableState();
}

} // namespace Digikam

#include "moc_lighttableitemlistmodel.cpp"
