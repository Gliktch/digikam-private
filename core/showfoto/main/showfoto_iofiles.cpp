/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor - IO files.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfoto_p.h"

namespace ShowFoto
{

void Showfoto::slotPrepareToLoad()
{
    Digikam::EditorWindow::slotPrepareToLoad();

    // Here we enable specific actions on showfoto.

    d->openFilesInFolderAction->setEnabled(true);
    d->fileOpenAction->setEnabled(true);
}

void Showfoto::slotLoadingStarted(const QString& filename)
{
    Digikam::EditorWindow::slotLoadingStarted(filename);

    // Here we disable specific actions on showfoto.

    d->openFilesInFolderAction->setEnabled(false);
    d->fileOpenAction->setEnabled(false);
}

void Showfoto::slotLoadingFinished(const QString& filename, bool success)
{
    Digikam::EditorWindow::slotLoadingFinished(filename, success);

    // Here we re-enable specific actions on showfoto.

    d->openFilesInFolderAction->setEnabled(true);
    d->fileOpenAction->setEnabled(true);
}

void Showfoto::slotSavingStarted(const QString& filename)
{
    Digikam::EditorWindow::slotSavingStarted(filename);

    // Here we disable specific actions on showfoto.

    d->openFilesInFolderAction->setEnabled(false);
    d->fileOpenAction->setEnabled(false);
}

void Showfoto::moveFile()
{
    /*
     * moveFile() -> moveLocalFile() ->  movingSaveFileFinished()
     *                                     |               |
     *                            finishSaving(true)  save...IsComplete()
     */

    qCDebug(DIGIKAM_SHOWFOTO_LOG) << m_savingContext.destinationURL
                                  << m_savingContext.destinationURL.isLocalFile();

    if (m_savingContext.destinationURL.isLocalFile())
    {
        qCDebug(DIGIKAM_SHOWFOTO_LOG) << "moving a local file";
        EditorWindow::moveFile();
    }
    else
    {
        QMessageBox::critical(this, i18nc("@title:window", "Error Saving File"),
                              i18n("Failed to save file: %1",
                              i18n("Remote file handling is not supported")));
    }
}

void Showfoto::finishSaving(bool success)
{
    Digikam::EditorWindow::finishSaving(success);

    // Here we re-enable specific actions on showfoto.

    d->openFilesInFolderAction->setEnabled(true);
    d->fileOpenAction->setEnabled(true);
}

void Showfoto::saveIsComplete()
{
    Digikam::LoadingCacheInterface::putImage(m_savingContext.destinationURL.toLocalFile(), m_canvas->currentImage());
/*
    d->thumbBar->invalidateThumb(d->currentItem);
*/
    // Pop-up a message to bring user when save is done.

    Digikam::DNotificationWrapper(QLatin1String("editorsavefilecompleted"), i18n("Image saved successfully"),
                                  this, windowTitle());

    resetOrigin();
}

void Showfoto::saveAsIsComplete()
{
    resetOriginSwitchFile();
/*
    Digikam::LoadingCacheInterface::putImage(m_savingContext.destinationURL.toLocalFile(), m_canvas->currentImage());

    // Add the file to the list of thumbbar images if it's not there already

    Digikam::ThumbBarItem* foundItem = d->thumbBar->findItemByUrl(m_savingContext.destinationURL);
    d->thumbBar->invalidateThumb(foundItem);
    qCDebug(DIGIKAM_SHOWFOTO_LOG) << wantedUrls;

    if (!foundItem)
    {
        foundItem = new Digikam::ThumbBarItem(d->thumbBar, m_savingContext.destinationURL);
    }

    // shortcut slotOpenUrl
    d->thumbBar->blockSignals(true);
    d->thumbBar->setSelected(foundItem);
    d->thumbBar->blockSignals(false);
    d->currentItem = foundItem;
    slotUpdateItemInfo();

    // Pop-up a message to bring user when save is done.

    Digikam::DNotificationWrapper("editorsavefilecompleted", i18n("Image saved successfully"),
                                  this, windowTitle());
*/
}

void Showfoto::saveVersionIsComplete()
{
}

QUrl Showfoto::saveDestinationUrl()
{
    if (d->thumbBar->currentInfo().isNull())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot return the url of the image to save "
                                       << "because no image is selected.";
        return QUrl();
    }

    return d->thumbBar->currentUrl();
}

bool Showfoto::save()
{
    if (d->thumbBar->currentInfo().isNull())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "This should not happen";

        return true;
    }

    startingSave(d->currentLoadedUrl);

    return true;
}

bool Showfoto::saveAs()
{
    if (d->thumbBar->currentInfo().isNull())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "This should not happen";

        return false;
    }

    return (startingSaveAs(d->currentLoadedUrl));
}

void Showfoto::slotDeleteCurrentItem()
{
    QUrl urlCurrent(d->thumbBar->currentUrl());

    QString warnMsg(i18n("About to delete file \"%1\"\nAre you sure?",
                         urlCurrent.fileName()));

    if (QMessageBox::warning(this, qApp->applicationName(), warnMsg,
                             QMessageBox::Apply | QMessageBox::Abort)
        !=  QMessageBox::Apply)
    {
        return;
    }
    else
    {
        bool ret = QFile::remove(urlCurrent.toLocalFile());

        if (!ret)
        {
            QMessageBox::critical(this, qApp->applicationName(),
                                  i18n("Cannot delete \"%1\"", urlCurrent.fileName()));
            return;
        }

        // No error, remove item in thumbbar.

        d->model->removeIndex(d->thumbBar->currentIndex());

        // Disable menu actions and SideBar if no current image.

        d->itemsNb = d->thumbBar->showfotoItemInfos().size();

        if (d->itemsNb == 0)
        {
            slotUpdateItemInfo();
            toggleActions(false);
            m_canvas->load(QString(), m_IOFileSettings);
            Q_EMIT signalNoCurrentItem();
        }
        else
        {
            // If there is an image after the deleted one, make that selected.

            slotOpenUrl(d->thumbBar->currentInfo());
        }
    }
}

void Showfoto::slotRevert()
{
    if (!promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    m_canvas->slotRestore();
}

bool Showfoto::saveNewVersion()
{
    return false;
}

bool Showfoto::saveCurrentVersion()
{
    return false;
}

bool Showfoto::saveNewVersionAs()
{
    return false;
}

bool Showfoto::saveNewVersionInFormat(const QString&)
{
    return false;
}

void Showfoto::slotRemoveImageFromAlbum(const QUrl& url)
{
    d->thumbBar->setCurrentUrl(url);

    d->model->removeIndex(d->thumbBar->currentIndex());

    // Disable menu actions and SideBar if no current image.

    d->itemsNb = d->thumbBar->showfotoItemInfos().size();

    if (d->itemsNb == 0)
    {
        slotUpdateItemInfo();
        toggleActions(false);
        m_canvas->load(QString(), m_IOFileSettings);
        Q_EMIT signalNoCurrentItem();
    }
}

} // namespace ShowFoto
