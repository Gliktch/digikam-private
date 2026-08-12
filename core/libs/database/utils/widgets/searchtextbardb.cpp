/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-25
 * Description : a bar used to search a string - version based on database models
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "searchtextbardb.h"

// Qt includes

#include <QContextMenuEvent>
#include <QMenu>
#include <QColor>
#include <QPalette>
#include <QString>
#include <QMap>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "albumfiltermodel.h"

namespace Digikam
{

class Q_DECL_HIDDEN SearchTextBarDb::Private
{
public:

    Private() = default;

public:

    QPointer<AlbumFilterModel> filterModel = nullptr;
};

SearchTextBarDb::SearchTextBarDb(QWidget* const parent, const QString& name, const QString& msg)
    : SearchTextBar(parent, name, msg),
      d            (new Private)
{
}

SearchTextBarDb::~SearchTextBarDb()
{
    delete d;
}

void SearchTextBarDb::setModel(QAbstractItemModel* model, int uniqueIdRole, int displayRole)
{
    completerModel()->setItemModel(model, uniqueIdRole, displayRole);
}

void SearchTextBarDb::setModel(AbstractAlbumModel* const model)
{
    completerModel()->setItemModel(model,
                                   AbstractAlbumModel::AlbumIdRole,
                                   AbstractAlbumModel::AlbumTitleRole);
}

void SearchTextBarDb::setFilterModel(AlbumFilterModel* const filterModel)
{
    // If there already was a model, disconnect from this model

    if (d->filterModel)
    {
        disconnect(d->filterModel);
    }

    d->filterModel = filterModel;

    // Connect to new model if desired

    if (d->filterModel)
    {
        connect(this, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
                d->filterModel, SLOT(setSearchTextSettings(SearchTextSettings)));

        connect(d->filterModel, SIGNAL(hasSearchResult(bool)),
                this, SLOT(slotSearchResult(bool)));
    }
}

} // namespace Digikam

#include "moc_searchtextbardb.cpp"
