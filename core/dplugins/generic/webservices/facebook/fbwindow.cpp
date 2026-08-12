/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-17-26
 * Description : a tool to export items to Facebook web service
 *
 * SPDX-FileCopyrightText: 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fbwindow.h"

// Qt includes

#include <QWindow>
#include <QComboBox>
#include <QMenu>
#include <QFileInfo>
#include <QCheckBox>
#include <QCloseEvent>
#include <QSpinBox>
#include <QMessageBox>
#include <QPointer>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "dmetadata.h"
#include "ditemslist.h"
#include "digikam_version.h"
#include "dprogresswdg.h"
#include "wstoolutils.h"
#include "fbitem.h"
#include "fbtalker.h"
#include "fbwidget.h"
#include "fbnewalbumdlg.h"
#include "previewloadthread.h"

namespace DigikamGenericFaceBookPlugin
{

class Q_DECL_HIDDEN FbWindow::Private
{
public:

    explicit Private(QWidget* const parent, DInfoInterface* const interface)
    {
        iface           = interface;
        widget          = new FbWidget(parent, iface, QLatin1String("Facebook"));
        imgList         = widget->imagesList();
        progressBar     = widget->progressBar();
        changeUserBtn   = widget->getChangeUserBtn();
        albumsCoB       = widget->getAlbumsCoB();
        newAlbumBtn     = widget->getNewAlbmBtn();
        reloadAlbumsBtn = widget->getReloadBtn();
        resizeChB       = widget->getResizeCheckBox();
        dimensionSpB    = widget->getDimensionSpB();
        imageQualitySpB = widget->getImgQualitySpB();
        imagesCount     = 0;
        imagesTotal     = 0;
        talker          = nullptr;
        albumDlg        = nullptr;
    }

    FbWidget*       widget;
    DItemsList*     imgList;
    QPushButton*    changeUserBtn;
    QComboBox*      albumsCoB;
    QPushButton*    newAlbumBtn;
    QPushButton*    reloadAlbumsBtn;
    QCheckBox*      resizeChB;
    QSpinBox*       dimensionSpB;
    QSpinBox*       imageQualitySpB;
    DProgressWdg*   progressBar;

    unsigned int    imagesCount;
    unsigned int    imagesTotal;
    QString         tmpDir;
    QString         tmpPath;

    QString         profileAID;
    QString         currentAlbumID;

    QList<QUrl>     transferQueue;

    FbTalker*       talker;
    FbNewAlbumDlg*  albumDlg;

