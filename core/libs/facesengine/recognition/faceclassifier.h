/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Face classifier
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "faceclassifierbase.h"
#include "facescansettings.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT FaceClassifier : public FaceClassifierBase
{
    Q_OBJECT

public:

    FaceClassifier();
    ~FaceClassifier()                                                         override;

    static FaceClassifier* instance();

    /**
     * @brief Tunes backend parameters.
     *
     * Available parameters:
     *
     * "accuracy", synonymous: "threshold", range: 0-1, type: float
     * Determines recognition threshold, 0->accept very insecure recognitions, 1-> be very sure about a recognition.
     *
     * "recognizeModel" : sets the recognizer model used to instantiate the correct recognizer
     */
    void setParameters(const FaceScanSettings& parameters);

    bool ready()                                                        const;

    int predict(const cv::Mat& target,
                const QList<int>& exclusionLabelList = QList<int>())    const override;
    int predict(const cv::UMat& target, 
                const QList<int>& exclusionLabelList = QList<int>())    const override;

    bool retrain()                                                            override;

    void cancel();

Q_SIGNALS:

    /**
     * @brief Emitted when the training process is complete.
     */
    void signalTrainingComplete();

protected:

    /**
     * arbitrary max distance to start calculating distance.
     */
    const float MAX_DISTANCE = 1000.0F;

protected:

    bool loadTrainingData()                                                   override;

private:

    cv::Ptr<cv::ml::KNearest> createKNearest() const;
    cv::Ptr<cv::ml::SVM>      createSVM(int iterations) const;

    int                       predictFullSearch(const cv::Mat& target, const QList<int>& exclusionLabelList)        const;
    int                       predictClassifier(const cv::Mat& target, const QList<int>& exclusionLabelList)        const;
    int                       listSearch(const cv::Mat& target,
                                         const QHash<int, QList<cv::Mat> >& identityFeatures,
                                         const QList<int>& exclusionLabelList)                                      const;

    bool                      validateKNNSVMResult(const cv::Mat& target, int label)                                const;
    bool                      featureSFaceCompare(const cv::Mat& target, const cv::Mat& sample, float& distance)    const;

private:

    class Private;
    Private* const d = nullptr;

private:

    /// @note disabled
    explicit FaceClassifier(QObject*)     = delete;
    FaceClassifier(const FaceClassifier&) = delete;
};

} // namespace Digikam
