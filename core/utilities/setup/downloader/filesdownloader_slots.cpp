/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-11-14
 * Description : Files downloader
 *
 * SPDX-FileCopyrightText: 2020-2024 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "filesdownloader_p.h"

namespace Digikam
{

void FilesDownloader::slotDownload()
{
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
    d->aiAutoToolsCheck->setEnabled(false);
    d->faceEngineCheck->setEnabled(false);
    d->aestheticCheck->setEnabled(false);
    d->autoTagsCheck->setEnabled(false);
    d->selector->setEnabled(false);

    if (d->error.isEmpty())
    {
        while (!d->files.isEmpty())
        {
            d->currentInfo = d->files.takeFirst();

            if (!downloadExists(d->currentInfo))
            {
                d->count++;
                download();

                return;
            }
        }

        QMessageBox::information(this, qApp->applicationName(),
                                 i18n("All model files were downloaded successfully."));

        deleteUnusedFiles();

        close();
    }
    else
    {
        QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Critical,
                 i18nc("@title:window", "Download Error"),
                 i18n("An error occurred during the download.\n\n"
                      "File: %1\n\n%2\n\n"
                      "You can try again or continue the "
                      "download at the next start.",
                      d->currentInfo.name, d->error),
                 QMessageBox::Yes | QMessageBox::Cancel,
                 qApp->activeWindow());

        msgBox->button(QMessageBox::Yes)->setText(i18nc("@action:button", "Try Again"));
        msgBox->button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("edit-download")));

        int result = msgBox->exec();
        delete msgBox;

        if (result == QMessageBox::Yes)
        {
            d->fallback = !d->fallback;
            d->error.clear();
            download();

            return;
        }

        close();
    }
}

void FilesDownloader::reject()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    QDialog::reject();
}

void FilesDownloader::slotDownloaded(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    d->reply = nullptr;

    if (
        (reply->error() != QNetworkReply::NoError)             &&
        (reply->error() != QNetworkReply::InsecureRedirectError)
       )
    {
        d->error = reply->errorString();

        reply->deleteLater();

        nextDownload();

        return;
    }

    QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (redirectUrl.isValid() && (reply->url() != redirectUrl) && (d->redirects < 10))
    {
        createRequest(redirectUrl);

        reply->deleteLater();

        return;
    }

    QCryptographicHash sha256(QCryptographicHash::Sha256);

    QByteArray data = reply->readAll();

    sha256.addData(data);

    if (d->currentInfo.hash != QString::fromLatin1(sha256.result().toHex()))
    {
        d->error = i18n("Checksum is incorrect.");

        reply->deleteLater();

        nextDownload();

        return;
    }

    QString path = getFacesEnginePath();

    if (!QFileInfo::exists(path))
    {
        QDir().mkpath(path);
    }

    QFile file(path + QLatin1Char('/') + d->currentInfo.name);

    if (file.open(QIODevice::WriteOnly))
    {
        qint64 written = file.write(data);

        if (written != d->currentInfo.size)
        {
            d->error = i18n("File write error.");
        }

        file.close();
    }
    else
    {
        d->error = i18n("File open error.");
    }

    reply->deleteLater();

    nextDownload();
}

void FilesDownloader::slotDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (d->reply && (bytesReceived > d->currentInfo.size))
    {
        d->reply->abort();
        d->reply = nullptr;

        d->error = i18n("File on the server is too large.");

        nextDownload();

        return;
    }

    d->progress->setMaximum(bytesTotal);
    d->progress->setValue(bytesReceived);
}

void FilesDownloader::slotFacesEnginePathChanged()
{
    QString path = d->selector->fileDlgPath();

    while (!path.isEmpty() && path.endsWith(QLatin1Char('/')))
    {
        if (path.length() == 1)
        {
            break;
        }

        path.chop(1);
    }

    {
        SystemSettings system(QLatin1String("digikam"));
        system.facesEnginePath = path;
        system.saveSettings();
    }

    slotUpdateDownloadInfo();
}

void FilesDownloader::slotOpenLocalRepo()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(getFacesEnginePath()));
}

void FilesDownloader::slotHelp()
{
    openOnlineDocumentation(QLatin1String("getting_started"),
                            QLatin1String("quick_start"),
                            QLatin1String("firstrun-downloads"));
}

void FilesDownloader::slotUpdateDownloadInfo()
{
    {
        SystemSettings appSystem(qApp->applicationName());

        appSystem.enableAIAutoTools = d->aiAutoToolsCheck->isChecked();
        appSystem.enableFaceEngine  = d->faceEngineCheck->isChecked();
        appSystem.enableAesthetic   = d->aestheticCheck->isChecked();
        appSystem.enableAutoTags    = d->autoTagsCheck->isChecked();
        appSystem.saveSettings();
    }

    createDownloadInfo();

    qint64 size = 0;
    d->total    = 0;

    for (const DownloadInfo& info : std::as_const(d->files))
    {
        if (!downloadExists(info))
        {
            // cppcheck-suppress useStlAlgorithm
            size += info.size;
            d->total++;
        }
    }

    QString sizeString = ItemPropertiesTab::humanReadableBytesCount(size);

    d->infoLabel->setText(i18n("<p>digiKam relies on large binary model files to power features like face management, "
                               "image classification based on aesthetics, AI AutoTools such as automatic image rotation, "
                               "and automatic tag assignment. You have the option to select the specific features you want to "
                               "enable.</p><p><b>Note:</b> The red-eye removal tool requires face management model files.</p>"));

    d->loadLabel->setText(i18n("<p>Some required files are missing. Click “Download” to start installing the necessary model files. "
                               "If you close this dialog you will be prompted again the next time you launch digiKam. The "
                               "selected features will not function without these files.</p>"
                               "<p><b>You must restart digiKam after successfully downloading the files.</b></p>"));
    if (size > 0)
    {
        d->sizeLabel->setText(i18nc("%1: file counter, %2: disk size with unit",
                                    "The download requires %1 files with a size of %2.",
                                    d->total, sizeString));

        QDir dir(getFacesEnginePath());

        while (!dir.isRoot() && !dir.exists())
        {
            if (!dir.cdUp())
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot change directory up to" << dir;

                dir = QDir();
                break;
            }
        }

        QFileInfo info(dir.path());
        d->loadLabel->setForegroundRole(foregroundRole());
        d->buttons->button(QDialogButtonBox::Ok)->setEnabled(
                                                             info.exists()                   &&
                                                             info.isWritable()               &&
                                                             (dir.path() != QLatin1String("."))
                                                            );
    }
    else
    {
        d->sizeLabel->setText(i18n("All files for the selected features were found."));

        d->loadLabel->setForegroundRole(backgroundRole());
        d->buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

} // namespace Digikam
