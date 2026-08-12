/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2019-06-01
 * Description : Face recognition using deep learning.
 *               The internal DNN library interface.
 *
 * SPDX-FileCopyrightText: 2019      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dnnfaceextractorbase.h"

// Qt includes

#include <QMutex>
#include <QString>
#include <QFileInfo>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <QStandardPaths>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"

namespace Digikam
{

DNNFaceExtractorBase::DNNFaceExtractorBase()
{
}

DNNFaceExtractorBase::~DNNFaceExtractorBase()
{
}

double DNNFaceExtractorBase::cosineDistance(const std::vector<float>& v1,
                                            const std::vector<float>& v2)
{
    Q_ASSERT(v1.size() == v2.size());

    double scalarProduct = std::inner_product(v1.begin(), v1.end(), v2.begin(), 0.0);
    double normV1        = sqrt(std::inner_product(v1.begin(), v1.end(), v1.begin(), 0.0));
    double normV2        = sqrt(std::inner_product(v2.begin(), v2.end(), v2.begin(), 0.0));

    return (scalarProduct / (normV1 * normV2));
}

double DNNFaceExtractorBase::L2squareDistance(const std::vector<float>& v1,
                                              const std::vector<float>& v2)
{
    Q_ASSERT(v1.size() == v2.size());

    double sqrDistance = 0.0;

    for (size_t i = 0 ; i < v1.size() ; ++i)
    {
        sqrDistance += pow((v1[i] - v2[i]), 2);
    }

    return sqrDistance;
}

double DNNFaceExtractorBase::L2squareNormDistance(const std::vector<float>& v1,
                                                  const std::vector<float>& v2)
{
    Q_ASSERT(v1.size() == v2.size());

    double normV1      = sqrt(std::inner_product(v1.begin(), v1.end(), v1.begin(), 0.0));
    double normV2      = sqrt(std::inner_product(v2.begin(), v2.end(), v2.begin(), 0.0));
    double sqrDistance = 0.0;

    for (size_t i = 0 ; i < v1.size() ; ++i)
    {
        sqrDistance += pow((v1[i] / (normV1 - v2[i]) / normV2), 2);
    }

    return sqrDistance;
}

cv::Mat DNNFaceExtractorBase::vectortomat(const std::vector<float>& vector)
{
    cv::Mat mat(1, vector.size(), 5);

    memcpy(mat.data, vector.data(), vector.size()*sizeof(float));

    return mat;
}

QJsonArray DNNFaceExtractorBase::encodeVector(const std::vector<float>& vector)
{
    QJsonArray array;

    for (size_t i = 0 ; i < vector.size() ; ++i)
    {
        array << vector[i];
    }

    return array;
}

std::vector<float> DNNFaceExtractorBase::decodeVector(const QJsonArray& json)
{
    std::vector<float> vector;

    for (int i = 0 ; i < json.size() ; ++i)
    {
        vector.push_back(static_cast<float>(json[i].toDouble()));
    }

    return vector;
}

} // namespace Digikam
