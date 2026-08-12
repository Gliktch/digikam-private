/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Box web service
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "boxwindow.h"

// Qt includes

#include <QPointer>
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
#include "boxtalker.h"
#include "boxitem.h"
#include "boxnewalbumdlg.h"
#include "boxwidget.h"

namespace DigikamGenericBoxPlugin
{

class Q_DECL_HIDDEN BOXWindow::Private
{
public:

    Private() = default;

public:

    unsigned int    imagesCount         = 0;
    unsigned int    imagesTotal         = 0;

    BOXWidget*      widget              = nullptr;
    BOXNewAlbumDlg* albumDlg            = nullptr;
    BOXTalker*      talker              = nullptr;

    QString         currentAlbumName;
    QList<QUrl>     transferQueue;
};

BOXWindow::BOXWindow(DInfoInterface* const iface,
                     QWidget* const /*parent*/)
    : WSToolDialog(nullptr, QLatin1String("Box Export Dialog")),
      d           (new Private)
{
    d->widget = new BOXWidget(this, iface, QLatin1String("Box"));

    d->widget->imagesList()->setIface(iface);

    setMainWidget(d->widget);
    setModal(false);
    setWindowTitle(i18nc("@title:window", "Export to Box"));

    startButton()->setText(i18nc("@action:button", "Start Upload"));
    startButton()->setToolTip(i18nc("@info:tooltip, button", "Start upload to Box"));

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

    d->albumDlg = new BOXNewAlbumDlg(this, QLatin1String("Box"));
    d->talker   = new BOXTalker(this);

    connect(d->talker,SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(d->talker, SIGNAL(signalLinkingFailed()),
            this, SLOT(slotSignalLinkingFailed()));

    connect(d->talker, SIGNAL(signalLinkingSucceeded()),
            this, SLOT(slotSignalLinkingSucceeded()));

    connect(d->talker, SIGNAL(signalSetUserName(QString)),
            this, SLOT(slotSetUserName(QString)));

    connect(d->talker, SIGNAL(signalListAlbumsFailed(QString)),
            this, SLOT(slotListAlbumsFailed(QString)));

    connect(d->talker, SIGNAL(signalListAlbumsDone(QList<QPair<QString,QString> >)), // krazy:exclude=normalize
            this, SLOT(slotListAlbumsDone(QList<QPair<QString,QString> >)));         // krazy:exclude=normalize

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

BOXWindow::~BOXWindow()
{
    delete d->widget;
    delete d->albumDlg;
    delete d->talker;
    delete d;
}

void BOXWindow::readSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(QLatin1String("Box Settings"));
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

    d->widget->getDimensionSpB()->setValue(grp.readEntry("Maximum Width",  1600));
    d->widget->getImgQualitySpB()->setValue(grp.readEntry("Image Quality", 90));
}

void BOXWindow::writeSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(QLatin1String("Box Settings"));

    grp.writeEntry("Current Album", d->currentAlbumName);
    grp.writeEntry("Resize",        d->widget->getResizeCheckBox()->isChecked());
    grp.writeEntry("Maximum Width", d->widget->getDimensionSpB()->value());
    grp.writeEntry("Image Quality", d->widget->getImgQualitySpB()->value());
}

void BOXWindow::reactivate()
{
    d->widget->imagesList()->loadImagesFromCurrentSelection();
    d->widget->progressBar()->hide();

    show();
}

void BOXWindow::setItemsList(const QList<QUrl>& urls)
{
    d->widget->imagesList()->slotAddImages(urls);
}

void BOXWindow::slotBusy(bool val)
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

void BOXWindow::slotSetUserName(const QString& msg)
{
    d->widget->updateLabels(msg, QLatin1String(""));
}

void BOXWindow::slotListAlbumsDone(const QList<QPair<QString,QString> >& list)
{
    d->widget->getAlbumsCoB()->clear();

    for (int i = 0 ; i < list.size() ; ++i)
    {
        d->widget->getAlbumsCoB()->addItem(
            QIcon::fromTheme(QLatin1String("system-users")),
            list.value(i).second, list.value(i).second);

        if (d->currentAlbumName == QString(list.value(i).second))
        {
            d->widget->getAlbumsCoB()->setCurrentIndex(i);
        }
    }

    buttonStateChange(true);
    d->talker->getUserName();
}

void BOXWindow::slotStartTransfer()
{
    d->widget->imagesList()->clearProcessedStatus();

    if (d->widget->imagesList()->imageUrls().isEmpty())
    {
        QMessageBox::critical(this, i18nc("@title:window", "Error"),
                              i18nc("@info", "No image selected. Please select which images should be uploaded."));
        return;
    }

    if (!(d->talker->authenticated()))
    {
        QPointer<QMessageBox> warn = new QMessageBox(QMessageBox::Warning,
                i18nc("@title: window start transfer", "Warning"),
                i18nc("@info", "Authentication failed. Click \"Continue\" to authenticate."),
                QMessageBox::Yes | QMessageBox::No);

        (warn->button(QMessageBox::Yes))->setText(i18nc("@action: button start transfer", "Continue"));
        (warn->button(QMessageBox::No))->setText(i18nc("@action: button start transfer", "Cancel"));

        if (warn->exec() == QMessageBox::Yes)
        {
            d->talker->link();
            delete warn;
            return;
        }
        else
        {
            delete warn;
            return;
        }
    }

    d->transferQueue = d->widget->imagesList()->imageUrls();

    if (d->transferQueue.isEmpty())
    {
        return;
    }

    d->currentAlbumName = d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "StartTransfer:" << d->currentAlbumName
                                     << "INDEX: " << d->widget->getAlbumsCoB()->currentIndex();
    d->imagesTotal = d->transferQueue.count();
    d->imagesCount = 0;

    d->widget->progressBar()->setFormat(i18nc("@info: progress bar", "%v / %m"));
    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(0);
    d->widget->progressBar()->show();
    d->widget->progressBar()->progressScheduled(i18nc("@info", "Box export"), true, true);
    d->widget->progressBar()->progressThumbnailChanged(
        QIcon::fromTheme(QLatin1String("dk-box")).pixmap(22, 22));

    uploadNextPhoto();
}

void BOXWindow::uploadNextPhoto()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "uploadNextPhoto:" << d->transferQueue.count();

