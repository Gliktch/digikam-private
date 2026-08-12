/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Integrated, multithread object detection / recognition
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "autotagsclassifiermultiyolo.h"

// C++ includes

#include <vector>

// Qt includes

#include <QList>
#include <QRect>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

/**
 * @note: In strict technical terms, this is not a classifier. This is a post-processor.
 * The initial classification is done by the YOLO detector, and this class is used to
 * interpret the results of that classification and provide a list of labels
 * based on the confidence of the classification.
 */
AutotagsClassifierYolo::AutotagsClassifierYolo(float _threshold, const QString& predefinedClassFile)
    : AutotagsClassifierBase(_threshold, predefinedClassFile)
{
}

void AutotagsClassifierYolo::setParams(YoloVersion version, QSize size)
{
    /// @warning: This classifier does not handle YOLO v10

    yoloVersion = version;
    imageSize   = size;
    ready       = (predefinedClasses.size() > 0);

    if ((yoloVersion == YOLOv5) || (yoloVersion == YOLOv7))
    {
        outputNumprob = 5 + predefinedClasses.size();
        outputNumbox  = 3 * (
                             (imageSize.width() / 8 ) * (imageSize.height() / 8 ) +
                             (imageSize.width() / 16) * (imageSize.height() / 16) +
                             (imageSize.width() / 32) * (imageSize.height() / 32)
                            );
    }
    if (yoloVersion == YOLOv6)
    {
        outputNumprob = 5 + predefinedClasses.size();
        outputNumbox  = (
                         (imageSize.width() / 8 ) * (imageSize.height() / 8 ) +
                         (imageSize.width() / 16) * (imageSize.height() / 16) +
                         (imageSize.width() / 32) * (imageSize.height() / 32)
                        );
    }
    if ((yoloVersion == YOLOv8) || (yoloVersion == YOLOv9) || (yoloVersion == YOLOv11))
    {
        outputNumprob = 4 + predefinedClasses.size();
        outputNumbox  = (
                         (imageSize.width() / 8 ) * (imageSize.height() / 8 ) +
                         (imageSize.width() / 16) * (imageSize.height() / 16) +
                         (imageSize.width() / 32) * (imageSize.height() / 32)
                        );
    }
}

QList<int> AutotagsClassifierYolo::predictMulti(const QList<cv::Mat>& targets)  const
{
    /// @warning: This classifier does not handle YOLOv10

    QList<int> results;

    // check if the classifier is ready

    if (!ready)
    {
        qCDebug(DIGIKAM_AUTOTAGSENGINE_LOG) << "AutotagsClassifierYolo::predictMulti: Not ready. Parameters not set";

        return results;
    }

    cv::Mat cvTranspose;
    std::vector<cv::Rect> boxes;
    std::vector<float>    scores;
    std::vector<int>      class_ids;
    std::vector<float>    objnesses;
    float*                outputHost = nullptr;

    if (targets[0].dims > 2)
    {
        // YOLOv8 and YOLOv11 require reshaping the output

        // check we have a valid cv::Mat

        if ((targets[0].size[1] - 4) != predefinedClasses.size())
        {
            qCDebug(DIGIKAM_AUTOTAGSENGINE_LOG) << "AutotagsClassifierYolo::predictMulti: Invalid cv::Mat size";

            return results;
        }

        cv::Mat cvReshape;
        std::vector<int> shape = { targets[0].size[1], targets[0].size[2] };
        cvReshape              = targets[0].reshape(1, shape);
        cv::transpose(cvReshape, cvTranspose);
        outputHost             = reinterpret_cast<float*>(cvTranspose.data);
    }
    else
    {
        outputHost = reinterpret_cast<float*>(targets[0].data);
    }

    for (int i = 0 ; i < outputNumbox ; ++i)
    {
        float* ptr    = outputHost + i * outputNumprob;
        int class_id  = -1;
        float score   = -1.0F;
        float objness = -1.0F;

        if ((yoloVersion == YOLOv5) || (yoloVersion == YOLOv6) || (yoloVersion == YOLOv7))
        {
            objness = ptr[4];

            if (objness < objThreshold)
            {
                continue;
            }

            float* classes_scores = ptr + 5;
            class_id              = std::max_element(classes_scores, classes_scores + predefinedClasses.size()) - classes_scores;
            score                 = classes_scores[class_id] * objness;
        }

        if ((yoloVersion == YOLOv8) || (yoloVersion == YOLOv9) || (yoloVersion == YOLOv11))
        {
            float* classes_scores = ptr + 4;
            class_id              = std::max_element(classes_scores, classes_scores + predefinedClasses.size()) - classes_scores;
            score                 = classes_scores[class_id];
        }

        // check the score

        if (score > 1.0F)
        {
            // any score greater than 1.0 is invalid, and all scores are ignored

            qCDebug(DIGIKAM_AUTOTAGSENGINE_LOG) << "AutotagsClassifierYolo::predictMulti: invalid score: " << score;

            return results;
        }

        if (score < scoreThreshold)
        {
            continue;
        }

        cv::Rect box;

        if (
            (yoloVersion == YOLOv5) || (yoloVersion == YOLOv6 ) ||
            (yoloVersion == YOLOv7) || (yoloVersion == YOLOv8 ) ||
            (yoloVersion == YOLOv9) || (yoloVersion == YOLOv11)
           )
        {
            float x    = ptr[0];
            float y    = ptr[1];
            float w    = ptr[2];
            float h    = ptr[3];
            int left   = int(x - 0.5 * w);
            int top    = int(y - 0.5 * h);
            int width  = int(w);
            int height = int(h);
            box        = cv::Rect(left, top, width, height);
        }
/*
        scale_box(box, m_image.size());
*/
        boxes.push_back(box);
        scores.push_back(score);
        class_ids.push_back(class_id);
        objnesses.push_back(objness);
    }

    if (
        (yoloVersion == YOLOv5) || (yoloVersion == YOLOv6 ) ||
        (yoloVersion == YOLOv7) || (yoloVersion == YOLOv8 ) ||
        (yoloVersion == YOLOv9) || (yoloVersion == YOLOv11)
       )
    {
        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, scores, scoreThreshold, nmsThreshold, indices);

        for (int i = 0 ; i < (int)indices.size() ; i++)
        {
            int idx         = indices[i];
            bool saveResult = false;

            if ((yoloVersion == YOLOv5) || (yoloVersion == YOLOv6) || (yoloVersion == YOLOv7))
            {
                saveResult = ((objnesses[idx] >= threshold) && (scores[idx] >= confidenceThreshold));
            }
            else
            {
                saveResult = (scores[idx] >= threshold);
            }

            // save the result to the return list

            if (!results.contains(class_ids[idx]) && saveResult)
            {
                results << class_ids[idx];
            }

            // debug log classifier decisions

            if (saveResult)
            {
                qCDebug(DIGIKAM_AUTOTAGSENGINE_LOG) << "New cls - Class ID:"
                                                    << class_ids[idx]
                                                    << predefinedClasses[class_ids[idx]]
                                                    << "Confidence:"
                                                    << scores[idx]
                                                    << "Objness:"
                                                    << objnesses[idx];
            }
            else
            {
                qCDebug(DIGIKAM_AUTOTAGSENGINE_LOG) << "New cls - Class ID:"
                                                    << class_ids[idx]
                                                    << predefinedClasses[class_ids[idx]]
                                                    << "Confidence:"
                                                    << scores[idx]
                                                    << "Objness:"
                                                    << objnesses[idx]
                                                    << "below threshold";
            }
        }
    }

    return results;
}

} // namespace Digikam
