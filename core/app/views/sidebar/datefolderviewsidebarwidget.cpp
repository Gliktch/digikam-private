/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : Side Bar Widget for the date folder view.
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "datefolderviewsidebarwidget.h"

// Qt includes

#include <QApplication>
#include <QStyle>
#include <QIcon>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "albumpointer.h"
#include "applicationsettings.h"
#include "datefolderview.h"

namespace Digikam
{

class Q_DECL_HIDDEN DateFolderViewSideBarWidget::Private
{
public:

    Private() = default;

    DateFolderView* dateFolderView = nullptr;
};

DateFolderViewSideBarWidget::DateFolderViewSideBarWidget(QWidget* const parent,
                                                         DateAlbumModel* const model,
                                                         ItemAlbumFilterModel* const imageFilterModel)
    : SidebarWidget(parent),
      d            (new Private)
{
    setObjectName(QLatin1String("DateFolderView Sidebar"));
    setProperty("Shortcut", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F4));

    const int spacing         = layoutSpacing();

    QVBoxLayout* const layout = new QVBoxLayout(this);

    d->dateFolderView         = new DateFolderView(this, model);
    d->dateFolderView->setConfigGroup(getConfigGroup());
    d->dateFolderView->setItemModel(imageFilterModel);

    layout->addWidget(d->dateFolderView);
    layout->setContentsMargins(0, 0, spacing, 0);
}

DateFolderViewSideBarWidget::~DateFolderViewSideBarWidget()
{
    delete d;
}

void DateFolderViewSideBarWidget::setActive(bool active)
{
    d->dateFolderView->setActive(active);
}

void DateFolderViewSideBarWidget::doLoadState()
{
    d->dateFolderView->loadState();
}

void DateFolderViewSideBarWidget::doSaveState()
{
    d->dateFolderView->saveState();
}

void DateFolderViewSideBarWidget::applySettings()
{
}

void DateFolderViewSideBarWidget::changeAlbumFromHistory(const QList<Album*>& album)
{
    d->dateFolderView->changeAlbumFromHistory(dynamic_cast<DAlbum*>(album.first()));
}

AlbumPointer<DAlbum> DateFolderViewSideBarWidget::currentAlbum() const
{
    return d->dateFolderView->currentAlbum();
}

void DateFolderViewSideBarWidget::gotoDate(const QDate& date)
{
    d->dateFolderView->gotoDate(date);
}

const QIcon DateFolderViewSideBarWidget::getIcon()
{
    return QIcon::fromTheme(QLatin1String("view-calendar-list"));
}

const QString DateFolderViewSideBarWidget::getCaption()
{
    return i18n("Dates");
}

} // namespace Digikam

#include "moc_datefolderviewsidebarwidget.cpp"
