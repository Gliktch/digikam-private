/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : factory of basic models used for views in digikam
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmodelfactory.h"

// Qt includes

#include <QIcon>

// Local settings

#include "applicationsettings.h"
#include "thememanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN DModelFactory::Private
{

public:

    Private() = default;

    TagModel*          tagModel          = nullptr;
    TagModel*          tagFaceModel      = nullptr;
    TagModel*          tagFilterModel    = nullptr;
    AlbumModel*        albumModel        = nullptr;
    SearchModel*       searchModel       = nullptr;
    DateAlbumModel*    dateAlbumModel    = nullptr;
    ItemVersionsModel* imageVersionModel = nullptr;
};

DModelFactory::DModelFactory()
    : d(new Private)
{
    d->tagModel          = new TagModel(AbstractAlbumModel::IncludeRootAlbum);
    d->tagFaceModel      = new TagModel(AbstractAlbumModel::IgnoreRootAlbum);
    d->tagFaceModel->activateFaceTagModel();
    d->tagFilterModel    = new TagModel(AbstractAlbumModel::IgnoreRootAlbum);
    d->tagFilterModel->setAddExcludeTristate(true);

    d->albumModel        = new AlbumModel(AlbumModel::IncludeRootAlbum);
    d->searchModel       = new SearchModel();
    d->dateAlbumModel    = new DateAlbumModel();
    d->imageVersionModel = new ItemVersionsModel();

    // set icons initially

    slotApplicationSettingsChanged();

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotApplicationSettingsChanged()));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotApplicationSettingsChanged()));
}

DModelFactory::~DModelFactory()
{
    delete d->tagModel;
    delete d->tagFaceModel;
    delete d->tagFilterModel;

    delete d->albumModel;
    delete d->searchModel;
    delete d->dateAlbumModel;
    delete d->imageVersionModel;

    delete d;
}

TagModel* DModelFactory::getTagModel() const
{
    return d->tagModel;
}

TagModel* DModelFactory::getTagFaceModel() const
{
    return d->tagFaceModel;
}

TagModel* DModelFactory::getTagFilterModel() const
{
    return d->tagFilterModel;
}

AlbumModel* DModelFactory::getAlbumModel() const
{
    return d->albumModel;
}

SearchModel* DModelFactory::getSearchModel() const
{
    return d->searchModel;
}

DateAlbumModel* DModelFactory::getDateAlbumModel() const
{
    return d->dateAlbumModel;
}

ItemVersionsModel* DModelFactory::getItemVersionsModel() const
{
    return d->imageVersionModel;
}

void DModelFactory::slotApplicationSettingsChanged()
{
    int size = ApplicationSettings::instance()->getTreeViewIconSize();
    d->dateAlbumModel->setPixmaps(QIcon::fromTheme(QLatin1String("view-calendar-list")).pixmap(size),
                                  QIcon::fromTheme(QLatin1String("view-calendar")).pixmap(size));
}

} // namespace Digikam

#include "moc_dmodelfactory.cpp"