    DInfoInterface* iface;
};

FbWindow::FbWindow(DInfoInterface* const iface, QWidget* const /*parent*/)
    : WSToolDialog(nullptr, QLatin1String("Facebook Export Dialog")),
      d           (new Private(this, iface))
{
    d->tmpPath.clear();
    d->tmpDir = WSToolUtils::makeTemporaryDir("facebook").absolutePath() + QLatin1Char('/');

    setMainWidget(d->widget);
    setModal(false);

    setWindowTitle(i18nc("@title:window", "Export to Facebook Web Service"));

    startButton()->setText(i18nc("@action:button", "Start Upload"));
    startButton()->setToolTip(i18nc("@info:tooltip, button", "Start upload to Facebook web service"));

    d->widget->setMinimumSize(700, 500);

    d->changeUserBtn->setStyleSheet(QLatin1String("QPushButton {background-color: "
                                                  "#3b5998; color: #ffffff;}"));
    d->changeUserBtn->setIcon(QIcon::fromTheme(QLatin1String("dk-facebook-white")));
    d->changeUserBtn->setText(i18n("Continue with Facebook"));

    // ------------------------------------------------------------------------

    connect(d->imgList, SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    connect(d->changeUserBtn, SIGNAL(clicked()),
            this, SLOT(slotUserChangeRequest()));

    connect(d->newAlbumBtn, SIGNAL(clicked()),
            this, SLOT(slotNewAlbumRequest()));

    connect(d->widget, SIGNAL(reloadAlbums(long long)),
            this, SLOT(slotReloadAlbumsRequest(long long)));

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotStartTransfer()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    connect(this, SIGNAL(cancelClicked()),
            this, SLOT(slotCancelClicked()));

    d->albumDlg = new FbNewAlbumDlg(this, QLatin1String("Facebook"));

    // ------------------------------------------------------------------------

    d->talker = new FbTalker(this);

    connect(d->talker, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(d->talker, SIGNAL(signalLoginProgress(int,int,QString)),
            this, SLOT(slotLoginProgress(int,int,QString)));

    connect(d->talker, SIGNAL(signalLoginDone(int,QString)),
            this, SLOT(slotLoginDone(int,QString)));

    connect(d->talker, SIGNAL(signalAddPhotoDone(int,QString)),
            this, SLOT(slotAddPhotoDone(int,QString)));

    connect(d->talker, SIGNAL(signalCreateAlbumDone(int,QString,QString)),
            this, SLOT(slotCreateAlbumDone(int,QString,QString)));

    connect(d->talker, SIGNAL(signalListAlbumsDone(int,QString,QList<FbAlbum>)),
            this, SLOT(slotListAlbumsDone(int,QString,QList<FbAlbum>)));

    connect(d->progressBar, SIGNAL(signalProgressCanceled()),
            this, SLOT(slotStopAndCloseProgressBar()));

    // ------------------------------------------------------------------------

    readSettings();
    buttonStateChange(false);

    authenticate(false);
}

FbWindow::~FbWindow()
{
    WSToolUtils::removeTemporaryDir("facebook");

    delete d->albumDlg;
    delete d->talker;
    delete d;
}

void FbWindow::slotStopAndCloseProgressBar()
{
    // Cancel the operation
    slotCancelClicked();

    // Write settings and tidy up
    slotFinished();

    // Close the dialog
    reject();
}

void FbWindow::slotFinished()
{
    writeSettings();
    d->imgList->listView()->clear();
    d->progressBar->progressCompleted();
}

void FbWindow::slotCancelClicked()
{
    setRejectButtonMode(QDialogButtonBox::Close);
    d->talker->cancel();
    d->transferQueue.clear();
    d->imgList->cancelProcess();
    d->progressBar->hide();
    d->progressBar->progressCompleted();
}

void FbWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

void FbWindow::readSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(QLatin1String("Facebook Settings"));

    if (grp.readEntry("Resize", false))
    {
        d->resizeChB->setChecked(true);
        d->dimensionSpB->setEnabled(true);
    }
    else
    {
        d->resizeChB->setChecked(false);
        d->dimensionSpB->setEnabled(false);
    }

    d->currentAlbumID = grp.readEntry("Current Album", QString());
    d->dimensionSpB->setValue(grp.readEntry("Maximum Width", 1600));
    d->imageQualitySpB->setValue(grp.readEntry("Image Quality", 85));
}

void FbWindow::writeSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(QLatin1String("Facebook Settings"));

    grp.writeEntry("Current Album", d->currentAlbumID);
    grp.writeEntry("Resize",        d->resizeChB->isChecked());
    grp.writeEntry("Maximum Width", d->dimensionSpB->value());
    grp.writeEntry("Image Quality", d->imageQualitySpB->value());
}

void FbWindow::authenticate(bool forceLogin)
{
    d->progressBar->show();
    d->progressBar->setFormat(QLatin1String(""));
    setRejectButtonMode(QDialogButtonBox::Cancel);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Calling Login method ";

    if (forceLogin)
    {
        d->talker->link();
    }
    else
    {
        d->talker->readSettings();
    }
}

void FbWindow::slotLoginProgress(int step, int maxStep, const QString& label)
{
    DProgressWdg* const progressBar = d->progressBar;

    if (!label.isEmpty())
    {
        progressBar->setFormat(label);
    }

    if (maxStep > 0)
    {
        progressBar->setMaximum(maxStep);
    }

    progressBar->setValue(step);
}

void FbWindow::slotLoginDone(int errCode, const QString& errMsg)
{
    setRejectButtonMode(QDialogButtonBox::Close);
    d->progressBar->hide();

    buttonStateChange(d->talker->linked());

    if (d->talker->linked())
    {
        d->changeUserBtn->setText(i18n("Log Out of Facebook"));
    }
    else
    {
        d->changeUserBtn->setText(i18n("Continue with Facebook"));
    }

    FbUser user = d->talker->getUser();
    setProfileAID(user.id.toLongLong());

    d->widget->updateLabels(user.name, user.profileURL);
    d->albumsCoB->clear();

    if (errCode == 0 && d->talker->linked())
    {
        d->albumsCoB->addItem(i18n("<i>auto create</i>"), QString());
        d->talker->listAlbums();    // get albums to fill combo box
    }
    else if (errCode > 0)
    {
        QMessageBox::critical(this, QString(), i18n("Facebook Call Failed: %1\n", errMsg));
    }
}

void FbWindow::slotListAlbumsDone(int errCode,
                                  const QString& errMsg,
                                  const QList<FbAlbum>& albumsList)
{
    QString albumDebug = QLatin1String("");

    for (const FbAlbum& album : std::as_const(albumsList))
    {
        albumDebug.append(QString::fromLatin1("%1: %2\n").arg(album.id).arg(album.title));
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Received albums (errCode = " << errCode << ", errMsg = "
                                     << errMsg << "): " << albumDebug;

    if (errCode != 0)
    {
        QMessageBox::critical(this, QString(), i18n("Facebook Call Failed: %1\n", errMsg));
        return;
    }

    d->albumsCoB->clear();
    d->albumsCoB->addItem(i18n("<i>auto create</i>"), QString());

    for (int i = 0 ; i < albumsList.size() ; ++i)
    {
        QString albumIcon;

        switch (albumsList.at(i).privacy)
        {
            case FB_ME:
                albumIcon = QLatin1String("secure-card");
                break;

            case FB_FRIENDS:
                albumIcon = QLatin1String("user-identity");
                break;

            case FB_FRIENDS_OF_FRIENDS:
                albumIcon = QLatin1String("system-users");
                break;

            case FB_EVERYONE:
                albumIcon = QLatin1String("folder-html");
                break;

            case FB_CUSTOM:
                albumIcon = QLatin1String("configure");
                break;
        }

        d->albumsCoB->addItem(QIcon::fromTheme(albumIcon),
                              albumsList.at(i).title,
                              albumsList.at(i).id);

        if (d->currentAlbumID == albumsList.at(i).id)
        {
            d->albumsCoB->setCurrentIndex(i + 1);
        }
    }
}

void FbWindow::buttonStateChange(bool state)
{
    d->newAlbumBtn->setEnabled(state);
    d->reloadAlbumsBtn->setEnabled(state);
    startButton()->setEnabled(state);
}

void FbWindow::slotBusy(bool val)
{
    if (val)
    {
        setCursor(Qt::WaitCursor);
        d->changeUserBtn->setEnabled(false);
        buttonStateChange(false);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        d->changeUserBtn->setEnabled(true);
        buttonStateChange(d->talker->linked());
    }
}

void FbWindow::slotUserChangeRequest()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Slot Change User Request";

    if (d->talker->linked())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Slot User Logout";
        d->talker->logout();
    }
    else
    {
        authenticate(true);
    }
}

void FbWindow::slotReloadAlbumsRequest(long long userID)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Reload Albums Request for UID:" << userID;

