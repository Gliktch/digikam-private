/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-30
 * Description : a tool to export items to Piwigo web service
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006      by Colin Guthrie <kde at colin dot guthr dot ie>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008      by Andrea Diamantini <adjam7 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Frederic Coiffier <frederic dot coiffier at free dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "piwigowindow.h"

// Qt includes

#include <QCheckBox>
#include <QDialog>
#include <QFileInfo>
#include <QBuffer>
#include <QImage>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeWidgetItem>
#include <QPointer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextStream>
#include <QFile>
#include <QApplication>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "piwigosession.h"
#include "piwigologindlg.h"
#include "piwigotalker.h"
#include "imagedialog.h"
#include "ditemslist.h"
#include "dprogresswdg.h"

namespace DigikamGenericPiwigoPlugin
{

class Q_DECL_HIDDEN PiwigoWindow::Private
{
public:

    explicit Private(PiwigoWindow* const parent,
                     DInfoInterface* const interface);

public:

    QWidget*                       widget           = nullptr;

    QTreeWidget*                   albumView        = nullptr;

    QPushButton*                   confButton       = nullptr;
    QPushButton*                   reloadButton     = nullptr;

    QCheckBox*                     resizeCheckBox   = nullptr;
    QSpinBox*                      widthSpinBox     = nullptr;
    QSpinBox*                      heightSpinBox    = nullptr;
    QSpinBox*                      qualitySpinBox   = nullptr;

    QHash<QString, PiwigoAlbum>    albumDict;

    PiwigoTalker*                  talker           = nullptr;
    PiwigoSession*                 pPiwigo          = nullptr;
    DInfoInterface*                iface            = nullptr;
    DItemsList*                    imageList        = nullptr;
    DProgressWdg*                  progressBar      = nullptr;

    unsigned int                   uploadCount      = 0;
    unsigned int                   uploadTotal      = 0;
    QStringList                    pUploadList;
    QString                        currPhotoPath;

    QLabel*                        userNameLbl      = nullptr;
    QLabel*                        userName         = nullptr;
    QLabel*                        urlLbl           = nullptr;
    QLabel*                        url              = nullptr;
};

PiwigoWindow::Private::Private(PiwigoWindow* const parent,
                               DInfoInterface* const interface)
    : widget(new QWidget(parent)),
      iface (interface)
{
    parent->setMainWidget(widget);
    parent->setModal(false);

    QHBoxLayout* const hlay = new QHBoxLayout(widget);

    // ---------------------------------------------------------------------------

    QLabel* const logo = new QLabel();
    logo->setContentsMargins(QMargins());
    logo->setScaledContents(false);
    logo->setOpenExternalLinks(true);
    logo->setTextFormat(Qt::RichText);
    logo->setFocusPolicy(Qt::NoFocus);
    logo->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    logo->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    logo->setToolTip(i18n("Visit Piwigo website"));
    logo->setAlignment(Qt::AlignLeft);
    QImage img = QImage(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                               QLatin1String("digikam/data/logo-piwigo.webp")));

    QByteArray byteArray;
    QBuffer    buffer(&byteArray);
    img.save(&buffer, "PNG");
    logo->setText(QString::fromLatin1("<a href=\"%1\">%2</a>")
                  .arg(QLatin1String("https://piwigo.org"))
                  .arg(QString::fromLatin1("<img src=\"data:image/png;base64,%1\">")
                  .arg(QLatin1String(byteArray.toBase64().data()))));

    // ---------------------------------------------------------------------------

    imageList              = new DItemsList(nullptr);
    imageList->setObjectName(QLatin1String("Piwigo ImagesList"));
    imageList->setIface(iface);
    imageList->listView()->clear();
    imageList->loadImagesFromCurrentSelection();

    progressBar            = new DProgressWdg(imageList);
    progressBar->setMaximum(0);
    progressBar->reset();
    imageList->appendControlButtonsWidget(progressBar);
    QBoxLayout* const blay = imageList->setControlButtonsPlacement(DItemsList::ControlButtonsBelow);
    blay->setStretchFactor(progressBar, 20);
    progressBar->hide();

    // ---------------------------------------------------------------------------

    albumView                   = new QTreeWidget;
    QStringList labels;
    labels << i18n("Remote Albums");
    albumView->setHeaderLabels(labels);
    albumView->setSortingEnabled(true);
    albumView->sortByColumn(0, Qt::AscendingOrder);

