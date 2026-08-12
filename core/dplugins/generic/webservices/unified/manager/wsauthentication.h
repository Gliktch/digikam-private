/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-03
 * Description : Web Service authentication container.
 *
 * SPDX-FileCopyrightText: 2018 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QtGlobal>
#include <QString>
#include <QUrl>
#include <QWidget>
#include <QMap>
#include <QSettings>
#include <QStringList>
#include <QList>

// Local includes

#include "dinfointerface.h"
#include "wssettings.h"
#include "wsitem.h"

using namespace Digikam;

namespace DigikamGenericUnifiedPlugin
{

class WSAuthentication : public QObject
{
    Q_OBJECT

public:

    explicit WSAuthentication(QWidget* const parent, DInfoInterface* const iface=0);
    ~WSAuthentication();

    /**
     * Return selected web service by user
     */
    QString webserviceName();

    /**
     * Create appropriate album dialog (filling form for creating new album).
     * Create appropriate talker for selected web service by user.
     */
    void    createTalker(WSSettings::WebService ws, const QString& serviceName=QString());

    /**
     * Wrapper of talker->cancel()
     */
    void    cancelTalker();

    /**
     * Wrapper of talker->authenticate()
     */
    void    authenticate();

    /**
     * Wrapper of talker->reauthenticate()
     */
    void    reauthenticate();

    /**
     * Wrapper of talker->linked()
     */
    bool    authenticated() const;

    /**
     * Get image title, comment -> concat all and return a string.
     */
    QString getImageCaption(const QString& fileName);

    /**
     * Image processing before uploading (compress, reformat, add/remove metadata)
     */
    void    prepareForUpload();

    /**
     * Return number of images to upload
     */
    unsigned int    numberItemsUpload();

    /**
     * Upload photo to web service (one at a time).
     * This method is called for the first time in startTransfer.
     * Then each time a photo is uploaded successfully, this method
     * will be called in slotAddPhotoDone(...)
     */
    void    uploadNextPhoto();

    /**
     * Begin transferring photos by calling uploadNextPhoto()
     */
    void    startTransfer();

private:

    /**
     * Parse albums list to a map of pairs <album_id, content> and a list of
     * albums at the very top level (root of other albums).
     */
    void    parseTreeFromListAlbums(const QList <WSAlbum>& albumsList,
                                    QMap<QString, AlbumSimplified>& albumTree,
                                    QStringList& rootAlbums);

Q_SIGNALS:

    /**
     * Connect WSTalker to WSAuthentication
     */
    void    signalOpenBrowser(const QUrl&);
    void    signalCloseBrowser();
    void    signalAuthenticationComplete(bool);

    /**
     * Connect WSAuthenticationPage to WSAuthentication
     */
    void    signalResponseTokenReceived(const QMap<QString, QString>&);

    /**
     * Connect WSAuthentication to WSImagePage
     */
    void    signalCreateAlbumDone(int errCode, const QString& errMsg, const QString& newAlbumId);
    void    signalListAlbumsDone(const QMap<QString, AlbumSimplified>&, const QStringList&, const QString&);

    /**
     * Connect WSAuthentication to WSFinalPage
     */
    void    signalProgress(int);
    void    signalMessage(const QString&, bool);
    void    signalDone();

public Q_SLOTS:

    /**
     * Cancel talker and purge temporary files
     */
    void    slotCancel();

    /**
     * Request from WSImagePage
     */
    void    slotNewAlbumRequest();
    void    slotListAlbumsRequest();

    /**
     * Handle signals for these functionalities from Talker
     */
    void    slotListAlbumsDone(int errCode, const QString& errMsg, const QList<WSAlbum>& albumsList);
    void    slotAddPhotoDone(int errCode, const QString& errMsg);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericUnifiedPlugin
