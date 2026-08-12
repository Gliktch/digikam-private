/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 02-02-2012
 * Description : Face database interface to train identities.
 *
 * SPDX-FileCopyrightText: 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020      by Nghia Duong <minhnghiaduong997 at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// C++ includes

#include <vector>

// Qt includes

#include <QFile>
#include <QString>
#include <QStandardPaths>

// Local includes

#include "digikam_config.h"
#include "digikam_opencv.h"
#include "identity.h"
#include "facedbbackend.h"
#include "facescansettings.h"

namespace Digikam
{

class KDTreeBase;

class FaceDb
{
public:

    explicit FaceDb(FaceDbBackend* const db, FaceScanSettings::FaceRecognitionModel recModel);
    ~FaceDb();

    BdEngineBackend::QueryState setSetting(const QString& keyword, const QString& value);
    QString setting(const QString& keyword)                                     const;

    /**
     * @return true if the integrity of the database is preserved.
     */
    bool integrityCheck();

    /**
     * @brief Shrinks the database.
     */
    void vacuum();

public:

    // --- Identity management (facedb_identity.cpp)

    int  addIdentity()                                                          const;
    int  getNumberOfIdentities()                                                const;

    void updateIdentity(const Identity& p);
    void deleteIdentity(int id);
    void deleteIdentity(const QString& uuid);
    void clearIdentities();

    Identity        identity(int id)                                            const;
    QList<Identity> identities()                                                const;
    QList<int>      identityIds()                                               const;

public:

    // --- OpenCV DNN

    /**
     * @brief insertFaceVector: insert a new face embedding to database.
     * @param faceEmbedding
     * @param label
     * @param hash
     * @return id of newly inserted entry.
     */
    int insertFaceVector(const cv::Mat& faceEmbedding,
                         const int label,
                         const QString& hash)                                   const;

    /**
     * @brief removeFaceVector: remove a face embedding from the database.
     * @param hash the removeHash (removeHash) to remove.
     * @return bool
     */
    bool removeFaceVector(const QString& hash)                                  const;

    /**
     * @brief removeFaceVector: remove a face embedding from the database.
     * @param id the nodeId (row id) to remove.
     * @return bool
     */
    bool removeFaceVector(const int id)                                         const;

    /**
     * @brief trainData: extract train data from database.
     * @return the train data instance.
     */
    cv::Ptr<cv::ml::TrainData> trainData()                                      const;

    /**
     * @brief clearDNNTraining: clear all trained data in the database.
     */
    void clearDNNTraining();

    /**
     * @brief clearDNNTraining: clear @param identities in the database.
     */
    void clearDNNTraining(const QList<int>& identities);

    /**
     * @brief get the digiKam version and extractor model the DB was trained with.
     */
    void getTrainingVersionInfo(QString& version, QString& model)                      const;

    /**
     * @brief set the digiKam version and extractor model the DB was trained with.
     */
    void setTrainingVersionInfo(const QString& version, const QString& model);

private:

    /// @note disabled
    FaceDb(const FaceDb&)            = delete;
    FaceDb& operator=(const FaceDb&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