    reloadButton                = new QPushButton();
    reloadButton->setText(i18nc("reload album list", "Reload"));
    reloadButton->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
    reloadButton->setToolTip(i18n("Reload album list"));
    reloadButton->setEnabled(false);

    // ---------------------------------------------------------------------------

    QFrame* const optionFrame   = new QFrame;
    QVBoxLayout* const vlay     = new QVBoxLayout();

    userNameLbl                 = new QLabel(i18nc("piwigo account settings", "Name:"), optionFrame);
    userName                    = new QLabel(optionFrame);
    urlLbl                      = new QLabel(i18nc("piwigo url settings", "Url:"), optionFrame);
    url                         = new QLabel(optionFrame);
    url->setOpenExternalLinks(true);
    url->setFocusPolicy(Qt::NoFocus);

    confButton                  = new QPushButton;
    confButton->setText(i18n("Change Account"));
    confButton->setIcon(QIcon::fromTheme(QLatin1String("system-switch-user")));
    confButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QGroupBox* const optionsBox = new QGroupBox(i18n("Options"));
    QVBoxLayout* const vlay2    = new QVBoxLayout();

    resizeCheckBox              = new QCheckBox(optionsBox);
    resizeCheckBox->setText(i18n("Resize photos before uploading"));

    QGridLayout* const glay     = new QGridLayout;
    QLabel* const widthLabel    = new QLabel(i18n("Maximum width:"));

    widthSpinBox                = new QSpinBox;
    widthSpinBox->setRange(1,8000);
    widthSpinBox->setValue(1600);

    QLabel* const heightLabel   = new QLabel(i18n("Maximum height:"));

    heightSpinBox               = new QSpinBox;
    heightSpinBox->setRange(1,8000);
    heightSpinBox->setValue(1600);

    QLabel* const qualityLabel  = new QLabel(i18n("Resized JPEG quality:"));

    qualitySpinBox              = new QSpinBox;
    qualitySpinBox->setRange(1,100);
    qualitySpinBox->setValue(95);

    resizeCheckBox->setChecked(false);
    widthSpinBox->setEnabled(false);
    heightSpinBox->setEnabled(false);
    qualitySpinBox->setEnabled(false);

    // ---------------------------------------------------------------------------

    glay->addWidget(widthLabel,     0, 0);
    glay->addWidget(widthSpinBox,   0, 1);
    glay->addWidget(heightLabel,    1, 0);
    glay->addWidget(heightSpinBox,  1, 1);
    glay->addWidget(qualityLabel,   2, 0);
    glay->addWidget(qualitySpinBox, 2, 1);

    // ---------------------------------------------------------------------------

    vlay2->addWidget(resizeCheckBox);
    vlay2->addLayout(glay);
    vlay2->addStretch(0);

    optionsBox->setLayout(vlay2);

    // ---------------------------------------------------------------------------

    vlay->addWidget(logo);
    vlay->addWidget(userNameLbl);
    vlay->addWidget(userName);
    vlay->addWidget(urlLbl);
    vlay->addWidget(url);
    vlay->addWidget(confButton);
    vlay->addWidget(albumView);
    vlay->addWidget(reloadButton);
    vlay->addWidget(optionsBox);

    optionFrame->setLayout(vlay);

    // ---------------------------------------------------------------------------

    hlay->addWidget(imageList);
    hlay->addWidget(optionFrame);

    widget->setLayout(hlay);
}

// ---------------------------------------------------------------------------

PiwigoWindow::PiwigoWindow(DInfoInterface* const iface,
                           QWidget* const /*parent*/)
    : WSToolDialog(nullptr, QLatin1String("PiwigoSync Dialog")),
      d           (new Private(this, iface))
{
    d->pPiwigo = new PiwigoSession();

    setWindowTitle(i18nc("@title:window", "Piwigo Export"));
    setModal(false);

    // "Start Upload" button

    startButton()->setText(i18nc("@action:button", "Start Upload"));
    startButton()->setEnabled(false);

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotAddPhoto()));

    // we need to let d->talker work..

    d->talker  = new PiwigoTalker(iface, d->widget);

    // connect functions

    connectSignals();

    KSharedConfigPtr config = KSharedConfig::openConfig();

    if (!config->hasGroup(QLatin1String("Piwigo Settings")))
    {
        QPointer<PiwigoLoginDlg> dlg = new PiwigoLoginDlg(QApplication::activeWindow(),
                                                          d->pPiwigo, i18n("Edit Piwigo Data"));
        dlg->exec();
        delete dlg;
    }

    readSettings();
    slotDoLogin();
}

