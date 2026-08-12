/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Foundation for all machine learning classifiers
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QReadWriteLock>
#include <QMap>
#include <QList>

// Local includes

#include "digikam_export.h"
#include "digikam_opencv.h"

namespace Digikam
{

class DIGIKAM_EXPORT MLClassifierFoundation : public QObject
{
    Q_OBJECT

public:

    MLClassifierFoundation();
    virtual ~MLClassifierFoundation()                                               = default;

public:

    virtual int predict(const cv::Mat& target,
                        const QList<int>& exclusionLabelList = QList<int>()) const  = 0;
    virtual int predict(const cv::UMat& target,
                        const QList<int>& exclusionLabelList = QList<int>()) const  = 0;

    virtual QList<int> predictMulti(const QList<cv::Mat>& targets)  const           = 0;
    virtual QList<int> predictMulti(const QList<cv::UMat>& targets) const           = 0;

    virtual bool retrain()                                                          = 0;

    void setThreshold(float _threshold)
    {
        threshold = _threshold;
    }

protected:

    class DIGIKAM_EXPORT VotingGroups
    {
    public:

        enum _WinnerType
        {
            VotesLowScore,
            VotesHighScore,
            LowScore,
            HighScore
        }
        typedef WinnerType;

        struct VoteTally
        {
            int     label = 0;
            int     votes = 0;
            float   score = 0.0F;
        };

        struct WinnerVotesLowScore;
        struct WinnerVotesHighScore;
        struct WinnerLowScore;
        struct WinnerHighScore;

    public:

        VotingGroups()  = default;
        ~VotingGroups() = default;

    public:

        void addVote(int label, float score);
        int  winner(WinnerType winnerType);

    private:

        QMap<int, QPair<int, float> > votes;
    };

    QReadWriteLock  lock;
    float           threshold = 0.0F;

protected:

    virtual bool loadTrainingData()                                     = 0;

private:

    /// @note disabled
    explicit MLClassifierFoundation(QObject*)                           = delete;
    MLClassifierFoundation(const MLClassifierFoundation&)               = delete;
    MLClassifierFoundation& operator=(const MLClassifierFoundation&)    = delete;
};

} // namespace Digikam