    if (userID == 0)
    {
        FbUser user = d->talker->getUser();
        setProfileAID(user.id.toLongLong());
        d->talker->listAlbums(); // re-get albums from current user
    }
    else
    {
        setProfileAID(userID);
        d->talker->listAlbums(userID); // re-get albums for friend
    }
}

void FbWindow::slotNewAlbumRequest()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Slot New Album Request";

    if (d->albumDlg->exec() == QDialog::Accepted)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Calling New Album method";
        FbAlbum newAlbum;
        d->albumDlg->getAlbumProperties(newAlbum);
        d->talker->createAlbum(newAlbum);
    }
}

void FbWindow::slotStartTransfer()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotStartTransfer invoked";

    d->imgList->clearProcessedStatus();
    d->transferQueue  = d->imgList->imageUrls();

    if (d->transferQueue.isEmpty())
    {
        return;
    }

    d->currentAlbumID = d->albumsCoB->itemData(d->albumsCoB->currentIndex()).toString();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "upload request got album id from widget: " << d->currentAlbumID;
    d->imagesTotal    = d->transferQueue.count();
    d->imagesCount    = 0;

    setRejectButtonMode(QDialogButtonBox::Cancel);
    d->progressBar->setFormat(i18n("%v / %m"));
    d->progressBar->setMaximum(d->imagesTotal);
    d->progressBar->setValue(0);
    d->progressBar->show();
    d->progressBar->progressScheduled(i18n("Facebook export"), true, true);
    d->progressBar->progressThumbnailChanged(QIcon::fromTheme(QLatin1String("dk-facebook")).pixmap(22, 22));

    uploadNextPhoto();
}