PiwigoWindow::~PiwigoWindow()
{
    // write config

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("PiwigoSync Galleries"));

    group.writeEntry("Resize",         d->resizeCheckBox->isChecked());
    group.writeEntry("Maximum Width",  d->widthSpinBox->value());
    group.writeEntry("Maximum Height", d->heightSpinBox->value());
    group.writeEntry("Quality",        d->qualitySpinBox->value());

    delete d->talker;
    delete d->pPiwigo;
    delete d;
}

void PiwigoWindow::connectSignals()
{
    connect(d->albumView, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotAlbumSelected()));

    connect(d->confButton, SIGNAL(clicked()),
            this, SLOT(slotSettings()));

    connect(d->reloadButton, SIGNAL(clicked()),
            this, SLOT(slotReloadAlbums()));

    connect(d->resizeCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(slotEnableSpinBox(int)));

    connect(d->progressBar, SIGNAL(signalProgressCanceled()),
            this, SLOT(slotAddPhotoCancel()));

    connect(d->talker, SIGNAL(signalProgressInfo(QString)),
            this, SLOT(slotProgressInfo(QString)));

    connect(d->talker, SIGNAL(signalError(QString)),
            this, SLOT(slotError(QString)));

    connect(d->talker, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(d->talker, SIGNAL(signalLoginFailed(QString)),
            this, SLOT(slotLoginFailed(QString)));

    connect(d->talker, SIGNAL(signalAlbums(QList<PiwigoAlbum>)),
            this, SLOT(slotAlbums(QList<PiwigoAlbum>)));

    connect(d->talker, SIGNAL(signalAddPhotoSucceeded()),
            this, SLOT(slotAddPhotoSucceeded()));

    connect(d->talker, SIGNAL(signalAddPhotoFailed(QString)),
            this, SLOT(slotAddPhotoFailed(QString)));
}

void PiwigoWindow::readSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("PiwigoSync Galleries"));

    if (group.readEntry("Resize", false))
    {
        d->resizeCheckBox->setChecked(true);
        d->widthSpinBox->setEnabled(true);
        d->heightSpinBox->setEnabled(true);
    }
    else
    {
        d->resizeCheckBox->setChecked(false);
        d->heightSpinBox->setEnabled(false);
        d->widthSpinBox->setEnabled(false);
    }

    d->widthSpinBox->setValue(group.readEntry("Maximum Width",   1600));
    d->heightSpinBox->setValue(group.readEntry("Maximum Height", 1600));
    d->qualitySpinBox->setValue(group.readEntry("Quality",       95));
}

void PiwigoWindow::slotReloadAlbums()
{
    d->talker->listAlbums();
}

void PiwigoWindow::slotDoLogin()
{
    QUrl url(d->pPiwigo->url());

    if (url.scheme().isEmpty())
    {
        url.setScheme(QLatin1String("http"));
        url.setHost(d->pPiwigo->url());
    }

    // If we've done something clever, save it back to the piwigo.

    if (!url.url().isEmpty() && (d->pPiwigo->url() != url.url()))
    {
        d->pPiwigo->setUrl(url.url());
        d->pPiwigo->save();
    }

    d->talker->login(url, d->pPiwigo->username(), d->pPiwigo->password());

    d->userName->setText(QString::fromLatin1("<b>%1</b>").arg(d->pPiwigo->username()));
    d->url->setText(QString::fromLatin1(
                            "<b><a href='%1'>"
                            "<font color=\"#9ACD32\">%2</font>"
                            "</a></b>")
                            .arg(url.url())
                            .arg(url.toDisplayString())
    );
}

