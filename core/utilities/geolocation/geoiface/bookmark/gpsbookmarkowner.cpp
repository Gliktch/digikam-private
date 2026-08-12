/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Central object for managing bookmarks
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gpsbookmarkowner.h"

// Qt includes

#include <QStandardItemModel>
#include <QStandardPaths>
#include <QPointer>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "gpsbookmarkmodelhelper.h"
#include "gpsundocommand.h"
#include "gpsitemmodel.h"
#include "bookmarknode.h"
#include "bookmarksmenu.h"
#include "bookmarksdlg.h"

namespace Digikam
{

class Q_DECL_HIDDEN GPSBookmarkOwner::Private
{
public:

    Private() = default;

    QWidget*                  parent                = nullptr;
    BookmarksManager*         bookmarkManager       = nullptr;
    BookmarksMenu*            bookmarkMenu          = nullptr;
    QPointer<BookmarksDialog> bookmarksDialog;
    bool                      addBookmarkEnabled    = true;
    GPSBookmarkModelHelper*   bookmarkModelHelper   = nullptr;
    GeoCoordinates            lastCoordinates;
    QString                   lastTitle;
};

GPSBookmarkOwner::GPSBookmarkOwner(GPSItemModel* const gpsItemModel, QWidget* const parent)
    : QObject(parent),
      d      (new Private)
{
    d->parent = parent;

    const QString bookmarksFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                                                       QLatin1String("/geobookmarks.xml");
    d->bookmarkManager              = new BookmarksManager(bookmarksFileName, this);
    d->bookmarkManager->load();
    d->bookmarkMenu                 = new BookmarksMenu(d->bookmarkManager, d->parent);
    d->bookmarkModelHelper          = new GPSBookmarkModelHelper(d->bookmarkManager,
                                                                 gpsItemModel, this);

    createBookmarksMenu();
}

GPSBookmarkOwner::~GPSBookmarkOwner()
{
    delete d;
}

QMenu* GPSBookmarkOwner::getMenu() const
{
    return dynamic_cast<QMenu*>(d->bookmarkMenu);
}

QString GPSBookmarkOwner::currentTitle() const
{
    if (d->lastTitle.isEmpty())
    {
        return currentUrl();
    }

    return d->lastTitle;
}

QString GPSBookmarkOwner::currentUrl() const
{
    // TODO : check if this QUrl from QString conversion is fine.

    return d->lastCoordinates.geoUrl();
}

void GPSBookmarkOwner::slotOpenBookmark(const QUrl& url)
{
    const QString gps                         = url.url().toLower();
    bool  okay                                = false;
    const GeoCoordinates coordinate = GeoCoordinates::fromGeoUrl(gps, &okay);

    if (okay)
    {
        GPSDataContainer position;
        position.setCoordinates(coordinate);

        Q_EMIT positionSelected(position);
    }
}

void GPSBookmarkOwner::slotShowBookmarksDialog()
{

#ifdef Q_OS_DARWIN

    // See bug #486341

    QPointer<BookmarksDialog> dlg = new BookmarksDialog(d->parent, d->bookmarkManager);
    dlg->exec();

#else

    if (
        d->bookmarksDialog &&
        (d->bookmarksDialog->isMinimized() || !d->bookmarksDialog->isHidden())
       )
    {
        d->bookmarksDialog->showNormal();       // krazy:exclude=qmethods
        d->bookmarksDialog->activateWindow();
        d->bookmarksDialog->raise();
    }
    else
    {
        d->bookmarksDialog = new BookmarksDialog(qApp->activeWindow(), d->bookmarkManager);
        d->bookmarksDialog->show();
        d->bookmarksDialog->raise();
    }

#endif

}

void GPSBookmarkOwner::slotAddBookmark()
{
    QPointer<AddBookmarkDialog> dlg = new AddBookmarkDialog(currentUrl(), currentTitle(),
                                                            d->parent, d->bookmarkManager);
    dlg->exec();
}

void GPSBookmarkOwner::changeAddBookmark(const bool state)
{
    d->addBookmarkEnabled = state;

    createBookmarksMenu();
}

void GPSBookmarkOwner::createBookmarksMenu()
{
    d->bookmarkMenu->clear();

    QList<QAction*> bookmarksActions;

    QAction* const showAllBookmarksAction = new QAction(i18n("Edit Bookmarks"), d->parent);
    bookmarksActions.append(showAllBookmarksAction);

    connect(showAllBookmarksAction, SIGNAL(triggered()),
            this, SLOT(slotShowBookmarksDialog()));

    QAction* const addBookmark = new QAction(i18n("Add Bookmark..."), d->parent);
    bookmarksActions.append(addBookmark);

    connect(addBookmark, SIGNAL(triggered()),
            this, SLOT(slotAddBookmark()));

    d->bookmarkMenu->setInitialActions(bookmarksActions);

    connect(d->bookmarkMenu, SIGNAL(openUrl(QUrl)),
            this, SLOT(slotOpenBookmark(QUrl)));
}

BookmarksManager* GPSBookmarkOwner::bookmarkManager() const
{
    return d->bookmarkManager;
}

GPSBookmarkModelHelper* GPSBookmarkOwner::bookmarkModelHelper() const
{
    return d->bookmarkModelHelper;
}

void GPSBookmarkOwner::setPositionAndTitle(const GeoCoordinates& coordinates,
                                           const QString& title)
{
    d->lastCoordinates = coordinates;
    d->lastTitle       = title;
}

} // namespace Digikam

#include "moc_gpsbookmarkowner.cpp"
