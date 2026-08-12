/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter (x.com) social network
 *
 * SPDX-FileCopyrightText: 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "twitterwindow.h"

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
#include "twittertalker.h"
#include "twitteritem.h"
#include "twitternewalbumdlg.h"
#include "twitterwidget.h"

namespace DigikamGenericTwitterPlugin
{

class Q_DECL_HIDDEN TwWindow::Private
{
public:

    Private() = default;

public:

    unsigned int   imagesCount          = 0;
    unsigned int   imagesTotal          = 0;

    TwWidget*      widget               = nullptr;
    TwNewAlbumDlg* albumDlg             = nullptr;
    TwTalker*      talker               = nullptr;

    QString        currentAlbumName;
    QList<QUrl>    transferQueue;
};

TwWindow::TwWindow(DInfoInterface* const iface,
                   QWidget* const /*parent*/)
    : WSToolDialog(nullptr, QLatin1String("Twitter Export Dialog")),
      d           (new Private)
{
    d->widget      = new TwWidget(this, iface, QLatin1String("Twitter"));

    d->widget->imagesList()->setIface(iface);

    setMainWidget(d->widget);
    setModal(false);
    setWindowTitle(i18nc("@title:window", "Export to x.com (aka Twitter)"));

    startButton()->setText(i18nc("@action:button", "Start Upload"));
    startButton()->setToolTip(i18nc("@info:tooltip, button", "Start upload to Twitter"));

    d->widget->setMinimumSize(700, 500);

    connect(d->widget->imagesList(), SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    connect(d->widget->getChangeUserBtn(), SIGNAL(clicked()),
            this, SLOT(slotUserChangeRequest()));

    connect(d->widget->getNewAlbmBtn(), SIGNAL(clicked()),
            this, SLOT(slotNewAlbumRequest()));
/*
    connect(d->widget->getReloadBtn(), SIGNAL(clicked()),
            this, SLOT(slotReloadAlbumsRequest()));
*/
    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotStartTransfer()));

    d->albumDlg = new TwNewAlbumDlg(this, QLatin1String("Twitter"));
    d->talker   = new TwTalker(this);

    connect(d->talker,SIGNAL(signalBusy(bool)),
            this,SLOT(slotBusy(bool)));

    connect(d->talker,SIGNAL(signalLinkingFailed()),
            this,SLOT(slotSignalLinkingFailed()));

    connect(d->talker,SIGNAL(signalLinkingSucceeded()),
            this,SLOT(slotSignalLinkingSucceeded()));

    connect(d->talker,SIGNAL(signalSetUserName(QString)),
            this,SLOT(slotSetUserName(QString)));

    connect(d->talker,SIGNAL(signalListAlbumsFailed(QString)),
            this,SLOT(slotListAlbumsFailed(QString)));

    connect(d->talker,SIGNAL(signalListAlbumsDone(QList<QPair<QString,QString> >)), // krazy:exclude=normalize
            this,SLOT(slotListAlbumsDone(QList<QPair<QString,QString> >)));         // krazy:exclude=normalize

    connect(d->talker,SIGNAL(signalCreateFolderFailed(QString)),
            this,SLOT(slotCreateFolderFailed(QString)));

    connect(d->talker,SIGNAL(signalCreateFolderSucceeded()),
            this,SLOT(slotCreateFolderSucceeded()));

    connect(d->talker,SIGNAL(signalAddPhotoFailed(QString)),
            this,SLOT(slotAddPhotoFailed(QString)));

