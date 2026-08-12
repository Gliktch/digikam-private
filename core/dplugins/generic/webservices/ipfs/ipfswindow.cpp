/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-12
 * Description : a tool to export images to IPFS web service
 *
 * SPDX-FileCopyrightText: 2018      by Amar Lakshya <amar dot lakshya at xaviers dot edu dot in>
 * SPDX-FileCopyrightText: 2018-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ipfswindow.h"

// Qt includes

#include <QDesktopServices>
#include <QInputDialog>
#include <QCloseEvent>
#include <QMessageBox>
#include <QBoxLayout>
#include <QWindow>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"

namespace DigikamGenericIpfsPlugin
{

class Q_DECL_HIDDEN IpfsWindow::Private
{
public:

    explicit Private()
      : list (nullptr),
        api  (nullptr)
    {
    }

    IpfsImagesList* list;
    IpfsTalker*     api;

    /**
     * Contains the ipfs username if API authorized.
     * If not, username is null.
     */
    QString         username;
};

IpfsWindow::IpfsWindow(DInfoInterface* const iface, QWidget* const /*parent*/)
    : WSToolDialog(nullptr, QLatin1String("IPFS Dialog")),
      d           (new Private)
{
    d->api = new IpfsTalker(this);

    // Connect API signals

    connect(d->api, &IpfsTalker::progress,
            this, &IpfsWindow::apiProgress);

    connect(d->api, &IpfsTalker::success,
            this, &IpfsWindow::apiSuccess);

    connect(d->api, &IpfsTalker::error,
            this, &IpfsWindow::apiError);

    connect(d->api, &IpfsTalker::busy,
            this, &IpfsWindow::apiBusy);

    // | List | Auth |

    auto* const mainLayout = new QHBoxLayout;
    auto* const mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    setMainWidget(mainWidget);

    d->list = new IpfsImagesList;
    d->list->setIface(iface);
    d->list->loadImagesFromCurrentSelection();
    mainLayout->addWidget(d->list);

    /**
     * |  Logged in as:  |
     * | <Not logged in> |
     * |     Forget      |
     */

    auto* const authLayout = new QVBoxLayout;
    mainLayout->addLayout(authLayout);

    authLayout->insertStretch(-1, 1);

    /**
     * Add anonymous upload button
     * Connect UI signals
     */

    connect(startButton(), &QPushButton::clicked,
            this, &IpfsWindow::slotUpload);

    connect(this, &IpfsWindow::finished,
            this, &IpfsWindow::slotFinished);

    connect(this, &IpfsWindow::cancelClicked,
            this, &IpfsWindow::slotCancel);

    setWindowIcon(QIcon::fromTheme(QString::fromLatin1("ipfs")));
    setWindowTitle(i18nc("@title:window", "Export to IPFS"));
    setModal(false);

    startButton()->setText(i18nc("@action:button", "Upload"));
    startButton()->setToolTip(i18nc("@info:tooltip, button", "Start upload to IPFS"));
    startButton()->setEnabled(true);

    // Only used if not overwritten by readSettings()

    resize(650, 320);
    readSettings();
}

IpfsWindow::~IpfsWindow()
{
    saveSettings();
    delete d;
}

void IpfsWindow::reactivate()
{
    d->list->loadImagesFromCurrentSelection();
    show();
}

void IpfsWindow::slotUpload()
{
    QList<const IpfsImagesListViewItem*> pending = d->list->getPendingItems();

    if (pending.isEmpty())
    {
        return;
    }

    for (const auto& item : std::as_const(pending))
    {
        IpfsTalkerAction action;
        action.type               = IpfsTalkerActionType::IMG_UPLOAD;
        action.upload.imgpath     = item->url().toLocalFile();
        action.upload.title       = item->Title();
        action.upload.description = item->Description();

        d->api->queueWork(action);
    }
}

void IpfsWindow::slotFinished()
{
    saveSettings();
}

void IpfsWindow::slotCancel()
{
    d->api->cancelAllWork();
}

/*
void IpfsWindow::apiAuthorized(bool success, const QString& username)
{
    if (success)
    {
        d->username = username;
        d->userLabel->setText(d->username);
        d->forgetButton->setEnabled(true);
        return;
    }

    d->username = QString();
    d->userLabel->setText(i18n("<Not logged in>"));
    d->forgetButton->setEnabled(false);
}
*/

/*
void IpfsWindow::apiAuthError(const QString& msg)
{
    QMessageBox::critical(this,
                          i18nc("@title:window", "Authorization Failed"),
                          i18n("Failed to log into IPFS: %1\n", msg));
}
*/

void IpfsWindow::apiProgress(unsigned int /*percent*/, const IpfsTalkerAction& action)
{
    d->list->processing(QUrl::fromLocalFile(action.upload.imgpath));
}

void IpfsWindow::apiRequestPin(const QUrl& url)
{
    QDesktopServices::openUrl(url);
}

void IpfsWindow::apiSuccess(const IpfsTalkerResult& result)
{
    d->list->slotSuccess(result);
}

void IpfsWindow::apiError(const QString& msg, const IpfsTalkerAction& action)
{
    d->list->processed(QUrl::fromLocalFile(action.upload.imgpath), false);

    // 1 here because the current item is still in the queue.

    if (d->api->workQueueLength() <= 1)
    {
        QMessageBox::critical(this,
                              i18nc("@title:window", "Uploading Failed"),
                              i18n("Failed to upload photo to IPFS: %1\n", msg));
        return;
    }

    QMessageBox::StandardButton cont = QMessageBox::question(this,
                                           i18nc("@title:window", "Uploading Failed"),
                                           i18n("Failed to upload photo to IPFS: %1\n"
                                                "Do you want to continue?", msg));

    if (cont != QMessageBox::Yes)
    {
        d->api->cancelAllWork();
    }
}

void IpfsWindow::apiBusy(bool busy)
{
    setCursor(busy ? Qt::WaitCursor : Qt::ArrowCursor);
    startButton()->setEnabled(!busy);
}

void IpfsWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

void IpfsWindow::readSettings()
{
    KSharedConfigPtr config  = KSharedConfig::openConfig();
    KConfigGroup groupAuth   = config->group(QLatin1String("IPFS Auth"));
    d->username              = groupAuth.readEntry("UserName", QString());
/*
    apiAuthorized(!d->username.isEmpty(), d->username);
*/
}

void IpfsWindow::saveSettings()
{
    KSharedConfigPtr config  = KSharedConfig::openConfig();
    KConfigGroup groupAuth   = config->group(QLatin1String("IPFS Auth"));
    groupAuth.writeEntry("UserName", d->username);
}

} // namespace DigikamGenericIpfsPlugin

#include "moc_ipfswindow.cpp"
