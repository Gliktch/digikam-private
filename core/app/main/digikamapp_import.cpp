/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation - Import tools
 *
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "digikamapp_p.h"

namespace Digikam
{

void DigikamApp::slotImportedImagefromScanner(const QUrl& url)
{
    ScanController::instance()->scannedInfo(url.toLocalFile());
}

void DigikamApp::updateQuickImportAction()
{
    d->quickImportMenu->clear();

    {
        const auto acs = d->solidCameraActionGroup->actions();

        for (QAction* const action : acs)
        {
            d->quickImportMenu->addAction(action);
        }
    }

    {
        const auto acs = d->solidUsmActionGroup->actions();

        for (QAction* const action : acs)
        {
            d->quickImportMenu->addAction(action);
        }
    }

    {
        const auto acs = d->manualCameraActionGroup->actions();

        for (QAction* const action : acs)
        {
            d->quickImportMenu->addAction(action);
        }
    }

    if (d->quickImportMenu->actions().isEmpty())
    {
        d->quickImportMenu->setEnabled(false);
    }
    else
    {
        disconnect(d->quickImportMenu->menuAction(), SIGNAL(triggered()),
                   nullptr, nullptr);

        QAction*  primaryAction = nullptr;
        QDateTime latest;
        const auto acs          = d->quickImportMenu->actions();

        for (QAction* const action : acs)
        {
            QDateTime appearanceTime = d->cameraAppearanceTimes.value(action->data().toString());

            if (latest.isNull() || (appearanceTime > latest))
            {
                primaryAction = action;
                latest        = appearanceTime;
            }
        }

        if (!primaryAction)
        {
            primaryAction = d->quickImportMenu->actions().constFirst();
        }

        connect(d->quickImportMenu->menuAction(), SIGNAL(triggered()),
                primaryAction, SLOT(trigger()));

        d->quickImportMenu->setEnabled(true);
    }
}

void DigikamApp::slotImportAddImages()
{
    QString startingPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QUrl url             = DFileDialog::getExistingDirectoryUrl(this, i18nc("@title:window", "Select Folder to Parse"),
                                                                QUrl::fromLocalFile(startingPath));

    if (url.isEmpty() || !url.isLocalFile())
    {
        return;
    }

    // The folder contents will be parsed by Camera interface in "Directory Browse" mode.

    downloadFrom(url.toLocalFile());
}

void DigikamApp::slotImportAddFolders()
{
    // NOTE: QFileDialog don't have an option to allow multiple selection of directories.
    // This work around is inspired from https://www.qtcentre.org/threads/34226-QFileDialog-select-multiple-directories
    // Check Later Qt 5.4 if a new native Qt way have been introduced.

    QPointer<DFileDialog> dlg = new DFileDialog(this);
    dlg->setWindowTitle(i18nc("@title:window", "Select Folders to Import into Album"));
    dlg->setFileMode(QFileDialog::Directory);
    dlg->setOptions(QFileDialog::ShowDirsOnly);

    QListView* const l = dlg->findChild<QListView*>(QLatin1String("listView"));

    if (l)
    {
        l->setSelectionMode(QAbstractItemView::MultiSelection);
    }

    QTreeView* const t = dlg->findChild<QTreeView*>();

    if (t)
    {
        t->setSelectionMode(QAbstractItemView::MultiSelection);
    }

    dlg->exec();

    if (!dlg->hasAcceptedUrls())
    {
        delete dlg;
        return;
    }

    QList<QUrl> urls = dlg->selectedUrls();
    delete dlg;

    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();
    Album* album            = nullptr;

    if (!albumList.isEmpty())
    {
        album = albumList.first();
    }

    if (album && (album->type() != Album::PHYSICAL))
    {
        album = nullptr;
    }

    QString header(i18n("<p>Please select the destination album from the digiKam library to "
                        "import folders into.</p>"));

    album = AlbumSelectDialog::selectAlbum(this, dynamic_cast<PAlbum*>(album), header);

    if (!album)
    {
        return;
    }

    PAlbum* const pAlbum = dynamic_cast<PAlbum*>(album);

    if (!pAlbum)
    {
        return;
    }

    QString albumRootPath = pAlbum->albumRootPath() + QLatin1Char('/');

    for (const QUrl& url : std::as_const(urls))
    {
        QString destPath = url.toLocalFile() + QLatin1Char('/');

        if (
            albumRootPath.contains(destPath) ||
            destPath.contains(albumRootPath)
           )
        {   // cppcheck-suppress useStlAlgorithm
            QMessageBox::warning(this, qApp->applicationName(),
                                 i18n("The folder %1 is part of the album "
                                      "path and cannot be imported recursively!",
                                      QDir::toNativeSeparators(url.toLocalFile())));
            return;
        }
    }

    DIO::copy(urls, pAlbum);
}

} // namespace Digikam
