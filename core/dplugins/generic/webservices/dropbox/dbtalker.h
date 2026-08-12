/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export images to Dropbox web service
 *
 * SPDX-FileCopyrightText: 2013 by Pankaj Kumar <me at panks dot me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QPair>
#include <QString>
#include <QNetworkReply>

namespace DigikamGenericDropBoxPlugin
{

class DBTalker : public QObject
{
    Q_OBJECT

public:

    explicit DBTalker(QWidget* const parent);
    ~DBTalker() override;

public:

    void link();
    void unLink();
    bool authenticated();
    void reauthenticate();
    void getUserName();
    void cancel();
    void listFolders(const QString& cursor = QString());
    bool addPhoto(const QString& imgPath, const QString& uploadFolder,
                  bool original, bool rescale, int maxDim, int imageQuality);
    void createFolder(const QString& path);

Q_SIGNALS:

    void signalBusy(bool val);
    void signalLinkingSucceeded();
    void signalLinkingFailed();
    void signalSetUserName(const QString& msg);
    void signalListAlbumsFailed(const QString& msg);
    void signalListAlbumsDone(const QList<QPair<QString, QString> >& list);
    void signalCreateFolderFailed(const QString& msg);
    void signalCreateFolderSucceeded();
    void signalAddPhotoFailed(const QString& msg);
    void signalAddPhotoSucceeded();

private Q_SLOTS:

    void slotLinkingFailed();
    void slotLinkingSucceeded();
    void slotOpenBrowser(const QUrl& url);
    void slotFinished(QNetworkReply* reply);

private:

    void parseResponseUserName(const QByteArray& data);
    void parseResponseListFolders(const QByteArray& data);
    void parseResponseCreateFolder(const QByteArray& data);
    void parseResponseAddPhoto(const QByteArray& data);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericDropBoxPlugin
