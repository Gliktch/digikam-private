/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * SPDX-FileCopyrightText: 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2025 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imageshackwindow.h"

// Qt includes

#include <QWindow>
#include <QButtonGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QApplication>
#include <QIcon>
#include <QMenu>
#include <QComboBox>
#include <QMessageBox>
#include <QTimer>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "imageshacksession.h"
#include "imageshackwidget_p.h"
#include "imageshacktalker.h"
#include "dprogresswdg.h"
#include "wslogindialog.h"
#include "imageshacknewalbumdlg.h"

namespace DigikamGenericImageShackPlugin
{

class Q_DECL_HIDDEN ImageShackWindow::Private
{
public:

    Private() = default;

public:

    unsigned int           imagesCount      = 0;
    unsigned int           imagesTotal      = 0;
    QString                newAlbmTitle;

    QList<QUrl>            transferQueue;

    ImageShackSession*     session          = nullptr;
    ImageShackWidget*      widget           = nullptr;
    ImageShackTalker*      talker           = nullptr;

    ImageShackNewAlbumDlg* albumDlg         = nullptr;

    DInfoInterface*        iface            = nullptr;
};

ImageShackWindow::ImageShackWindow(DInfoInterface* const iface,
                                   QWidget* const /*parent*/)
    : WSToolDialog(nullptr, QLatin1String("ImageShack Dialog")),
      d           (new Private)
{
    d->session = new ImageShackSession();
    d->iface   = iface;
    d->widget  = new ImageShackWidget(this,
                                      d->session,
                                      d->iface,
                                      QLatin1String("ImageShack"));
    d->widget->setMinimumSize(700, 500);

    setMainWidget(d->widget);
    setWindowTitle(i18nc("@title:window", "Export to ImageShack"));
    setModal(false);

    d->albumDlg = new ImageShackNewAlbumDlg(this, QLatin1String("ImageShack"));

    connect(d->widget->d->chgRegCodeBtn, SIGNAL(clicked(bool)),
            this, SLOT(slotChangeRegistrantionCode()));

    startButton()->setText(i18nc("@action:button", "Upload"));
    startButton()->setToolTip(i18nc("@info:tooltip, button", "Start upload to ImageShack web service"));
    startButton()->setEnabled(false);

    connect(d->widget->d->imgList, SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    // -----------------------------------------------------------

    connect(this, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    d->talker = new ImageShackTalker(d->session);

    connect(d->talker, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(d->talker, SIGNAL(signalJobInProgress(int,int,QString)),
            this, SLOT(slotJobInProgress(int,int,QString)));

    connect(d->talker, SIGNAL(signalLoginDone(int,QString)),
            this, SLOT(slotLoginDone(int,QString)));

    connect(d->talker, SIGNAL(signalGetGalleriesDone(int,QString)),
            this, SLOT(slotGetGalleriesDone(int,QString)));

    connect(d->talker, SIGNAL(signalUpdateGalleries(QStringList,QStringList)),
            d->widget, SLOT(slotGetGalleries(QStringList,QStringList)));

    connect(d->talker, SIGNAL(signalAddPhotoDone(int,QString)),
            this, SLOT(slotAddPhotoDone(int,QString)));

    connect(d->widget, SIGNAL(signalReloadGalleries()),
            this, SLOT(slotGetGalleries()));

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotStartTransfer()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    connect(this, SIGNAL(cancelClicked()),
            this, SLOT(slotCancelClicked()));

    connect(d->widget->getNewAlbmBtn(), SIGNAL(clicked()),
            this, SLOT(slotNewAlbumRequest()));

    readSettings();

    QTimer::singleShot(20, this, SLOT(authenticate()));
}

ImageShackWindow::~ImageShackWindow()
{
    delete d->session;
    delete d;
}

void ImageShackWindow::slotImageListChanged()
{
    startButton()->setEnabled(!d->widget->d->imgList->imageUrls().isEmpty());
}

void ImageShackWindow::slotFinished()
{
    saveSettings();
    d->widget->d->progressBar->progressCompleted();
    d->widget->d->imgList->listView()->clear();
}

void ImageShackWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

void ImageShackWindow::readSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("ImageShack Settings"));

    if (group.readEntry("Private", false))
    {
        d->widget->d->privateImagesChb->setChecked(true);
    }

    if (group.readEntry("Rembar", false))
    {
        d->widget->d->remBarChb->setChecked(true);
    }
    else
    {
        d->widget->d->remBarChb->setChecked(false);
    }
}

void ImageShackWindow::saveSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("ImageShack Settings"));