    connect(d->talker,SIGNAL(signalAddPhotoSucceeded()),
            this,SLOT(slotAddPhotoSucceeded()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    readSettings();
    buttonStateChange(false);

    d->talker->link();
}

TwWindow::~TwWindow()
{
    delete d->widget;
    delete d->albumDlg;
    delete d->talker;
    delete d;
}

void TwWindow::readSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(QLatin1String("Twitter Settings"));
    d->currentAlbumName     = grp.readEntry("Current Album", QString());
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "readsettings:" << d->currentAlbumName;

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

void TwWindow::writeSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(QLatin1String("Twitter Settings"));

    grp.writeEntry("Current Album", d->currentAlbumName);
    grp.writeEntry("Resize",        d->widget->getResizeCheckBox()->isChecked());
    grp.writeEntry("Maximum Width", d->widget->getDimensionSpB()->value());
    grp.writeEntry("Image Quality", d->widget->getImgQualitySpB()->value());
}

void TwWindow::reactivate()
{
    d->widget->imagesList()->loadImagesFromCurrentSelection();
    d->widget->progressBar()->hide();

    show();
}

void TwWindow::setItemsList(const QList<QUrl>& urls)
{
    d->widget->imagesList()->slotAddImages(urls);
}

void TwWindow::slotBusy(bool val)
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

void TwWindow::slotSetUserName(const QString& msg)
{
    d->widget->updateLabels(msg, QLatin1String(""));
}

void TwWindow::slotListAlbumsDone(const QList<QPair<QString,QString> >& list)
{
    d->widget->getAlbumsCoB()->clear();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotListAlbumsDone:" << list.size();

    for (int i = 0 ; i < list.size() ; ++i)
    {
        d->widget->getAlbumsCoB()->addItem(QIcon::fromTheme(QLatin1String("system-users")),
        list.value(i).second, list.value(i).first);
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotListAlbumsDone:" << list.value(i).second << " " << list.value(i).first;
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotListAlbumsDone:" <<d->currentAlbumName;

        if (d->currentAlbumName == list.value(i).first)
        {
            d->widget->getAlbumsCoB()->setCurrentIndex(i);
        }
    }

    buttonStateChange(true);
    d->talker->getUserName();
}

void TwWindow::slotStartTransfer()
{
    d->widget->imagesList()->clearProcessedStatus();

    if (d->widget->imagesList()->imageUrls().isEmpty())
    {
        QMessageBox::critical(this, i18nc("@title: window start transfer", "Error"),
                              i18n("No image selected. Please select which images should be uploaded."));
        return;
    }

    if (!(d->talker->authenticated()))
    {
        QPointer<QMessageBox> warn = new QMessageBox(QMessageBox::Warning,
                         i18nc("@title: window start transfer", "Warning"),
                         i18n("Authentication failed. Click \"Continue\" to authenticate."),
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

    d->widget->progressBar()->setFormat(i18n("%v / %m"));
    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(0);
    d->widget->progressBar()->show();
    d->widget->progressBar()->progressScheduled(i18n("Twitter export"), true, true);
    d->widget->progressBar()->progressThumbnailChanged(QIcon::fromTheme(QLatin1String("twitter")).pixmap(22, 22));

    uploadNextPhoto();
}

void TwWindow::uploadNextPhoto()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "uploadNextPhoto:" << d->transferQueue.count();

    if (d->transferQueue.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "empty";
        d->widget->progressBar()->progressCompleted();
        return;
    }

    QString imgPath = d->transferQueue.first().toLocalFile();
    QString temp = d->currentAlbumName + QLatin1Char('/');

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

void TwWindow::slotAddPhotoFailed(const QString& msg)
{
    if (QMessageBox::question(this, i18nc("@title:window", "Uploading Failed"),
                              i18n("Failed to upload photo to Twitter."
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

void TwWindow::slotAddPhotoSucceeded()
{
    // Remove photo uploaded from the list

    d->widget->imagesList()->removeItemByUrl(d->transferQueue.first());
    d->transferQueue.pop_front();
    d->imagesCount++;
    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(d->imagesCount);
    uploadNextPhoto();
}

void TwWindow::slotImageListChanged()
{
    startButton()->setEnabled(!(d->widget->imagesList()->imageUrls().isEmpty()));
}

void TwWindow::slotNewAlbumRequest()
{
    if (d->albumDlg->exec() == QDialog::Accepted)
    {
        TwAlbum newAlbum;
        d->albumDlg->getAlbumProperties(newAlbum);
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotNewAlbumRequest:" << newAlbum.title;
        d->currentAlbumName = d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString();
        QString temp = d->currentAlbumName + newAlbum.title;
        Q_UNUSED(temp);
/*
        d->talker->createFolder(temp);
*/
    }
}

/*
void TwWindow::slotReloadAlbumsRequest()
{
  //  d->talker->listFolders();
}
*/

void TwWindow::slotSignalLinkingFailed()
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

void TwWindow::slotSignalLinkingSucceeded()
{
/*
    d->talker->listFolders();
*/
    slotBusy(false);
}

void TwWindow::slotListAlbumsFailed(const QString& msg)
{
    QMessageBox::critical(this, QString(), i18n("Twitter call failed:\n%1", msg));
}

void TwWindow::slotCreateFolderFailed(const QString& msg)
{
    QMessageBox::critical(this, QString(), i18n("Twitter call failed:\n%1", msg));
}

void TwWindow::slotCreateFolderSucceeded()
{
/*
    d->talker->listFolders();
*/
}

void TwWindow::slotTransferCancel()
{
    d->transferQueue.clear();
    d->widget->progressBar()->hide();
    d->talker->cancel();
}

void TwWindow::slotUserChangeRequest()
{
    slotSetUserName(QLatin1String(""));
    d->widget->getAlbumsCoB()->clear();
    d->talker->unLink();
    d->talker->link();
}

void TwWindow::buttonStateChange(bool state)
{
    d->widget->getNewAlbmBtn()->setEnabled(state);
    d->widget->getReloadBtn()->setEnabled(state);
    startButton()->setEnabled(state);
}

void TwWindow::slotFinished()
{
    writeSettings();
    d->widget->imagesList()->listView()->clear();
}

void TwWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

} // namespace DigikamGenericTwitterPlugin

#include "moc_twitterwindow.cpp"
