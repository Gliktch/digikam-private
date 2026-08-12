/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-26
 * Description : a tool to export items to Facebook web service
 *
 * SPDX-FileCopyrightText: 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2009 by Luka Renko <lure at kubuntu dot org>
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

class QCloseEvent;
class QUrl;

using namespace Digikam;

namespace DigikamGenericFaceBookPlugin
{

class FbAlbum;

class FbWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit FbWindow(DInfoInterface* const iface, QWidget* const parent);
    ~FbWindow()                         override;

private Q_SLOTS:

    void slotBusy(bool val);
    void slotLoginProgress(int step, int maxStep, const QString& label);
    void slotLoginDone(int errCode, const QString& errMsg);
    void slotAddPhotoDone(int errCode, const QString& errMsg);
    void slotCreateAlbumDone(int errCode, const QString& errMsg,
                             const QString& newAlbumID);
    void slotListAlbumsDone(int errCode, const QString& errMsg,
                            const QList<FbAlbum>& albumsList);

    void slotUserChangeRequest();
    void slotReloadAlbumsRequest(long long userID);
    void slotNewAlbumRequest();
    void slotStartTransfer();
    void slotImageListChanged();
    void slotStopAndCloseProgressBar();

    void slotFinished();
    void slotCancelClicked();

private:

    void    setProfileAID(long long userID);
    QString getImageCaption(const QString& fileName);
    bool    prepareImageForUpload(const QString& imgPath, QString& caption);

    void    uploadNextPhoto();

    void    readSettings();
    void    writeSettings();

    void    authenticate(bool forceLogin);

    void    buttonStateChange(bool state);

    void    closeEvent(QCloseEvent*)    override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericFaceBookPlugin
