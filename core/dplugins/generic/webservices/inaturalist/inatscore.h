/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-03-20
 * Description : a tool to export images to iNaturalist web service
 *
 * SPDX-FileCopyrightText: 2021      by Joerg Lohse <joergmlpts at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QPair>

// Local includes

#include "inattaxon.h"

namespace DigikamGenericINatPlugin
{

class ComputerVisionScore
{
public:

    ComputerVisionScore();

    ComputerVisionScore(double frequencyStore, double visionScore,
                        double combinedScore, const Taxon& taxon);

    ComputerVisionScore(const ComputerVisionScore&);

    ~ComputerVisionScore();

    ComputerVisionScore& operator=(const ComputerVisionScore&);

    bool isValid()              const;

    bool visuallySimilar()      const;

    bool seenNearby()           const;

    double getFrequencyScore()  const;

    double getVisionScore()     const;

    double getCombinedScore()   const;

    const Taxon& getTaxon()     const;

private:

    class Private;
    Private* const d = nullptr;

    class Node;
};

typedef QPair<QString, QList<ComputerVisionScore> > ImageScores;

} // namespace DigikamGenericINatPlugin