    group.writeEntry("Private", d->widget->d->privateImagesChb->isChecked());
    group.writeEntry("Rembar",  d->widget->d->remBarChb->isChecked());
}

void ImageShackWindow::slotStartTransfer()
{
    d->widget->d->imgList->clearProcessedStatus();
    d->transferQueue = d->widget->d->imgList->imageUrls();

    if (d->transferQueue.isEmpty())
    {
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Transfer started!";

    d->imagesTotal = d->transferQueue.count();
    d->imagesCount = 0;

    d->widget->d->progressBar->setFormat(i18n("%v / %m"));
    d->widget->d->progressBar->setMaximum(d->imagesTotal);
    d->widget->d->progressBar->setValue(0);
    d->widget->d->progressBar->setVisible(true);
    d->widget->d->progressBar->progressScheduled(i18n("Image Shack Export"), false, true);
    d->widget->d->progressBar->progressThumbnailChanged(
        QIcon::fromTheme(QLatin1String("dk-imageshack")).pixmap(22, 22));

    uploadNextItem();
}

void ImageShackWindow::slotCancelClicked()
{
    d->talker->cancel();
    d->transferQueue.clear();
    d->widget->d->imgList->cancelProcess();
    d->widget->d->progressBar->setVisible(false);
    d->widget->d->progressBar->progressCompleted();
}

void ImageShackWindow::slotChangeRegistrantionCode()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Change registration code";
    authenticate();
}

void ImageShackWindow::authenticate()
{
    Q_EMIT signalBusy(true);

    d->widget->progressBar()->show();
    d->widget->d->progressBar->setValue(0);
    d->widget->d->progressBar->setMaximum(4);
    d->widget->progressBar()->setFormat(i18n("Authenticating..."));

    WSLoginDialog* const dlg = new WSLoginDialog(this, QLatin1String("ImageShack"));

    if (dlg->exec() == QDialog::Accepted)
    {
        d->session->setEmail(dlg->login());
        d->session->setPassword(dlg->password());
        d->talker->authenticate();
    }
}

void ImageShackWindow::slotBusy(bool val)
{
    if (val)
    {
        setCursor(Qt::WaitCursor);
        d->widget->d->chgRegCodeBtn->setEnabled(false);
        startButton()->setEnabled(false);
        setRejectButtonMode(QDialogButtonBox::Cancel);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        d->widget->d->chgRegCodeBtn->setEnabled(true);
        startButton()->setEnabled(d->session->loggedIn() &&
                                  !d->widget->imagesList()->imageUrls().isEmpty());
        setRejectButtonMode(QDialogButtonBox::Close);
    }
}

void ImageShackWindow::slotJobInProgress(int step, int maxStep, const QString& format)
{
    if (maxStep > 0)
    {
        d->widget->d->progressBar->setMaximum(maxStep);
    }

    d->widget->d->progressBar->setValue(step);

    if (!format.isEmpty())
    {
        d->widget->d->progressBar->setFormat(format);
    }
}

