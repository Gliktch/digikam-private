/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-03-20
 * Description : a tool to export images to iNaturalist web service
 *
 * SPDX-FileCopyrightText: 2021-2022 by Joerg Lohse <joergmlpts at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2009 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2017-2019 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QList>
#include <QPair>
#include <QString>
#include <QObject>
#include <QNetworkReply>
#include <QNetworkCookie>

// Local includes

#include "dinfointerface.h"
#include "ditemslist.h"
#include "inattaxon.h"
#include "inatscore.h"

class QProgressDialog;

using namespace Digikam;

namespace DigikamGenericINatPlugin
{

class INatPhotoInfo;
class INatPhotoSet;

class AuthRequest;
class ApiTokenRequest;

typedef QPair<QString, QList<Taxon> > AutoCompletions;

class INatTalker : public QObject
{
    Q_OBJECT

public:

    /**
     * This struct contains all the data needed for photo uploads.
     */
    struct PhotoUploadRequest
    {
        PhotoUploadRequest() = default;

        PhotoUploadRequest(const QList<QUrl>& imgs,
                           bool updId,
                           bool resize,
                           int mxDim,
                           int q, const QString& userName)
            : m_totalImages  (imgs.count()),
              m_images       (imgs),
              m_user         (userName),
              m_updateIds    (updId),
              m_rescale      (resize),
              m_maxDim       (mxDim),
              m_quality      (q)
        {
        }

        int         m_observationId = -1;
        int         m_totalImages   = 0;
        QList<QUrl> m_images;
        QString     m_apiKey;
        QString     m_user;
        bool        m_updateIds     = false;
        bool        m_rescale       = false;
        int         m_maxDim        = -1;
        int         m_quality       = -1;
    };

    /**
     * This struct is sent after each photo has been uploaded.
     */
    struct PhotoUploadResult
    {
        PhotoUploadResult() = default;

        PhotoUploadResult(const PhotoUploadRequest& req, int obsPhId, int phId)
            : m_request           (req),
              m_observationPhotoId(obsPhId),
              m_photoId           (phId)
        {
        }

        PhotoUploadRequest m_request;
        int                m_observationPhotoId = -1;
        int                m_photoId            = -1;
    };

    struct NearbyObservation
    {
        NearbyObservation() = default;

        NearbyObservation(int id,
                          double latitude,
                          double longitude,
                          double distanceMeters,
                          bool obscured,
                          uint taxon,
                          double referenceLatitude,
                          double referenceLongitude)
            : m_observationId       (id),
              m_latitude            (latitude),
              m_longitude           (longitude),
              m_distanceMeters      (distanceMeters),
              m_obscured            (obscured),
              m_referenceTaxon      (taxon),
              m_referenceLatitude   (referenceLatitude),
              m_referenceLongitude  (referenceLongitude)
        {
        }

        void updateObservation(int id,
                               double latitude,
                               double longitude,
                               double distanceMeters)
        {
            m_observationId  = id;
            m_latitude       = latitude;
            m_longitude      = longitude;
            m_distanceMeters = distanceMeters;
        }

        bool isValid() const
        {
            return (m_observationId != -1);
        }

        int    m_observationId      = -1;
        double m_latitude           = 0.0;
        double m_longitude          = 0.0;
        double m_distanceMeters     = -1.0;
        bool   m_obscured           = false;
        uint   m_referenceTaxon     = 0;
        double m_referenceLatitude  = 0.0;
        double m_referenceLongitude = 0.0;
    };

    INatTalker(QWidget* const parent,
               const QString& serviceName,
               DInfoInterface* const iface);
    ~INatTalker() override;

    void    unLink();
    void    cancel();
    void    removeUserName(const QString& userName);

    /**
     * Are we still uploading observations or photos?
     */
    bool    stillUploading() const;

    /**
     * Returns -1 if there is no valid token; number of seconds otherwise.
     */
    int     apiTokenExpiresIn() const;

    /**
     * Try to restore a valid API token; they are good for 24 hours.
     */
    bool    restoreApiToken(const QString& username,
                            QList<QNetworkCookie>&,
                            bool emitSignal);

    /**
     * Download an image from iNaturalist servers.
     */
    void    loadUrl(const QUrl& url, int retries = 0);

    /**
     * Obtain auto completions for taxa from iNaturalist servers.
     */
    void    taxonAutoCompletions(const QString& partialName);

    /**
     * Retrieve login, name, and icon of current user.
     */
    void    userInfo(const QList<QNetworkCookie>&);

    /**
     * Get list of nearby places.
     */
    void    nearbyPlaces(double latitude, double longitude);

    /**
     * Get closest nearby observation.
     */
    void    closestObservation(uint taxon, double latitude, double longitude,
                               double radius_km = 10.0,
                               const QString& origQuery = QString());

    /**
     * Identify taxa from a given photo.
     */
    void    computerVision(const QUrl& localImage);

    /**
     * Create an iNaturalist observation; photo uploads follow.
     */
    void    createObservation(const QByteArray&, const PhotoUploadRequest&);

    /**
     * Check whether an iNaturalist observation has been created. Called
     * upon timeout in createObservation().
     */
    void    verifyCreateObservation(const QByteArray&,
                                    const PhotoUploadRequest&,
                                    int page, int retries);

    /**
     * Delete an observation; called when canceling uploads.
     */
    void    deleteObservation(int id, const QString& apiKey, int retries = 0);

    /**
     * Upload another photo to previously created observation.
     */
    void    uploadNextPhoto(const PhotoUploadRequest&);

    /**
     * Check whether an observation photo has been uploaded. Called
     * upon timeout in uploadNextPhoto().
     */
    void    verifyUploadNextPhoto(const PhotoUploadRequest&, int retries);

public:

    QProgressDialog* m_authProgressDlg = nullptr;

Q_SIGNALS:

    void signalBusy(bool val);

    void signalLoadUrlSucceeded(const QUrl&, const QByteArray&);
    void signalTaxonAutoCompletions(const AutoCompletions&);
    void signalNearbyObservation(const INatTalker::NearbyObservation&);
    void signalComputerVisionResults(const ImageScores&);
    void signalNearbyPlaces(const QStringList&);
    void signalObservationCreated(const INatTalker::PhotoUploadRequest&);
    void signalObservationDeleted(int id);
    void signalPhotoUploaded(const INatTalker::PhotoUploadResult&);

    void signalLinkingSucceeded(const QString& login,
                                const QString& name, const QUrl& iconUrl);
    void signalLinkingFailed(const QString& error);

private:

    QString tmpFileName(const QString& path);

private Q_SLOTS:

    void slotApiToken(const QString&, const QList<QNetworkCookie>&);
    void slotFinished(QNetworkReply* reply);
    void slotTimeout();

private:

    class Private;
    Private* const d = nullptr;

    friend class UserRequest;
    friend class LoadUrlRequest;
    friend class AutoCompletionRequest;
    friend class NearbyPlacesRequest;
    friend class NearbyObservationRequest;
    friend class ComputerVisionRequest;
};

} // namespace DigikamGenericINatPlugin
