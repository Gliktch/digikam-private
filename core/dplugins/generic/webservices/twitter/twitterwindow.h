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

#pragma once

// Qt includes

#include <QList>

// Local includes

#include "dinfointerface.h"
#include "wstooldialog.h"

using namespace Digikam;

class QCloseEvent;
class QUrl;

namespace DigikamGenericTwitterPlugin
{

class TwAlbum;

class TwWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit TwWindow(DInfoInterface* const iface, QWidget* const parent);
    ~TwWindow()                     override;

    void reactivate();

    void setItemsList(const QList<QUrl>& urls);

private:

    void readSettings();
    void writeSettings();

    void uploadNextPhoto();

    void buttonStateChange(bool state);
    void closeEvent(QCloseEvent*)   override;

private Q_SLOTS:

    void slotImageListChanged();
    void slotUserChangeRequest();
    void slotNewAlbumRequest();
/*
    void slotReloadAlbumsRequest();
*/
    void slotStartTransfer();

    void slotBusy(bool);
    void slotSignalLinkingFailed();
    void slotSignalLinkingSucceeded();
    void slotSetUserName(const QString& msg);
    void slotListAlbumsFailed(const QString& msg);
    void slotListAlbumsDone(const QList<QPair<QString, QString> >& list);
    void slotCreateFolderFailed(const QString& msg);
    void slotCreateFolderSucceeded();
    void slotAddPhotoFailed(const QString& msg);
    void slotAddPhotoSucceeded();
    void slotTransferCancel();

    void slotFinished();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericTwitterPlugin
