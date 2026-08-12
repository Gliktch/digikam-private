/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-03
 * Description : Face recognition benchmarker
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "recognitionbenchmarker.h"

// Local includes

#include "digikam_debug.h"
#include "tagscache.h"

namespace Digikam
{

RecognitionBenchmarker::RecognitionBenchmarker(FacePipeline::Private* const dd)
    : d(dd)
{
}

/**
 * NOTE: Bench performance code. No need i18n here
 */
QString RecognitionBenchmarker::result() const
{
    int totalImages = 0;

    for (const Statistics& stat : std::as_const(results))
    {
        // cppcheck-suppress useStlAlgorithm
        totalImages += stat.knownFaces;
    }

    QString s = QString::fromUtf8("<p>"
                                  "<u>Collection Properties:</u><br>"
                                  "%1 Images <br>"
                                  "%2 Identities <br>"
                                  "</p><p>").arg(totalImages).arg(results.size());

    for (QMap<int, Statistics>::const_iterator it = results.begin() ;
         it != results.end() ; ++it)
    {
        const Statistics& stat = it.value();
        double correctRate     = double(stat.correctlyRecognized) / stat.knownFaces;
        s                     += TagsCache::instance()->tagName(it.key());
        s                     += QString::fromUtf8(": %1 faces, %2 (%3%) correctly recognized<br>")
                                 .arg(stat.knownFaces).arg(stat.correctlyRecognized).arg(correctRate * 100);
    }

    s += QLatin1String("</p>");

    return s;
}

// TODO: investigate this method

void RecognitionBenchmarker::process(const FacePipelineExtendedPackage::Ptr& package)
{
    FaceUtils utils;

    for (int i = 0 ; i < package->databaseFaces.size() ; ++i)
    {
/*
        Identity identity  = utils.identityForTag(package->databaseFaces[i].tagId(), recognizer);
*/
        Identity identity  = utils.identityForTag(package->databaseFaces[i].tagId());
        Statistics& res    = results[package->databaseFaces[i].tagId()];
        res.knownFaces++;

        if (identity == package->recognitionResults[i])
        {
            res.correctlyRecognized++;
        }
    }

    Q_EMIT processed(package);
}

} // namespace Digikam

#include "moc_recognitionbenchmarker.cpp"