    if (d->transferQueue.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "empty";
        d->widget->progressBar()->progressCompleted();
        return;
    }

    QString imgPath = d->transferQueue.first().toLocalFile();
    QString temp    = d->currentAlbumName;

    bool result = d->talker->addPhoto(imgPath,
                                      temp,
                                      d->widget->getResizeCheckBox()->isChecked(),
                                      d->widget->getDimensionSpB()->value(),
                                      d->widget->getImgQualitySpB()->value());

    if (!result)
    {
        slotAddPhotoFailed(QLatin1String(""));
        return;
    }
}

void BOXWindow::slotAddPhotoFailed(const QString& msg)
{
    if (QMessageBox::question(this, i18nc("@title:window", "Uploading Failed"),
                              i18nc("@info", "Failed to upload photo to Box."
                                    "\n%1\n"
                                    "Do you want to continue?", msg))
        != QMessageBox::Yes)
    {
        d->transferQueue.clear();
        d->widget->progressBar()->hide();
    }
    else
    {
        d->transferQueue.pop_front();
        d->imagesTotal--;
        d->widget->progressBar()->setMaximum(d->imagesTotal);
        d->widget->progressBar()->setValue(d->imagesCount);
        uploadNextPhoto();
    }
}

void BOXWindow::slotAddPhotoSucceeded()
{
    // Remove photo uploaded from the list

    d->widget->imagesList()->removeItemByUrl(d->transferQueue.first());
    d->transferQueue.pop_front();
    d->imagesCount++;
    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(d->imagesCount);
    uploadNextPhoto();
}

void BOXWindow::slotImageListChanged()
{
    startButton()->setEnabled(!(d->widget->imagesList()->imageUrls().isEmpty()));
}

void BOXWindow::slotNewAlbumRequest()
{
    if (d->albumDlg->exec() == QDialog::Accepted)
    {
        BOXFolder newFolder;
        d->albumDlg->getFolderTitle(newFolder);
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotNewAlbumRequest:" << newFolder.title;
        d->currentAlbumName = d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString();
        d->currentAlbumName = d->currentAlbumName + newFolder.title;
        d->talker->createFolder(d->currentAlbumName);
    }
}

void BOXWindow::slotReloadAlbumsRequest()
{
    d->talker->listFolders();
}

void BOXWindow::slotSignalLinkingFailed()
{
    slotSetUserName(QLatin1String(""));
    d->widget->getAlbumsCoB()->clear();

    if (QMessageBox::question(this, i18nc("@title:window", "Login Failed"),
                              i18nc("@info", "Authentication failed. Do you want to try again?"))
        == QMessageBox::Yes)
    {
        d->talker->link();
    }
}

void BOXWindow::slotSignalLinkingSucceeded()
{
    slotBusy(false);
    d->talker->listFolders();
}

void BOXWindow::slotListAlbumsFailed(const QString& msg)
{
    QMessageBox::critical(this, QString(), i18nc("@info", "Box call failed:\n%1", msg));
}

void BOXWindow::slotCreateFolderFailed(const QString& msg)
{
    QMessageBox::critical(this, QString(), i18nc("@info", "Box call failed:\n%1", msg));
}

void BOXWindow::slotCreateFolderSucceeded()
{
    d->talker->listFolders();
}

void BOXWindow::slotTransferCancel()
{
    d->transferQueue.clear();
    d->widget->progressBar()->hide();
    d->talker->cancel();
}

void BOXWindow::slotUserChangeRequest()
{
    slotSetUserName(QLatin1String(""));
    d->widget->getAlbumsCoB()->clear();
    d->talker->unLink();
    d->talker->link();
}

void BOXWindow::buttonStateChange(bool state)
{
    d->widget->getNewAlbmBtn()->setEnabled(state);
    d->widget->getReloadBtn()->setEnabled(state);
    startButton()->setEnabled(state);
}

void BOXWindow::slotFinished()
{
    writeSettings();
    d->widget->imagesList()->listView()->clear();
}

void BOXWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

} // namespace DigikamGenericBoxPlugin

#include "moc_boxwindow.cpp"
