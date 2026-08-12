/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-03
 * Description : Face detection benchmarker
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "detectionbenchmarker.h"

// Local includes

#include "digikam_debug.h"
#include "tagscache.h"

namespace Digikam
{

DetectionBenchmarker::DetectionBenchmarker(FacePipeline::Private* const dd)
    : d(dd)
{
}

void DetectionBenchmarker::process(const FacePipelineExtendedPackage::Ptr& package)
{
    if (package->databaseFaces.isEmpty())
    {
        // Detection / Recognition

        qCDebug(DIGIKAM_GENERAL_LOG) << "Benchmarking image" << package->info.name();

        FaceUtils utils;
        QList<FaceTagsIface> groundTruth          = utils.databaseFaces(package->info.id());

        QList<FaceTagsIface> testedFaces          = utils.toFaceTagsIfaces(package->info.id(),
                                                                           package->detectedFaces,
                                                                           package->recognitionResults,
                                                                           package->image.originalSize());

        QList<FaceTagsIface> unmatchedTrueFaces   = groundTruth;
        QList<FaceTagsIface> unmatchedTestedFaces = testedFaces;
        QList<FaceTagsIface> matchedTrueFaces;

        int trueFaces                             = groundTruth.size();
        const double minOverlap                   = 0.75;

        qCDebug(DIGIKAM_GENERAL_LOG) << "There are" << trueFaces
                                     << "faces to be detected. The detector found" << testedFaces.size();

        ++totalImages;
        faces       += trueFaces;
        totalPixels += package->image.originalSize().width() * package->image.originalSize().height();

        for (const FaceTagsIface& trueFace : std::as_const(groundTruth))
        {
            ++faces;
            QRect rect  = trueFace.region().toRect();
            facePixels += rect.width() * rect.height();

            for (const FaceTagsIface& testedFace : std::as_const(testedFaces))
            {
                if (trueFace.region().intersects(testedFace.region(), minOverlap))
                {
                    // cppcheck-suppress useStlAlgorithm
                    matchedTrueFaces << trueFace;
                    unmatchedTrueFaces.removeOne(trueFace);

                    break;
                }
            }
        }

        for (const FaceTagsIface& testedFace : std::as_const(testedFaces))
        {
            for (const FaceTagsIface& trueFace : std::as_const(groundTruth))
            {
                if (trueFace.region().intersects(testedFace.region(), minOverlap))
                {
                    // cppcheck-suppress useStlAlgorithm
                    unmatchedTestedFaces.removeOne(testedFace);

                    break;
                }
            }
        }

        if (groundTruth.isEmpty())
        {
            if (testedFaces.isEmpty())
            {
                ++trueNegativeImages;
            }

            else
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "The image, truly without faces, is false-positive";
                ++falsePositiveImages;
            }
        }

        truePositiveFaces  += matchedTrueFaces.size();
        falseNegativeFaces += unmatchedTrueFaces.size();
        falsePositiveFaces += unmatchedTestedFaces.size();

        qCDebug(DIGIKAM_GENERAL_LOG) << "Faces detected correctly:"
                                     << matchedTrueFaces.size()
                                     << ", faces missed:"
                                     << unmatchedTrueFaces.size()
                                     << ", faces falsely detected:"
                                     << unmatchedTestedFaces.size();
    }

    package->processFlags  |= FacePipelinePackage::WrittenToDatabase;

    Q_EMIT processed(package);
}

/**
 * NOTE: Bench performance code. No need i18n here
 */
QString DetectionBenchmarker::result() const
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Per-image:"
                                 << trueNegativeImages
                                 << falsePositiveFaces;

    qCDebug(DIGIKAM_GENERAL_LOG) << "Per-face:"
                                 << truePositiveFaces
                                 << falseNegativeFaces
                                 << falsePositiveFaces; // 26 7 1

    int negativeImages = trueNegativeImages + falsePositiveImages;
    int trueFaces      = truePositiveFaces  + falseNegativeFaces;
    QString specificityWarning, sensitivityWarning;

    if (negativeImages < (0.2 * totalImages))
    {
        specificityWarning = QString::fromUtf8("<p><b>Note:</b><br> "
                                               "Only %1 of the %2 test images have <i>no</i> depicted faces. "
                                               "This means the result is cannot be representative; "
                                               "it can only be used to compare preselected collections, "
                                               "and the specificity and false-positive rate have little meaning. </p>")
                             .arg(negativeImages).arg(totalImages);
        negativeImages     = qMax(negativeImages, 1);
    }

    if (trueFaces == 0)
    {
        sensitivityWarning = QString::fromUtf8("<p><b>Note:</b><br> "
                                               "No picture in the test collection contained a face. "
                                               "This means that sensitivity and PPV have no meaning and will be zero. </p>");
        trueFaces          = 1;
    }

    // collection properties

    double pixelCoverage     = facePixels                  / totalPixels;

    // per-image

    double specificity       = double(trueNegativeImages)  / negativeImages;
    double falsePositiveRate = double(falsePositiveImages) / negativeImages;

    // per-face

    double sensitivity       = double(truePositiveFaces)   / trueFaces;
    double ppv               = double(truePositiveFaces)   / (truePositiveFaces + falsePositiveFaces);

    return QString::fromUtf8("<p>"
                             "<u>Collection Properties:</u><br>"
                             "%1 Images <br>"
                             "%2 Faces <br>"
                             "%3% of pixels covered by faces <br>"
                             "</p>"
                             "%8"
                             "%9"
                             "<p>"
                             "<u>Per-Image Performance:</u> <br>"
                             "Specificity: %4% <br>"
                             "False-Positive Rate: %5%"
                             "</p>"
                             "<p>"
                             "<u>Per-Face Performance:</u> <br>"
                             "Sensitivity: %6% <br>"
                             "Positive Predictive Value: %7% <br>"
                             "</p>"
                             "<p>"
                             "In other words, if a face is detected as face, it will "
                             "with a probability of %7% truly be a face. "
                             "Of all true faces, %6% will be detected. "
                             "Given face with no images on it, the detector will with a probability "
                             "of %5% falsely find a face on it. "
                             "</p>")
           .arg(totalImages).arg(faces).arg(pixelCoverage * 100, 0, 'f', 1)
           .arg(specificity * 100, 0, 'f', 1).arg(falsePositiveRate * 100, 0, 'f', 1)
           .arg(sensitivity * 100, 0, 'f', 1).arg(ppv * 100, 0, 'f', 1)
           .arg(specificityWarning).arg(sensitivityWarning);
}

} // namespace Digikam

#include "moc_detectionbenchmarker.cpp"