void ImageShackWindow::slotLoginDone(int errCode, const QString& errMsg)
{
    d->widget->updateLabels();

    if (!errCode && d->session->loggedIn())
    {
        d->session->saveSettings();
        startButton()->setEnabled(!d->widget->imagesList()->imageUrls().isEmpty());
        d->talker->getGalleries();
    }
    else
    {
        QMessageBox::critical(this, QString(), i18n("Login failed: %1\n", errMsg));
        startButton()->setEnabled(false);
        d->widget->d->progressBar->setVisible(false);
        slotBusy(false);
    }
}

void ImageShackWindow::slotGetGalleriesDone(int errCode, const QString& errMsg)
{
    slotBusy(false);
    d->widget->d->progressBar->setVisible(false);

    if (errCode)
    {
        QMessageBox::critical(this, QString(), i18n("Failed to get galleries list: %1\n", errMsg));
    }
}

void ImageShackWindow::uploadNextItem()
{
    if (d->transferQueue.isEmpty())
    {
        d->widget->d->progressBar->hide();
        return;
    }

    d->widget->d->imgList->processing(d->transferQueue.first());
    QString imgPath = d->transferQueue.first().toLocalFile();

    d->widget->d->progressBar->setMaximum(d->imagesTotal);
    d->widget->d->progressBar->setValue(d->imagesCount);

    QMap<QString, QString> opts;

    if (d->widget->d->privateImagesChb->isChecked())
    {
        opts[QLatin1String("public")] = QLatin1String("no");
    }

    if (d->widget->d->remBarChb->isChecked())
    {
        opts[QLatin1String("rembar")] = QLatin1String("yes");
    }

    // tags

    if (!d->widget->d->tagsFld->text().isEmpty())
    {
        QString str = d->widget->d->tagsFld->text();
        QStringList tagsList;
        tagsList = str.split(QRegularExpression(QLatin1String("\\W+")), Qt::SkipEmptyParts);
        opts[QLatin1String("tags")] = tagsList.join(QLatin1Char(','));
    }

    opts[QLatin1String("auth_token")] = d->session->authToken();

    int gidx = d->widget->d->galleriesCob->currentIndex();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Album ID is "<< d->widget->d->galleriesCob->itemData(gidx).toString();

    switch (gidx)
    {
        case 0:
        {
            d->talker->uploadItem(imgPath, opts);
            break;
        }

        case 1:
        {
            opts[QLatin1String("album")] = d->newAlbmTitle;
            d->talker->uploadItemToGallery(imgPath, d->newAlbmTitle, opts);
            break;
        }

        default:
        {
            opts[QLatin1String("album")] = d->widget->d->galleriesCob->itemData(gidx).toString();
            d->talker->uploadItemToGallery(imgPath, d->widget->d->galleriesCob->itemData(gidx).toString(), opts);
        }
    }
}

void ImageShackWindow::slotAddPhotoDone(int errCode, const QString& errMsg)
{
    d->widget->d->imgList->processed(d->transferQueue.first(), (errCode == 0));

    if (!errCode)
    {
        d->widget->imagesList()->removeItemByUrl(d->transferQueue.first());
        d->transferQueue.removeFirst();
        d->imagesCount++;
    }
    else
    {
        if (QMessageBox::question(this, i18nc("@title:window", "Uploading Failed"),
                                  i18n("Failed to upload photo into ImageShack: %1\n"
                                       "Do you want to continue?", errMsg))
            != QMessageBox::Yes)
        {
            d->widget->d->progressBar->setVisible(false);
            d->transferQueue.clear();
            return;
        }
    }

    uploadNextItem();
}

void ImageShackWindow::slotGetGalleries()
{
    d->widget->d->progressBar->setVisible(true);
    d->talker->getGalleries();
}

void ImageShackWindow::slotNewAlbumRequest()
{
    if (d->albumDlg->exec() == QDialog::Accepted)
    {
        d->newAlbmTitle = d->albumDlg->getAlbumTitle();
    }
}

} // namespace DigikamGenericImageShackPlugin

#include "moc_imageshackwindow.cpp"
