/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export items to Google web services
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QPair>
#include <QUrl>
#include <QPointer>

// Local includes

#include "wstooldialog.h"
#include "gsitem.h"
#include "dinfointerface.h"
#include "dmetadata.h"

class QCloseEvent;

using namespace Digikam;

namespace DigikamGenericGoogleServicesPlugin
{
class GDTalker;
class GPTalker;
class GSWidget;
class GSPhoto;
class GSFolder;
class GSNewAlbumDlg;

class GSWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit GSWindow(DInfoInterface* const iface,
                      QWidget* const parent,
                      const QString& serviceName);
    ~GSWindow() override;

    void reactivate();

Q_SIGNALS:

    void updateHostApp(const QUrl& url);

private:

    void readSettings();
    void writeSettings();

    void uploadNextPhoto();
    void downloadNextPhoto();

    void buttonStateChange(bool state);
    void closeEvent(QCloseEvent*) override;

private Q_SLOTS:

    void slotImageListChanged();
    void slotUserChangeRequest();
    void slotNewAlbumRequest();
    void slotReloadAlbumsRequest();
    void slotStartTransfer();
    void slotFinished();

    void slotBusy(bool);
    void slotAccessTokenObtained();
    void slotAuthenticationRefused();
    void slotSetUserName(const QString& msg);
    void slotListAlbumsDone(int, const QString&,
                            const QList <GSFolder>&);
    void slotListPhotosDoneForDownload(int errCode,
                                       const QString& errMsg,
                                       const QList <GSPhoto>& photosList);
    void slotCreateFolderDone(int,
                              const QString& msg,
                              const QString& = QLatin1String("-1"));
    void slotAddPhotoDone(int,
                          const QString& msg);
    void slotUploadPhotoDone(int,
                             const QString& msg,
                             const QStringList&);
    void slotGetPhotoDone(int errCode,
                          const QString& errMsg,
                          const QByteArray& photoData,
                          const QString& fileName);
    void slotTransferCancel();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericGoogleServicesPlugin
