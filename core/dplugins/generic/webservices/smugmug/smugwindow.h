/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-17-06
 * Description : a tool to export images to Smugmug web service
 *
 * SPDX-FileCopyrightText: 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QUrl>
#include <QCloseEvent>

// Local includes

#include "wstooldialog.h"
#include "wslogindialog.h"
#include "smugitem.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericSmugPlugin
{

class SmugWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit SmugWindow(DInfoInterface* const iface,
                        QWidget* const parent,
                        bool import = false,
                        const QString& nickName = QString());
    ~SmugWindow()                   override;

    /**
     * Use this method to (re-)activate the dialog after it has been created
     * to display it. This also loads the currently selected images.
     */
    void reactivate();

Q_SIGNALS:

    void updateHostApp(const QUrl& url);

protected:

    void closeEvent(QCloseEvent*)   override;

private Q_SLOTS:

    void slotBusy(bool val);
    void slotLoginProgress(int step, int maxStep, const QString& label);
    void slotLoginDone(int errCode, const QString& errMsg);
    void slotAddPhotoDone(int errCode, const QString& errMsg);

    void slotGetPhotoDone(int errCode,
                          const QString& errMsg,
                          const QByteArray& photoData);

    void slotCreateAlbumDone(int errCode,
                             const QString& errMsg,
                             qint64 newAlbumID,
                             const QString& newAlbumKey);

    void slotListAlbumsDone(int errCode,
                            const QString& errMsg,
                            const QList <SmugAlbum>& albumsList);

    void slotListPhotosDone(int errCode,
                            const QString& errMsg,
                            const QList <SmugPhoto>& photosList);

    void slotListAlbumTmplDone(int errCode,
                               const QString& errMsg,
                               const QList <SmugAlbumTmpl>& albumTList);

/*  Categories now are deprecated in API v2

    void slotListCategoriesDone(int errCode,
                                const QString& errMsg,
                                const QList <SmugCategory>& categoriesList);

    void slotListSubCategoriesDone(int errCode,
                                   const QString& errMsg,
                                   const QList <SmugCategory>& categoriesList);
*/

    void slotUserChangeRequest(bool anonymous);
    void slotReloadAlbumsRequest();
    void slotNewAlbumRequest();

    void slotStartTransfer();
    void slotCancelClicked();
    void slotStopAndCloseProgressBar();
    void slotDialogFinished();

    void slotImageListChanged();

    void slotTemplateSelectionChanged(int index);

/*  Categories now are deprecated in API v2

    void slotCategorySelectionChanged(int index);
*/

private:

    bool prepareImageForUpload(const QString& imgPath) const;
    void uploadNextPhoto();
    void downloadNextPhoto();

    void readSettings();
    void writeSettings();

    void authenticate();

    void buttonStateChange(bool state);
    void setUiInProgressState(bool inProgress);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
