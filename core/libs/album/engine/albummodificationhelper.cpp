/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : helper class used to modify physical albums in views
 *
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albummodificationhelper.h"

// Qt includes

#include <QApplication>
#include <QDirIterator>
#include <QInputDialog>
#include <QMessageBox>
#include <QAction>
#include <QUrl>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "albumpropsedit.h"
#include "albumpointer.h"
#include "applicationsettings.h"
#include "collectionmanager.h"
#include "deletedialog.h"
#include "dio.h"
#include "itemiconview.h"
#include "digikamapp.h"
#include "coredb.h"
#include "coredbaccess.h"

namespace Digikam
{

class Q_DECL_HIDDEN AlbumModificationHelper::Private
{
public:

    Private() = default;

public:

    QWidget* dialogParent = nullptr;
};

AlbumModificationHelper::AlbumModificationHelper(QObject* const parent, QWidget* const dialogParent)
    : QObject(parent),
      d      (new Private)
{
    d->dialogParent = dialogParent;
}

AlbumModificationHelper::~AlbumModificationHelper()
{
    delete d;
}

void AlbumModificationHelper::bindAlbum(QAction* const action, PAlbum* const album) const
{
    action->setData(QVariant::fromValue(AlbumPointer<PAlbum>(album)));
}

PAlbum* AlbumModificationHelper::boundAlbum(QObject* const sender) const
{
    QAction* action = nullptr;

    if ((action = qobject_cast<QAction*>(sender)))
    {
        return action->data().value<AlbumPointer<PAlbum> >();
    }

    return nullptr;
}

PAlbum* AlbumModificationHelper::slotAlbumNew()
{
    return slotAlbumNew(boundAlbum(sender()));
}

PAlbum* AlbumModificationHelper::slotAlbumNew(PAlbum* parent)
{
    if (!parent)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "No parent album given";
        return nullptr;
    }

    const ApplicationSettings* settings = ApplicationSettings::instance();

    if (!settings)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "could not get Album Settings";
        return nullptr;
    }

/*
    QDir libraryDir(settings->getAlbumLibraryPath());

    if (!libraryDir.exists())
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("The album library has not been set correctly.\n"
                                   "Select \"Configure Digikam\" from the Settings "
                                   "menu and choose a folder to use for the album "
                                   "library."));
        return;
    }
*/

    // if we create an album under root, need to supply the album root path.

    QString albumRootPath;

    albumRootPath = CollectionManager::instance()->albumRootPath(parent->albumRootId());

    QString     title;
    QString     comments;
    QString     category;
    QDate       date;
    QStringList albumCategories;
    int         parentSelector;

    if (!AlbumPropsEdit::createNew(parent, title, comments, date, category,
                                   albumCategories, parentSelector))
    {
        return nullptr;
    }

    QStringList oldAlbumCategories(ApplicationSettings::instance()->getAlbumCategoryNames());

    if (albumCategories != oldAlbumCategories)
    {
        ApplicationSettings::instance()->setAlbumCategoryNames(albumCategories);
    }

    QString errMsg;
    PAlbum* album = nullptr;

    if (parent->isRoot() || (parentSelector == 1))
    {
        album = AlbumManager::instance()->createPAlbum(albumRootPath, title, comments,
                                                       date, category, errMsg);
    }
    else
    {
        album = AlbumManager::instance()->createPAlbum(parent, title, comments,
                                                       date, category, errMsg);
    }

    if (!album)
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(), errMsg);

        return nullptr;
    }

    return album;
}

void AlbumModificationHelper::slotAlbumDelete()
{
    slotAlbumDelete(boundAlbum(sender()));
}

void AlbumModificationHelper::slotAlbumDelete(PAlbum* album)
{
    if (!album || album->isRoot() || album->isAlbumRoot())
    {
        return;
    }

    // find subalbums

    QList<QUrl> childrenList;
    addAlbumChildrenToList(childrenList, album);

    DeleteDialog dialog(d->dialogParent);

    // All subalbums will be presented in the list as well

    if (!dialog.confirmDeleteList(childrenList,
                                  (childrenList.size() == 1) ? DeleteDialogMode::Albums
                                                             : DeleteDialogMode::Subalbums,
                                  DeleteDialogMode::UserPreference))
    {
        return;
    }

    bool useTrash = !dialog.shouldDelete();
    QFileInfo fileInfo(album->folderPath());

    // If the trash is used no check is necessary, as the trash lists all files
    // and only perform this check if the album is a directory

    if (!useTrash && fileInfo.isDir())
    {
        QStringList imageTypes, audioTypes, videoTypes, allTypes, foundTypes;
        QDirIterator it(fileInfo.path(), QDir::Files, QDirIterator::Subdirectories);

        CoreDbAccess().db()->getFilterSettings(&imageTypes, &videoTypes, &audioTypes);
        allTypes << imageTypes << audioTypes << videoTypes;
        bool notEmpty = false;

        while (it.hasNext())
        {
            it.next();
            QString ext = it.fileInfo().suffix().toLower();

            if      (!allTypes.contains(ext) && !foundTypes.contains(ext))
            {
                foundTypes << ext;
            }
            else if (allTypes.contains(ext) && !notEmpty)
            {
                notEmpty = true;
            }
        }

        if      (!foundTypes.isEmpty())
        {
            foundTypes.sort();

            QString found = foundTypes.join(QLatin1String(", "));

            int result    = QMessageBox::warning(qApp->activeWindow(), qApp->applicationName(),
                                                 i18n("<p>The album you want to delete contains files "
                                                      "(%1) which are not displayed in digiKam.</p>"
                                                      "<p>Do you want to continue?</p>", found),
                                                 QMessageBox::Yes | QMessageBox::No);

            if (result != QMessageBox::Yes)
            {
                return;
            }
        }
        else if (notEmpty)
        {
            int result    = QMessageBox::warning(qApp->activeWindow(), qApp->applicationName(),
                                                 i18n("<p>The album you want to delete is not empty.</p>"
                                                      "<p>Do you want to continue?</p>"),
                                                 QMessageBox::Yes | QMessageBox::No);

            if (result != QMessageBox::Yes)
            {
                return;
            }
        }
    }

    DIO::del(album, useTrash);
}

void AlbumModificationHelper::slotAlbumRename()
{
    slotAlbumRename(boundAlbum(sender()));
}

void AlbumModificationHelper::slotAlbumRename(PAlbum* album)
{
    if (!album)
    {
        return;
    }

    QString oldTitle(album->title());

    QPointer<QInputDialog> textDlg = new QInputDialog(d->dialogParent);

    if (!album->isAlbumRoot())
    {
        textDlg->setWindowTitle(i18nc("@title:window", "Rename Album (%1)", oldTitle));
    }
    else
    {
        textDlg->setWindowTitle(i18nc("@title:window", "Rename Album Root (%1)", oldTitle));
    }

    textDlg->setLabelText(i18n("Enter new album name:"));
    textDlg->resize(450, textDlg->sizeHint().height());
    textDlg->setInputMode(QInputDialog::TextInput);
    textDlg->setTextEchoMode(QLineEdit::Normal);
    textDlg->setTextValue(oldTitle);

    if (textDlg->exec() != QDialog::Accepted)
    {
        delete textDlg;
        return;
    }

    QString title = textDlg->textValue().trimmed();
    delete textDlg;

    if (title != oldTitle)
    {
        if (album->isAlbumRoot())
        {
            CollectionLocation location = CollectionManager::instance()->
                                          locationForAlbumRootId(album->albumRootId());

            if (!location.isNull())
            {
                CollectionManager::instance()->setLabel(location, title);
            }
        }
        else
        {
            QString errMsg;

            if (!AlbumManager::instance()->renamePAlbum(album, title, errMsg))
            {
                QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(), errMsg);
            }
        }

        // Resorting the tree View after changing metadata

        DigikamApp::instance()->view()->slotSortAlbums(ApplicationSettings::instance()->getAlbumSortRole());
    }
}

void AlbumModificationHelper::addAlbumChildrenToList(QList<QUrl>& list, Album* const album)
{
    // simple recursive helper function

    if (album)
    {
        if (!list.contains(album->databaseUrl()))
        {
            list.append(album->databaseUrl());
        }

        AlbumIterator it(album);

        while (it.current())
        {
            addAlbumChildrenToList(list, *it);
            ++it;
        }
    }
}

void AlbumModificationHelper::slotAlbumEdit()
{
    slotAlbumEdit(boundAlbum(sender()));
}

void AlbumModificationHelper::slotAlbumEdit(PAlbum* album)
{
    if (!album || album->isRoot() || album->isAlbumRoot())
    {
        return;
    }

    QString     oldTitle(album->title());
    QString     oldComments(album->caption());
    QString     oldCategory(album->category());
    QDate       oldDate(album->date());
    QString     title, comments, category;
    QDate       date;
    QStringList albumCategories;

    if (AlbumPropsEdit::editProps(album, title, comments, date,
                                  category, albumCategories))
    {
        if (comments != oldComments)
        {
            album->setCaption(comments);
        }

        if ((date != oldDate) && date.isValid())
        {
            album->setDate(date);
        }

        if (category != oldCategory)
        {
            album->setCategory(category);
        }

        ApplicationSettings::instance()->setAlbumCategoryNames(albumCategories);

        // Do this last : so that if anything else changed we can
        // successfully save to the db with the old name

        if (title != oldTitle)
        {
            QString errMsg;

            if (!AlbumManager::instance()->renamePAlbum(album, title, errMsg))
            {
                QMessageBox::critical(d->dialogParent, qApp->applicationName(), errMsg);
            }
        }

        // Resorting the tree View after changing metadata

        DigikamApp::instance()->view()->slotSortAlbums(ApplicationSettings::instance()->getAlbumSortRole());
    }
}

void AlbumModificationHelper::slotAlbumResetIcon(PAlbum* album)
{
    if (!album)
    {
        return;
    }

    QString err;
    AlbumManager::instance()->updatePAlbumIcon(album, 0, err);
}

void AlbumModificationHelper::slotAlbumResetIcon()
{
    slotAlbumResetIcon(boundAlbum(sender()));
}

} // namespace Digikam

#include "moc_albummodificationhelper.cpp"