void FbWindow::setProfileAID(long long userID)
{
    // store AID of Profile Photos album
    // wiki.developers.facebook.com/index.php/Profile_archive_album

    d->profileAID = QString::number((userID << 32) + (-3 & 0xFFFFFFFF));
}

QString FbWindow::getImageCaption(const QString& fileName)
{
    DItemInfo info(d->iface->itemInfo(QUrl::fromLocalFile(fileName)));

    // Facebook doesn't support image titles. Include it in descriptions if needed.

    QStringList descriptions = QStringList() << info.title() << info.comment();
    descriptions.removeAll(QLatin1String(""));

    return descriptions.join(QLatin1String("\n\n"));
}

bool FbWindow::prepareImageForUpload(const QString& imgPath, QString& caption)
{
    QImage image = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();

    if (image.isNull())
    {
        image.load(imgPath);
    }

    if (image.isNull())
    {
        return false;
    }

    // get temporary file name

    d->tmpPath = d->tmpDir + QFileInfo(imgPath).baseName().trimmed() + QLatin1String(".jpg");

    // rescale image if requested

    int maxDim = d->dimensionSpB->value();

    if (d->resizeChB->isChecked() &&
        (image.width() > maxDim || image.height() > maxDim))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Resizing to " << maxDim;
        image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Saving to temp file: " << d->tmpPath;
    image.save(d->tmpPath, "JPEG", d->imageQualitySpB->value());

    // copy meta data to temporary image

    QScopedPointer<DMetadata> meta(new DMetadata);

    if (meta->load(imgPath))
    {
        caption = getImageCaption(imgPath);
        meta->setItemDimensions(image.size());
        meta->setItemOrientation(MetaEngine::ORIENTATION_NORMAL);
        meta->setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);
        meta->save(d->tmpPath, true);
    }
    else
    {
        caption.clear();
    }

    return true;
}

void FbWindow::uploadNextPhoto()
{
    if (d->transferQueue.isEmpty())
    {
        setRejectButtonMode(QDialogButtonBox::Close);
        d->progressBar->hide();
        d->progressBar->progressCompleted();
        return;
    }

    d->imgList->processing(d->transferQueue.first());
    QString imgPath = d->transferQueue.first().toLocalFile();

    d->progressBar->setMaximum(d->imagesTotal);
    d->progressBar->setValue(d->imagesCount);

    QString caption;

    if (d->resizeChB->isChecked())
    {
        if (!prepareImageForUpload(imgPath, caption))
        {
            slotAddPhotoDone(666, i18n("Cannot open file"));
            return;
        }

        d->talker->addPhoto(d->tmpPath, d->currentAlbumID, caption);
    }
    else
    {
        caption = getImageCaption(imgPath);
        d->tmpPath.clear();
        d->talker->addPhoto(imgPath, d->currentAlbumID, caption);
    }
}

void FbWindow::slotAddPhotoDone(int errCode, const QString& errMsg)
{
    // Remove temporary file if it was used

    if (!d->tmpPath.isEmpty())
    {
        QFile::remove(d->tmpPath);
        d->tmpPath.clear();
    }

    d->imgList->processed(d->transferQueue.first(), (errCode == 0));

    if (errCode == 0)
    {
        d->transferQueue.removeFirst();
        d->imagesCount++;
    }
    else
    {
        if (QMessageBox::question(this, i18nc("@title:window", "Uploading Failed"),
                                  i18n("Failed to upload photo into Facebook: %1\n"
                                       "Do you want to continue?", errMsg))
            != QMessageBox::Yes)
        {
            setRejectButtonMode(QDialogButtonBox::Close);
            d->progressBar->hide();
            d->progressBar->progressCompleted();
            d->transferQueue.clear();
            return;
        }
    }

    uploadNextPhoto();
}

void FbWindow::slotCreateAlbumDone(int errCode, const QString& errMsg, const QString& newAlbumID)
{
    if (errCode != 0)
    {
        QMessageBox::critical(this, QString(), i18n("Facebook Call Failed: %1", errMsg));
        return;
    }

    // reload album list and automatically select new album

    d->currentAlbumID = newAlbumID;
    d->talker->listAlbums();
}

void FbWindow::slotImageListChanged()
{
    startButton()->setEnabled(!(d->imgList->imageUrls().isEmpty()));
}

} // namespace DigikamGenericFaceBookPlugin

#include "moc_fbwindow.cpp"
