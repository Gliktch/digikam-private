/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor - Open files
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfoto_p.h"

namespace ShowFoto
{

void Showfoto::slotOpenFile()
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    QList<QUrl> urls = Digikam::ImageDialog::getImageURLs(this, d->lastOpenedDirectory);

    if      (urls.count() > 1)
    {
        slotClearThumbBar();

        openUrls(urls);

        Q_EMIT signalInfoList(d->infoList);

        slotOpenUrl(d->thumbBar->currentInfo());
        toggleNavigation(1);
    }
    else if (urls.count() == 1)
    {
        slotClearThumbBar();

        openFolder(urls.first().adjusted(QUrl::RemoveFilename));

        Q_EMIT signalInfoList(d->infoList);

        slotOpenUrl(d->thumbBar->findItemByUrl(urls.first()));
        d->thumbBar->setCurrentUrl(urls.first());
    }
}

void Showfoto::slotOpenFolder()
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    QUrl url = DFileDialog::getExistingDirectoryUrl(this, i18nc("@title:window", "Open Images from Folder"),
                                                    d->lastOpenedDirectory);
    if (!url.isEmpty())
    {
        slotClearThumbBar();

        openFolder(url);

        Q_EMIT signalInfoList(d->infoList);

        slotOpenUrl(d->thumbBar->currentInfo());
        toggleNavigation(1);
    }
}

void Showfoto::slotOpenFolderFromPath(const QString& path)
{
    qCDebug(DIGIKAM_SHOWFOTO_LOG) << "Open folder from path =" << path;

    QFileInfo inf(path);
    slotClearThumbBar();

    if      (inf.isFile())
    {
        slotDroppedUrls(QList<QUrl>() << QUrl::fromLocalFile(inf.absolutePath()), false, QUrl());
        d->thumbBar->setCurrentUrl(QUrl::fromLocalFile(path));
        slotOpenUrl(d->thumbBar->currentInfo());
    }
    else if (inf.isDir())
    {
        QString dpath(path.endsWith(QLatin1Char('/')) ? path : path + QLatin1Char('/'));
        slotDroppedUrls(QList<QUrl>() << QUrl::fromLocalFile(dpath), false, QUrl());
        QList<QUrl> urls = d->thumbBar->urls();

        if (!urls.isEmpty())
        {
            d->thumbBar->setCurrentUrl(urls.first());
            slotOpenUrl(d->thumbBar->currentInfo());
        }
    }
    else
    {
        return;
    }

    applySortSettings();
}

void Showfoto::openUrls(const QList<QUrl>& urls)
{
    if (urls.isEmpty())
    {
        return;
    }

    ShowfotoItemInfo iteminfo;

    for (QList<QUrl>::const_iterator it = urls.constBegin() ; it != urls.constEnd() ; ++it)
    {
        QFileInfo fi((*it).toLocalFile());
        iteminfo = ShowfotoItemInfo::itemInfoFromFile(fi);

        if (!d->infoList.contains(iteminfo))
        {
            d->infoList << iteminfo;
        }
    }
}

