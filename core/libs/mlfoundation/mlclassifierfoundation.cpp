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

#include "mlclassifierfoundation.h"

namespace Digikam
{

MLClassifierFoundation::MLClassifierFoundation()
    : QObject()
{
}

void MLClassifierFoundation::VotingGroups::addVote(int label, float score)
{
    if (!votes.contains(label))
    {
        votes.insert(label, QPair<int, float>(0, 0.0f));
    }

    votes[label] = QPair<int, float>(votes.value(label).first + 1, votes.value(label).second + score);
}

struct Q_DECL_HIDDEN MLClassifierFoundation::VotingGroups::WinnerVotesLowScore
{
    bool operator()(const MLClassifierFoundation::VotingGroups::VoteTally& a,
                    const MLClassifierFoundation::VotingGroups::VoteTally& b) const
    {
        if (a.votes == b.votes)
        {
            return (a.score < b.score);
        }

        return (a.votes > b.votes);
    }
};

struct Q_DECL_HIDDEN MLClassifierFoundation::VotingGroups::WinnerVotesHighScore
{
    bool operator()(const MLClassifierFoundation::VotingGroups::VoteTally& a,
                    const MLClassifierFoundation::VotingGroups::VoteTally& b) const
    {
        // May want to check that the pointers aren't zero...

        if (a.votes == b.votes)
        {
            return (a.score > b.score);
        }

        return (a.votes > b.votes);
    }
};

struct Q_DECL_HIDDEN MLClassifierFoundation::VotingGroups::WinnerLowScore
{
    bool operator()(const MLClassifierFoundation::VotingGroups::VoteTally& a,
                    const MLClassifierFoundation::VotingGroups::VoteTally& b) const
    {
        // May want to check that the pointers aren't zero...

        return (a.score < b.score);
    }
};

struct Q_DECL_HIDDEN MLClassifierFoundation::VotingGroups::WinnerHighScore
{
    bool operator()(const MLClassifierFoundation::VotingGroups::VoteTally& a,
                    const MLClassifierFoundation::VotingGroups::VoteTally& b) const
    {
        // May want to check that the pointers aren't zero...

        return (a.score > b.score);
    }
};

int MLClassifierFoundation::VotingGroups::winner(WinnerType winnerType)
{
    if (0 == votes.count())
    {
        return -1;
    }

    QList<VoteTally> voteTally;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    for (auto [key, value] : votes.asKeyValueRange())
    {

#else

    for (const auto& key : votes.keys())
    {
        const auto& value = votes[key];

#endif

        VoteTally t;
        t.label = key;
        t.votes = value.first;
        t.score = value.second;
        voteTally << t;
    }

    switch (winnerType)
    {
        case WinnerType::VotesLowScore:
        {
            std::sort(voteTally.begin(), voteTally.end(), WinnerVotesLowScore());
            break;
        }

        case WinnerType::VotesHighScore:
        {
            std::sort(voteTally.begin(), voteTally.end(), WinnerVotesHighScore());
            break;
        }

        case WinnerType::LowScore:
        {
            std::sort(voteTally.begin(), voteTally.end(), WinnerLowScore());
            break;
        }

        case WinnerType::HighScore:
        {
            std::sort(voteTally.begin(), voteTally.end(), WinnerHighScore());
            break;
        }
    }

    return voteTally.value(0).label;
}

} // namespace Digikam

#include "moc_mlclassifierfoundation.cpp"
