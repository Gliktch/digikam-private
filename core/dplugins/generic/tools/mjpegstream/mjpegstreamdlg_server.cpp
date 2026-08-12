/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-24
 * Description : MJPEG Stream configuration dialog - Server methods
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Quoc Hưng Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mjpegstreamdlg_p.h"

namespace DigikamGenericMjpegStreamPlugin
{

void MjpegStreamDlg::updateServerStatus()
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
        d->srvPreview->setVisible(true);
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
        d->srvPreview->setVisible(false);
    }
}

bool MjpegStreamDlg::setMjpegServerContents()
{
    if (d->albumSupport)
    {
        DInfoInterface::DAlbumIDs albums = d->settings.iface->albumChooserItems();
        MjpegServerMap map;

        for (int id : std::as_const(albums))
        {
            DAlbumInfo anf(d->settings.iface->albumInfo(id));
            map.insert(anf.title(), d->settings.iface->albumItems(id));
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

bool MjpegStreamDlg::startMjpegServer()
{
    if (d->dirty)
    {
        d->dirty = false;
    }

    if (!setMjpegServerContents())
    {
        return false;
    }

    d->mngr->setSettings(d->settings);

    if (!d->mngr->startMjpegServer())
    {
        QMessageBox::warning(this, i18nc("@title:window", "Starting Media Server"),
                             i18nc("@info", "An error occurs while to start Media Server..."));
    }
    else
    {
        d->mngr->mjpegServerNotification(true);
    }

    updateServerStatus();

    return true;
}

void MjpegStreamDlg::slotSelectionChanged()
{
    d->dirty = true;
}

void MjpegStreamDlg::slotToggleMjpegServer()
{
    bool b = false;

    if (!d->mngr->isRunning())
    {
        if (startMjpegServer())
        {
            b = true;
        }
    }
    else
    {
        d->mngr->cleanUp();
        updateServerStatus();
    }

    d->tabView->setTabEnabled(Private::Stream,     !b);
    d->tabView->setTabEnabled(Private::Effect,     !b);
    d->tabView->setTabEnabled(Private::Transition, !b);
    d->tabView->setTabEnabled(Private::OSD,        !b);

    d->srvPort->setDisabled(b);
}

} // namespace DigikamGenericMjpegStreamPlugin
