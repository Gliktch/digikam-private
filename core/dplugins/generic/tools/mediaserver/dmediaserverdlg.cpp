/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : Media Server configuration dialog to share a single list of files
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2017      by Ahmed Fathy <ahmed dot fathi dot abdelmageed at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmediaserverdlg.h"

// Qt includes

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QStyle>
#include <QIcon>
#include <QCheckBox>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "ditemslist.h"
#include "dxmlguiwindow.h"
#include "workingwidget.h"
#include "dmediaservermngr.h"

namespace DigikamGenericMediaServerPlugin
{

class Q_DECL_HIDDEN DMediaServerDlg::Private
{
public:

    Private() = default;

public:

    bool                dirty           = false;
    DMediaServerMngr*   mngr            = DMediaServerMngr::instance();
    QPushButton*        srvButton       = nullptr;
    QLabel*             srvStatus       = nullptr;
    WorkingWidget*      progress        = nullptr;
    QLabel*             aStats          = nullptr;
    QLabel*             separator       = nullptr;
    QLabel*             iStats          = nullptr;
    QCheckBox*          startOnStartup  = nullptr;
    bool                albumSupport    = false;
    QWidget*            albumSelector   = nullptr;
    DItemsList*         listView        = nullptr;
    DInfoInterface*     iface           = nullptr;
    QWidget*            page            = nullptr;
    QDialogButtonBox*   buttons         = nullptr;
};

DMediaServerDlg::DMediaServerDlg(QObject* const /*parent*/,
                                 DInfoInterface* const iface)
    : DPluginDialog(nullptr, DMediaServerMngr::instance()->configGroupName()),
      d            (new Private)
{
    setWindowTitle(i18nc("@title:window", "Share Files With DLNA Media Server"));

    d->iface                 = iface;

    // NOTE: We overwrite the default albums chooser object name for load save check items state between sessions.
    // The goal is not mix these settings with other export tools.

    d->iface->setObjectName(QLatin1String("SetupMediaServerIface"));

    m_buttons->addButton(QDialogButtonBox::Cancel);
    m_buttons->addButton(QDialogButtonBox::Ok);
    m_buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    d->page                  = new QWidget(this);
    QVBoxLayout* const vbx   = new QVBoxLayout(this);
    vbx->addWidget(d->page);
    vbx->addWidget(m_buttons);
    setLayout(vbx);
    setModal(false);

    // -------------------

    QGridLayout* const grid = new QGridLayout(d->page);
    d->albumSupport         = (d->iface && d->iface->supportAlbums());

    if (d->albumSupport)
    {
        d->albumSelector = d->iface->albumChooser(this);
        grid->addWidget(d->albumSelector, 0, 0, 1, 6);

        connect(d->iface, SIGNAL(signalAlbumChooserSelectionChanged()),
                this, SLOT(slotSelectionChanged()));
    }
    else
    {
        d->listView = new DItemsList(d->page);
        d->listView->setObjectName(QLatin1String("MediaServer ImagesList"));
        d->listView->setControlButtonsPlacement(DItemsList::ControlButtonsRight);
        d->listView->setIface(d->iface);

        // Add all items currently loaded in application.

        d->listView->loadImagesFromCurrentSelection();

        // Replug the previous shared items list.

        d->listView->slotAddImages(d->mngr->itemsList());
        grid->addWidget(d->listView, 0, 0, 1, 6);

        connect(d->listView, SIGNAL(signalImageListChanged()),
                this, SLOT(slotSelectionChanged()));
    }

    // -------------------

    const int spacing         = layoutSpacing();


    d->startOnStartup         = new QCheckBox(i18nc("@option", "Start Server at Startup"));
    d->startOnStartup->setWhatsThis(i18nc("@info", "Set this option to turn-on the DLNA server at application start-up automatically"));
    d->startOnStartup->setChecked(true);

    d->srvButton              = new QPushButton(this);
    d->srvStatus              = new QLabel(this);
    d->progress               = new WorkingWidget(this);
    d->aStats                 = new QLabel(this);
    d->separator              = new QLabel(QLatin1String(" / "), this);
    d->iStats                 = new QLabel(this);

    QLabel* const explanation = new QLabel(this);
    explanation->setOpenExternalLinks(true);
    explanation->setWordWrap(true);
    explanation->setFrameStyle(QFrame::Box | QFrame::Plain);

    explanation->setText(i18nc("@info",
                               "The media server allows to share items through the local network "
                               "using %1 standard and %2 protocol. "
                               "Many kind of electronic devices can support DLNA, as tablets, cellulars, TV, etc.<br><br>"
                               "Note: depending of the network features and the configuration, "
                               "the delay to discover the server on client devices can take a while.",
                               QLatin1String("<a href='https://en.wikipedia.org/wiki/Digital_Living_Network_Alliance'>DLNA</a>"),
                               QLatin1String("<a href='https://en.wikipedia.org/wiki/Universal_Plug_and_Play'>UPNP</a>")));

    grid->addWidget(d->startOnStartup, 1, 0, 1, 6);
    grid->addWidget(d->srvButton,      2, 0, 1, 1);
    grid->addWidget(d->srvStatus,      2, 1, 1, 1);
    grid->addWidget(d->aStats,         2, 2, 1, 1);
    grid->addWidget(d->separator,      2, 3, 1, 1);
    grid->addWidget(d->iStats,         2, 4, 1, 1);
    grid->addWidget(d->progress,       2, 5, 1, 1);
    grid->addWidget(explanation,       3, 0, 1, 6);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(0, 10);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->srvButton, SIGNAL(clicked()),
            this, SLOT(slotToggleMediaServer()));

    connect(m_buttons->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &DMediaServerDlg::reject);

    connect(m_buttons->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &DMediaServerDlg::accept);

    // -------------------

    readSettings();
}

DMediaServerDlg::~DMediaServerDlg()
{
    delete d;
}

void DMediaServerDlg::accept()
{
    if (d->dirty)
    {
        bool empty = false;

        if (d->albumSupport)
        {
            empty = d->iface->albumChooserItems().isEmpty();
        }
        else
        {
            empty = d->listView->imageUrls().isEmpty();
        }

        if (!empty)
        {
            int rc = QMessageBox::question(this, i18nc("@title:window", "Media Server Contents"),
                                           i18nc("@info", "The items list to share has changed. "
                                                "Do you want to start now the media server with this contents?"));
            if (rc == QMessageBox::Yes)
            {
                startMediaServer();
            }
        }
    }

    saveSettings();
    QDialog::accept();
}

void DMediaServerDlg::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->mngr->configGroupName());

    d->startOnStartup->setChecked(group.readEntry(d->mngr->configStartServerOnStartupEntry(), false));

    updateServerStatus();
}

void DMediaServerDlg::saveSettings()
{
    setMediaServerContents();

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->mngr->configGroupName());
    group.writeEntry(d->mngr->configStartServerOnStartupEntry(), d->startOnStartup->isChecked());
    config->sync();
}

void DMediaServerDlg::updateServerStatus()
{
    if (d->mngr->isRunning())
    {
        d->srvStatus->setText(i18nc("@label", "Server is running"));
        d->aStats->setText(i18ncp("@info", "1 album shared", "%1 albums shared", d->mngr->albumsShared()));
        d->separator->setVisible(true);
        d->iStats->setText(i18ncp("@info", "1 item shared",  "%1 items shared",  d->mngr->itemsShared()));
        d->srvButton->setText(i18nc("@action: button", "Stop"));
        d->srvButton->setIcon(QIcon::fromTheme(QLatin1String("media-playback-stop")));
        d->progress->toggleTimer(true);
        d->progress->setVisible(true);
    }
    else
    {
        d->srvStatus->setText(i18nc("@label", "Server is not running"));
        d->aStats->clear();
        d->separator->setVisible(false);
        d->iStats->clear();
        d->srvButton->setText(i18nc("@action: button", "Start"));
        d->srvButton->setIcon(QIcon::fromTheme(QLatin1String("media-playback-start")));
        d->progress->toggleTimer(false);
        d->progress->setVisible(false);
    }
}

bool DMediaServerDlg::setMediaServerContents()
{
    if (d->albumSupport)
    {
        DInfoInterface::DAlbumIDs albums = d->iface->albumChooserItems();
        MediaServerMap map;

        for (int id : std::as_const(albums))
        {
            DAlbumInfo anf(d->iface->albumInfo(id));
            map.insert(anf.title(), d->iface->albumItems(id));
        }

        if (map.isEmpty())
        {
            QMessageBox::information(this, i18nc("@title:window", "Media Server Contents"),
                                     i18nc("@info", "There is no collection to share with the current selection..."));
            return false;
        }

        d->mngr->setCollectionMap(map);
    }
    else
    {
        QList<QUrl> urls = d->listView->imageUrls();

        if (urls.isEmpty())
        {
            QMessageBox::information(this, i18nc("@title:window", "Media Server Contents"),
                                     i18nc("@info", "There is no item to share with the current selection..."));

            return false;
        }

        d->mngr->setItemsList(i18nc("@info", "Shared Items"), urls);
    }

    return true;
}

void DMediaServerDlg::startMediaServer()
{
    if (d->dirty)
    {
        d->dirty = false;
    }

    if (!setMediaServerContents())
    {
        return;
    }

    if (!d->mngr->startMediaServer())
    {
        QMessageBox::warning(this, i18nc("@title:window", "Starting Media Server"),
                             i18nc("@info", "An error occurs while to start Media Server..."));
    }
    else
    {
        d->mngr->mediaServerNotification(true);
    }

    updateServerStatus();
}

void DMediaServerDlg::slotSelectionChanged()
{
    d->dirty = true;
}

void DMediaServerDlg::slotToggleMediaServer()
{
    if (!d->mngr->isRunning())
    {
        startMediaServer();
    }
    else
    {
        d->mngr->cleanUp();
        updateServerStatus();
    }
}

} // namespace DigikamGenericMediaServerPlugin

#include "moc_dmediaserverdlg.cpp"