void PiwigoWindow::slotLoginFailed(const QString& msg)
{
    if (QMessageBox::question(this, i18nc("@title:window", "Login Failed"),
                              i18n("Failed to login into remote piwigo. ") + msg +
                              i18n("\nDo you want to check your settings and try again?"))
            != QMessageBox::Yes)
    {
        close();
        return;
    }

    QPointer<PiwigoLoginDlg> dlg = new PiwigoLoginDlg(QApplication::activeWindow(),
                                                      d->pPiwigo, i18n("Edit Piwigo Data"));

    int result = dlg->exec();
    delete dlg;

    if (result != QDialog::Accepted)
    {
        return;
    }

    slotDoLogin();
}

void PiwigoWindow::slotBusy(bool val)
{
    if (val)
    {
        setCursor(Qt::WaitCursor);
        startButton()->setEnabled(false);
        d->reloadButton->setEnabled(false);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        bool loggedIn = d->talker->loggedIn();
        d->reloadButton->setEnabled(loggedIn);

        bool b = (loggedIn && d->albumView->currentItem());
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "busy : " << b;
        startButton()->setEnabled(b);
    }
}

void PiwigoWindow::slotProgressInfo(const QString& msg)
{
    d->progressBar->progressStatusChanged(msg);
}

void PiwigoWindow::slotError(const QString& msg)
{
    setUiInProgressState(false);
    QMessageBox::critical(this, QString(), msg);
}

void PiwigoWindow::slotAlbums(const QList<PiwigoAlbum>& albumList)
{
    d->albumDict.clear();
    d->albumView->clear();

    // album work list

    QList<PiwigoAlbum> workList(albumList);
    QList<QTreeWidgetItem*> parentItemList;

    // fill QTreeWidget

    while (!workList.isEmpty())
    {
        // the album to work on

        PiwigoAlbum album = workList.takeFirst();
        int parentRefNum  = album.m_parentRefNum;

        if (parentRefNum == -1)
        {
            QTreeWidgetItem* const item = new QTreeWidgetItem();
            item->setText(0, cleanName(album.m_name));
            item->setIcon(0, QIcon::fromTheme(QLatin1String("inode-directory")));
            item->setData(1, Qt::UserRole, QVariant(album.m_refNum));
            item->setText(2, i18n("Album"));

            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Top : " << album.m_name << " " << album.m_refNum << "\n";

            d->albumView->addTopLevelItem(item);
            d->albumDict.insert(album.m_name, album);
            parentItemList << item;
        }
        else
        {
            QTreeWidgetItem* parentItem = nullptr;
            bool found                  = false;
            int i                       = 0;

            while (!found && (i < parentItemList.size()))
            {
                parentItem = parentItemList.at(i);

                if (parentItem && (parentItem->data(1, Qt::UserRole).toInt() == parentRefNum))
                {
                    QTreeWidgetItem* const item = new QTreeWidgetItem(parentItem);
                    item->setText(0, cleanName(album.m_name));
                    item->setIcon(0, QIcon::fromTheme(QLatin1String("inode-directory")));
                    item->setData(1, Qt::UserRole, album.m_refNum);
                    item->setText(2, i18n("Album"));

                    parentItem->addChild(item);
                    d->albumDict.insert(album.m_name, album);
                    parentItemList << item;
                    found = true;
                }

                i++;
            }
        }
    }
}

void PiwigoWindow::slotAlbumSelected()
{
    QTreeWidgetItem* const item = d->albumView->currentItem();

    // stop loading if user clicked an image

    if (item && (item->text(2) == i18n("Image")))
    {
        return;
    }

    if (!item)
    {
        startButton()->setEnabled(false);
    }
    else
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Album selected\n";

        int albumId = item->data(1, Qt::UserRole).toInt();
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << albumId << "\n";

        if (d->talker->loggedIn() && albumId)
        {
            startButton()->setEnabled(true);
        }
        else
        {
            startButton()->setEnabled(false);
        }
    }
}

void PiwigoWindow::slotAddPhoto()
{
    const QList<QUrl> urls(d->imageList->imageUrls());

    if (urls.isEmpty())
    {
        QMessageBox::critical(this, QString(),
                              i18n("Nothing to upload - please select photos to upload."));
        return;
    }

    for (QList<QUrl>::const_iterator it = urls.constBegin() ;
         it != urls.constEnd() ; ++it)
    {
        d->pUploadList.append((*it).toLocalFile());
    }

    d->uploadTotal = d->pUploadList.count();
    d->progressBar->reset();
    d->progressBar->setMaximum(d->uploadTotal);
    d->uploadCount = 0;
    startButton()->setEnabled(false);
    slotAddPhotoNext();
}

