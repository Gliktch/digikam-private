/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * SPDX-FileCopyrightText: 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2025 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QString>
#include <QMap>
#include <QNetworkReply>

class QDomElement;
class QByteArray;

namespace DigikamGenericImageShackPlugin
{

class ImageShackSession;

class ImageShackTalker : public QObject
{
    Q_OBJECT

public:

    explicit ImageShackTalker(ImageShackSession* const session);
    ~ImageShackTalker()                                                           override;

public:

    void authenticate();
    void cancelLogIn();
    void cancel();
    void getGalleries();

    void uploadItem(const QString& path, const QMap<QString, QString>& opts);
    void uploadItemToGallery(const QString& path,
                             const QString& gallery,
                             const QMap<QString, QString>& opts);

Q_SIGNALS:

    void signalBusy(bool busy);
    void signalJobInProgress(int step,
                             int maxStep = 0,
                             const QString& label = QString());
    void signalLoginDone(int errCode,  const QString& errMsg);
    void signalGetGalleriesDone(int errCode, const QString& errMsg);

    void signalAddPhotoDone(int errCode, const QString& errMsg);
    void signalUpdateGalleries(const QStringList& gTexts,
                               const QStringList& gNames);

private Q_SLOTS:

    void slotFinished(QNetworkReply* reply);

private:

    QString getCallString(QMap<QString, QString>& args)                     const;
    void    checkRegistrationCodeDone(int errCode, const QString& errMsg);
    void    parseAccessToken(const QByteArray& data);
    void    parseGetGalleries(const QByteArray& data);
    void    authenticationDone(int errCode, const QString& errMsg);

    void    logOut();

    int     parseErrorResponse(const QDomElement& elem, QString& errMsg)    const;

    void    parseUploadPhotoDone(const QByteArray& data);
    void    parseAddPhotoToGalleryDone(const QByteArray& data);

    QString mimeType(const QString& path)                                   const;

private:

    /// @note disabled
    explicit ImageShackTalker(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericImageShackPlugin
