/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export images to Dropbox web service
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbwindow.h"

// Qt includes

#include <QWindow>
#include <QSpinBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QCloseEvent>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "ditemslist.h"
#include "digikam_version.h"
#include "dbtalker.h"
#include "dbitem.h"
#include "dbnewalbumdlg.h"
#include "dbwidget.h"

namespace DigikamGenericDropBoxPlugin
{

class Q_DECL_HIDDEN DBWindow::Private
{
public:

    Private() = default;

    unsigned int   imagesCount          = 0;
    unsigned int   imagesTotal          = 0;

    DBWidget*      widget               = nullptr;
    DBNewAlbumDlg* albumDlg             = nullptr;
    DBTalker*      talker               = nullptr;

    QString        currentAlbumName;
    QList<QUrl>    transferQueue;
};

DBWindow::DBWindow(DInfoInterface* const iface, QWidget* const /*parent*/)
    : WSToolDialog(nullptr, QLatin1String("Dropbox Export Dialog")),
      d           (new Private)
{
    d->widget   = new DBWidget(this, iface, QLatin1String("Dropbox"));
    d->widget->imagesList()->setIface(iface);

    setMainWidget(d->widget);
    setModal(false);
    setWindowTitle(i18nc("@title:window", "Export to Dropbox"));

    startButton()->setText(i18nc("@action:button", "Start Upload"));
    startButton()->setToolTip(i18nc("@info:tooltip, button", "Start upload to Dropbox"));

    d->widget->setMinimumSize(700, 500);

    connect(d->widget->imagesList(), SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    connect(d->widget->getChangeUserBtn(), SIGNAL(clicked()),
            this, SLOT(slotUserChangeRequest()));

    connect(d->widget->getNewAlbmBtn(), SIGNAL(clicked()),
            this, SLOT(slotNewAlbumRequest()));

    connect(d->widget->getReloadBtn(), SIGNAL(clicked()),
            this, SLOT(slotReloadAlbumsRequest()));

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotStartTransfer()));

    d->albumDlg = new DBNewAlbumDlg(this, QLatin1String("Dropbox"));

    d->talker   = new DBTalker(this);

    connect(d->talker, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(d->talker, SIGNAL(signalLinkingFailed()),
            this, SLOT(slotSignalLinkingFailed()));

    connect(d->talker, SIGNAL(signalLinkingSucceeded()),
            this, SLOT(slotSignalLinkingSucceeded()));

    connect(d->talker, SIGNAL(signalSetUserName(QString)),
            this, SLOT(slotSetUserName(QString)));

    connect(d->talker, SIGNAL(signalListAlbumsFailed(QString)),
            this, SLOT(slotListAlbumsFailed(QString)));

    connect(d->talker, SIGNAL(signalListAlbumsDone(QList<QPair<QString,QString> >)),    // krazy:exclude=normalize
            this, SLOT(slotListAlbumsDone(QList<QPair<QString,QString> >)));            // krazy:exclude=normalize

    connect(d->talker, SIGNAL(signalCreateFolderFailed(QString)),
            this, SLOT(slotCreateFolderFailed(QString)));

    connect(d->talker, SIGNAL(signalCreateFolderSucceeded()),
            this, SLOT(slotCreateFolderSucceeded()));

    connect(d->talker, SIGNAL(signalAddPhotoFailed(QString)),
            this, SLOT(slotAddPhotoFailed(QString)));

    connect(d->talker, SIGNAL(signalAddPhotoSucceeded()),
            this, SLOT(slotAddPhotoSucceeded()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    readSettings();
    buttonStateChange(false);

    d->talker->link();
}

DBWindow::~DBWindow()
{
    delete d->talker;
    delete d;
}

void DBWindow::setItemsList(const QList<QUrl>& urls)
{
    d->widget->imagesList()->slotAddImages(urls);
}

void DBWindow::reactivate()
{
    d->widget->imagesList()->loadImagesFromCurrentSelection();
    d->widget->progressBar()->hide();

    show();
}

void DBWindow::readSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(QLatin1String("Dropbox Settings"));
    d->currentAlbumName     = grp.readEntry("Current Album", QString());

    if (grp.readEntry("Resize", false))
    {
        d->widget->getResizeCheckBox()->setChecked(true);
        d->widget->getDimensionSpB()->setEnabled(true);
    }
    else
    {
        d->widget->getResizeCheckBox()->setChecked(false);
        d->widget->getDimensionSpB()->setEnabled(false);
    }

    d->widget->getOriginalCheckBox()->setChecked(grp.readEntry("Upload Original", false));
    d->widget->getDimensionSpB()->setValue(grp.readEntry("Maximum Width",         1600));
    d->widget->getImgQualitySpB()->setValue(grp.readEntry("Image Quality",        90));
}

void DBWindow::writeSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(QLatin1String("Dropbox Settings"));

    grp.writeEntry("Current Album",   d->currentAlbumName);
    grp.writeEntry("Resize",          d->widget->getResizeCheckBox()->isChecked());
    grp.writeEntry("Upload Original", d->widget->getOriginalCheckBox()->isChecked());
    grp.writeEntry("Maximum Width",   d->widget->getDimensionSpB()->value());
    grp.writeEntry("Image Quality",   d->widget->getImgQualitySpB()->value());
}

void DBWindow::slotSetUserName(const QString& msg)
{
    d->widget->updateLabels(msg, QLatin1String(""));
}

void DBWindow::slotListAlbumsDone(const QList<QPair<QString, QString> >& list)
{
    d->widget->getAlbumsCoB()->clear();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotListAlbumsDone:" << list.size();

    for (int i = 0 ; i < list.size() ; ++i)
    {
        d->widget->getAlbumsCoB()->addItem(
        QIcon::fromTheme(QLatin1String("system-users")),
        list.value(i).second, list.value(i).first);

        if (d->currentAlbumName == list.value(i).first)
        {
            d->widget->getAlbumsCoB()->setCurrentIndex(i);
        }
    }

    buttonStateChange(true);
    d->talker->getUserName();
}

void DBWindow::slotBusy(bool val)
{
    if (val)
    {
        setCursor(Qt::WaitCursor);
        d->widget->getChangeUserBtn()->setEnabled(false);
        buttonStateChange(false);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        d->widget->getChangeUserBtn()->setEnabled(true);
        buttonStateChange(true);
    }
}

void DBWindow::slotStartTransfer()
{
    d->widget->imagesList()->clearProcessedStatus();

    if (d->widget->imagesList()->imageUrls().isEmpty())
    {
        QMessageBox::critical(this, i18nc("@title:window", "Error"),
                              i18n("No image selected. Please select which images should be uploaded."));
        return;
    }

    if (!(d->talker->authenticated()))
    {
        if (QMessageBox::question(this, i18nc("@title:window", "Login Failed"),
                                  i18n("Authentication failed. Do you want to try again?"))
            == QMessageBox::Yes)
        {
            d->talker->link();
            return;
        }
        else
        {
            return;
        }
    }

    d->transferQueue = d->widget->imagesList()->imageUrls();

    if (d->transferQueue.isEmpty())
    {
        return;
    }

    d->currentAlbumName = d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString();

    d->imagesTotal = d->transferQueue.count();
    d->imagesCount = 0;

    d->widget->progressBar()->setFormat(i18n("%v / %m"));
    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(0);
    d->widget->progressBar()->show();
    d->widget->progressBar()->progressScheduled(i18n("Dropbox export"), true, true);
    d->widget->progressBar()->progressThumbnailChanged(
        QIcon::fromTheme(QLatin1String("dk-dropbox")).pixmap(22, 22));

    uploadNextPhoto();
}

void DBWindow::uploadNextPhoto()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "uploadNextPhoto:" << d->transferQueue.count();

    if (d->transferQueue.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "empty";
        d->widget->progressBar()->progressCompleted();
        return;
    }