void PiwigoWindow::slotAddPhotoNext()
{
    if (d->pUploadList.isEmpty())
    {
        d->progressBar->reset();
        setUiInProgressState(false);
        startButton()->setEnabled(true);

        return;
    }

    QTreeWidgetItem* const item = d->albumView->currentItem();
    int column                  = d->albumView->currentColumn();
    QString albumTitle          = item->text(column);
    const PiwigoAlbum& album    = d->albumDict.value(albumTitle);
    d->currPhotoPath            = d->pUploadList.takeFirst();
    bool res                    = d->talker->addPhoto(album.m_refNum,
                                                      d->currPhotoPath,
                                                      d->resizeCheckBox->isChecked(),
                                                      d->widthSpinBox->value(),
                                                      d->heightSpinBox->value(),
                                                      d->qualitySpinBox->value());

    if (!res)
    {
        slotAddPhotoFailed(i18n("The file %1 is not a supported image or video format",
                                QUrl(d->currPhotoPath).fileName()));
        return;
    }

    d->progressBar->progressStatusChanged(i18n("Uploading file %1", QUrl(d->currPhotoPath).fileName()));
    d->imageList->processing(QUrl::fromLocalFile(d->currPhotoPath));

    if (d->progressBar->isHidden())
    {
        setUiInProgressState(true);
    }
}

void PiwigoWindow::slotAddPhotoSucceeded()
{
    d->uploadCount++;
    d->progressBar->setValue(d->uploadCount);
    d->imageList->processed(QUrl::fromLocalFile(d->currPhotoPath), true);
    slotAddPhotoNext();
}

void PiwigoWindow::slotAddPhotoFailed(const QString& msg)
{
    d->imageList->processed(QUrl::fromLocalFile(d->currPhotoPath), false);
    d->progressBar->reset();
    setUiInProgressState(false);

    if (QMessageBox::question(this, i18nc("@title:window", "Uploading Failed"),
                              i18n("Failed to upload media into remote Piwigo. ") + msg +
                              i18n("\nDo you want to continue?"))
            != QMessageBox::Yes)
    {
        return;
    }
    else
    {
        slotAddPhotoNext();
    }
}

void PiwigoWindow::slotAddPhotoCancel()
{
    d->imageList->processed(QUrl::fromLocalFile(d->currPhotoPath), false);
    d->progressBar->reset();
    setUiInProgressState(false);
    d->talker->cancel();
}

void PiwigoWindow::slotEnableSpinBox(int n)
{
    bool b;

    switch (n)
    {
        case 0:
        {
            b = false;
            break;
        }

        case 1:
        case 2:
        {
            b = true;
            break;
        }

        default:
        {
            b = false;
            break;
        }
    }

    d->widthSpinBox->setEnabled(b);
    d->heightSpinBox->setEnabled(b);
    d->qualitySpinBox->setEnabled(b);
}

void PiwigoWindow::slotSettings()
{
    // TODO: reload albumlist if OK slot used.

    QPointer<PiwigoLoginDlg> dlg = new PiwigoLoginDlg(QApplication::activeWindow(),
                                                      d->pPiwigo, i18n("Edit Piwigo Data"));

    if (dlg->exec() == QDialog::Accepted)
    {
        slotDoLogin();
    }

    delete dlg;
}

QString PiwigoWindow::cleanName(const QString& str) const
{
    QString plain = str;
    plain.replace(QLatin1String("&lt;"),   QLatin1String("<"));
    plain.replace(QLatin1String("&gt;"),   QLatin1String(">"));
    plain.replace(QLatin1String("&quot;"), QLatin1String("\""));
    plain.replace(QLatin1String("&amp;"),  QLatin1String("&"));

    return plain;
}

void PiwigoWindow::setUiInProgressState(bool inProgress)
{
    setRejectButtonMode(inProgress ? QDialogButtonBox::Cancel : QDialogButtonBox::Close);

    if (inProgress)
    {
        d->progressBar->show();
    }
    else
    {
        d->progressBar->hide();
        d->progressBar->progressCompleted();
    }
}

} // namespace DigikamGenericPiwigoPlugin

#include "moc_piwigowindow.cpp"
