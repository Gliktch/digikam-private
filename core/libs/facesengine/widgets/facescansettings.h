/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-09
 * Description : Face scan settings container.
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDebug>
#include <QThread>

// Local includes

#include "album.h"
#include "iteminfo.h"
#include "digikam_export.h"
#include "dnnmodeldefinitions.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_GUI_EXPORT FaceScanSettings
{
    Q_GADGET

public:

    /**
     * @brief Different possible tasks processed while scanning operation.
     */
    enum ScanTask
    {
//      Detect,                 ///< Detect faces only.
        DetectAndRecognize,     ///< Detect and recognize faces only.
        RecognizeMarkedFaces,   ///< Recognize already marked faces only.
        RetrainAll,             ///< Retrain faces only.
        Reset
//      BenchmarkDetection,     ///< Bench performances of detect faces.
//      BenchmarkRecognition    ///< Bench performance of recognize faces.
    };
    Q_ENUM(ScanTask)

    /**
     * @brief Scan Mode as detect and recognize.
     */
    enum AlreadyScannedHandling
    {
        Skip,                   ///< Skip faces from images already scanned.
//      Merge,                  ///< Merge faces from images already scanned.
        Rescan,                 ///< Rescan faces from images already scanned.
        ClearAll,               ///< Clear all faces data from images already scanned. Clear identities and training data from FacesDb.
        RecognizeOnly           ///< Recognize faces from images already scanned.
    };
    Q_ENUM(AlreadyScannedHandling)

    /**
     * @brief Face detection AI models.
     */
    enum FaceDetectionModel
    {
        /**
         * YuNet neural network inference.
         * https://github.com/opencv/opencv_zoo/tree/main
         */
        YuNet
    };
    Q_ENUM(FaceDetectionModel)

    /**
     * @brief Face detection size.
     */
    enum FaceDetectionSize
    {
        ExtraSmall,
        Small,
        Medium,
        Large,
        ExtraLarge
    };
    Q_ENUM(FaceDetectionSize)

    /**
     * @brief Face recognition AI models.
     */
    enum FaceRecognitionModel
    {
        /**
         * SFace pre-trained neural network model.
         * https://github.com/opencv/opencv_zoo/blob/main/models/face_recognition_sface/
         */
        SFace
    };
    Q_ENUM(FaceRecognitionModel)

    /**
     * @brief How the FaceScan was started.
     */
    enum FaceScanSource
    {
        FaceScanWidget,
        ItemIconView,
        MaintenanceTool,
        BackgroundRecognition,
        BQM
    };
    Q_ENUM(FaceScanSource)

public:

    FaceScanSettings()  = default;
    FaceScanSettings(const FaceScanSettings& other);
    ~FaceScanSettings() = default;

    FaceScanSettings& operator=(const FaceScanSettings& other);

public:

    void readFromConfig();
    void readFromConfig(const KConfigGroup&);
    void writeToConfig();
    void writeToConfig(KConfigGroup&);

public:

    /**
     * Scanning mode.
     */
    AlreadyScannedHandling                  alreadyScannedHandling           = Skip;

    /**
     * @brief Tasks processed while scanning.
     */
    ScanTask                                task                             = DetectAndRecognize;

    /**
     * @brief Processing using all CPU available.
     */
    bool                                    useFullCpu                       = false;

    /**
     * @brief Detection Model.
     */
    FaceDetectionModel                      detectModel                      = FaceDetectionModel::YuNet;

    /**
     * @brief Face size used by the detection Model.
     */
    FaceDetectionSize                       detectSize                       = FaceDetectionSize::Large;

    /**
     * @brief Detection accuracy.
     * @note Use default value from dnnmodels.conf
     */
    int                                     detectAccuracy                   = DNN_MODEL_THRESHOLD_NOT_SET;

    /**
     * @brief Detection Model.
     */
    FaceRecognitionModel                    recognizeModel                   = FaceRecognitionModel::SFace;

    /**
     * @brief Recognition accuracy.
     * @note Use default value from dnnmodels.conf
     */
    int                                     recognizeAccuracy                = DNN_MODEL_THRESHOLD_NOT_SET;

    /**
     * @brief Whole albums checked.
     */
    bool                                    wholeAlbums                      = false;

    /**
     * @brief Albums to scan.
     */
    AlbumList                               albums;

    /**
     * @brief Image infos to scan.
     */
    ItemInfoList                            infos;

    /**
     * @brief Worker thread priority.
     * @note Most of the time, this is set to LowPriority.
     *       The BackgroundRecognizer is set to IdlePriority.
     *       Do not save this value in the config file.
     */
    QThread::Priority                       workerThreadPriority            = QThread::LowPriority;

    /**
     * @brief Source of the scan.
     */
    FaceScanSource                          source                           = FaceScanWidget;

private:

    const QString                           configName                       = QLatin1String("Face Management Settings");
    const QString                           configFaceAlreadyScannedHandling = QLatin1String("Face Already Scanned Handling");
    const QString                           configFaceScanTask               = QLatin1String("Face Scan Task");
    const QString                           configUseFullCpu                 = QLatin1String("Face Use Full CPU");
    const QString                           configFaceDetectionAccuracy      = QLatin1String("Face Detection Accuracy");
    const QString                           configFaceDetectionModel         = QLatin1String("Face Detection Model");
    const QString                           configFaceDetectionSize          = QLatin1String("Face Detection Size");
    const QString                           configFaceRecognitionAccuracy    = QLatin1String("Face Recognition Accuracy");
    const QString                           configFaceRecognitionModel       = QLatin1String("Face Recognition Model");
};

//! qDebug() stream operator. Writes property @a s to the debug output in a nicely formatted way.
DIGIKAM_GUI_EXPORT QDebug operator<<(QDebug dbg, const FaceScanSettings& s);

} // namespace Digikam