    QString imgPath = d->transferQueue.first().toLocalFile();
    QString temp    = d->currentAlbumName + QLatin1Char('/');

    bool res = d->talker->addPhoto(imgPath,
                                   temp,
                                   d->widget->getOriginalCheckBox()->isChecked(),
                                   d->widget->getResizeCheckBox()->isChecked(),
                                   d->widget->getDimensionSpB()->value(),
                                   d->widget->getImgQualitySpB()->value());

    if (!res)
    {
        slotAddPhotoFailed(QLatin1String(""));
        return;
    }
}

void DBWindow::slotAddPhotoFailed(const QString& msg)
{
    if (QMessageBox::question(this, i18nc("@title:window", "Uploading Failed"),
                              i18n("Failed to upload photo to Dropbox."
                                   "\n%1\n"
                                   "Do you want to continue?", msg))
        != QMessageBox::Yes)
    {
        d->transferQueue.clear();
        d->widget->progressBar()->hide();
    }
    else
    {
        d->transferQueue.removeFirst();
        d->imagesTotal--;
        d->widget->progressBar()->setMaximum(d->imagesTotal);
        d->widget->progressBar()->setValue(d->imagesCount);
        uploadNextPhoto();
    }
}

void DBWindow::slotAddPhotoSucceeded()
{
    // Remove photo uploaded from the list

    d->widget->imagesList()->removeItemByUrl(d->transferQueue.first());
    d->transferQueue.removeFirst();
    d->imagesCount++;
    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(d->imagesCount);
    uploadNextPhoto();
}

void DBWindow::slotImageListChanged()
{
    startButton()->setEnabled(!(d->widget->imagesList()->imageUrls().isEmpty()));
}

void DBWindow::slotNewAlbumRequest()
{
    if (d->albumDlg->exec() == QDialog::Accepted)
    {
        DBFolder newFolder;
        d->albumDlg->getFolderTitle(newFolder);
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotNewAlbumRequest:" << newFolder.title;
        d->currentAlbumName = d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString();
        d->currentAlbumName = d->currentAlbumName + newFolder.title;
        d->talker->createFolder(d->currentAlbumName);
    }
}

void DBWindow::slotReloadAlbumsRequest()
{
    d->talker->listFolders();
}

void DBWindow::slotSignalLinkingFailed()
{
    slotSetUserName(QLatin1String(""));
    d->widget->getAlbumsCoB()->clear();

    if (QMessageBox::question(this, i18nc("@title:window", "Login Failed"),
                              i18n("Authentication failed. Do you want to try again?"))
        == QMessageBox::Yes)
    {
        d->talker->link();
    }
}

void DBWindow::slotSignalLinkingSucceeded()
{
    d->talker->listFolders();
}

void DBWindow::slotListAlbumsFailed(const QString& msg)
{
    QMessageBox::critical(this, QString(), i18n("Dropbox call failed:\n%1", msg));
}

void DBWindow::slotCreateFolderFailed(const QString& msg)
{
    QMessageBox::critical(this, QString(), i18n("Dropbox call failed:\n%1", msg));
}

void DBWindow::slotCreateFolderSucceeded()
{
    d->talker->listFolders();
}

void DBWindow::slotTransferCancel()
{
    d->transferQueue.clear();
    d->widget->progressBar()->hide();
    d->talker->cancel();
}

void DBWindow::slotUserChangeRequest()
{
    slotSetUserName(QLatin1String(""));
    d->widget->getAlbumsCoB()->clear();
    d->talker->unLink();
    d->talker->link();
}

void DBWindow::buttonStateChange(bool state)
{
    d->widget->getNewAlbmBtn()->setEnabled(state);
    d->widget->getReloadBtn()->setEnabled(state);
    startButton()->setEnabled(state);
}

void DBWindow::slotFinished()
{
    writeSettings();
    d->widget->imagesList()->listView()->clear();
}

void DBWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

} // namespace DigikamGenericDropBoxPlugin

#include "moc_dbwindow.cpp"