void Showfoto::openFolder(const QUrl& url)
{
    if (!url.isValid() || !url.isLocalFile())
    {
        return;
    }

    d->lastOpenedDirectory = url;

    // Parse image IO mime types registration to get files filter pattern.

    QString filter;
    QString patterns       = filter.toLower();
    patterns.append(QLatin1Char(' '));
    patterns.append(filter.toUpper());

    qCDebug(DIGIKAM_SHOWFOTO_LOG) << "patterns=" << patterns;

    // Get all image files from directory.

    QDir dir(url.toLocalFile(), patterns);
    dir.setFilter(QDir::Files);

    if (!dir.exists())
    {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QFileInfoList fileinfolist = dir.entryInfoList();

    if (fileinfolist.isEmpty())
    {
        QApplication::restoreOverrideCursor();
        return;
    }

    QFileInfoList::const_iterator fi;
    ShowfotoItemInfo iteminfo;

    // And open all items in image editor.

    for (fi = fileinfolist.constBegin() ; fi != fileinfolist.constEnd() ; ++fi)
    {
        iteminfo = ShowfotoItemInfo::itemInfoFromFile(*fi);

        if (!d->infoList.contains(iteminfo))
        {
            d->infoList << iteminfo;
        }
    }

    QApplication::restoreOverrideCursor();
}

void Showfoto::slotOpenFilesfromPath(const QStringList& files, const QString& current)
{
    if (files.isEmpty())
    {
        return;
    }

    slotClearThumbBar();
    QList<QUrl> urls;

    for (const QString& path : std::as_const(files))
    {
        urls << QUrl::fromLocalFile(path);
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "slotOpenFilesfromPath:: urls list" << urls;

    openUrls(urls);

    Q_EMIT signalInfoList(d->infoList);

    QUrl curl = QUrl::fromLocalFile(current);
    slotOpenUrl(d->thumbBar->showfotoItemModel()->showfotoItemInfo(curl));

    int index = d->thumbBar->thumbnailIndexForUrl(curl);
    toggleNavigation(index);

    qCDebug(DIGIKAM_GENERAL_LOG) << "slotOpenFilesfromPath:: switch to thumb index" << index;

    applySortSettings();
    d->thumbBar->setCurrentUrl(curl);
}

void Showfoto::slotAppendFilesfromPath(const QStringList& files, const QString& current)
{
    if (files.isEmpty())
    {
        return;
    }

    QList<QUrl> urls;

    for (const QString& file : std::as_const(files))
    {
        urls << QUrl::fromLocalFile(file);
    }

    slotDroppedUrls(urls, false, QUrl::fromLocalFile(current));
    applySortSettings();
}

void Showfoto::slotDroppedUrls(const QList<QUrl>& droppedUrls, bool dropped, const QUrl& current)
{
    if (droppedUrls.isEmpty())
    {
        return;
    }

    QList<QUrl> imagesUrls;
    QList<QUrl> foldersUrls;

    for (const QUrl& drop : std::as_const(droppedUrls))
    {
        if (drop.isValid())
        {
            QFileInfo info(drop.toLocalFile());
            QUrl url(QUrl::fromLocalFile(info.canonicalFilePath()));

            if (isReadableImageFile(info.absoluteFilePath()))
            {
                imagesUrls << url;
            }

            if (info.isDir())
            {
                foldersUrls << url;
            }
        }
    }

    if (!imagesUrls.isEmpty())
    {
        openUrls(imagesUrls);
    }

    if (!foldersUrls.isEmpty())
    {
        for (const QUrl& fUrl : std::as_const(foldersUrls))
        {
            openFolder(fUrl);
        }
    }

    if (!d->infoList.isEmpty())
    {
        if (!dropped && foldersUrls.isEmpty() && (imagesUrls.count() == 1))
        {
            openFolder(imagesUrls.first().adjusted(QUrl::RemoveFilename));
            d->model->clearShowfotoItemInfos();

            Q_EMIT signalInfoList(d->infoList);

            slotOpenUrl(d->thumbBar->findItemByUrl(current.isValid() ? current : imagesUrls.first()));
            d->thumbBar->setCurrentUrl(current.isValid() ? current : imagesUrls.first());

            return;
        }

        d->model->clearShowfotoItemInfos();

        Q_EMIT signalInfoList(d->infoList);

        slotOpenUrl(d->thumbBar->currentInfo());
    }
    else
    {
        QMessageBox::information(this, qApp->applicationName(),
                                 i18n("There is no item to process."));
        qCWarning(DIGIKAM_SHOWFOTO_LOG) << "infolist is empty..";
    }
}

void Showfoto::slotOpenUrl(const ShowfotoItemInfo& info)
{
    if (d->thumbBar->currentInfo().isNull() || info.isNull())
    {
        return;
    }

    QString localFile;

    if (info.url.isLocalFile())
    {
        // file protocol. We do not need the network

        localFile = info.url.toLocalFile();
    }
    else
    {
        QMessageBox::critical(this, i18nc("@title:window", "Error Loading File"),
                              i18n("Failed to load file: \"%1\"\n"
                                   "Remote file handling is not supported",
                                   info.url.fileName()));
        return;
    }

    d->currentLoadedUrl = info.url;
    d->folderView->setCurrentPath(localFile);
    m_canvas->load(localFile, m_IOFileSettings);

    //TODO : add preload here like in ImageWindow::slotLoadCurrent() ???
}

void Showfoto::slotAddedDropedItems(QDropEvent* e)
{
    QList<QUrl> list = e->mimeData()->urls();
    QList<QUrl> urls;

    for (const QUrl& url : std::as_const(list))
    {
        QFileInfo fi(url.toLocalFile());

        if (fi.exists())
        {
            urls.append(url);
        }
    }

    e->accept();

    if (!urls.isEmpty())
    {
        slotDroppedUrls(urls, true, QUrl());
    }
}

void Showfoto::slotFileWithDefaultApplication()
{
    Digikam::DFileOperations::openFilesWithDefaultApplication(QList<QUrl>() << d->thumbBar->currentUrl());
}

void Showfoto::slotOpenWith(QAction* action)
{
    openWith(d->thumbBar->currentUrl(), action);
}

} // namespace ShowFoto
