/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-07
 * Description : Similarity database interface.
 *
 * SPDX-FileCopyrightText:      2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2017 by Swati Lodha    <swatilodha27 at gmail dot com>
 * SPDX-FileCopyrightText:      2018 by Mario Frank    <mario dot frank at uni minus potsdam dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QStringList>
#include <QString>
#include <QList>
#include <QPair>
#include <QSet>

// Local includes

#include "similaritydbbackend.h"
#include "similaritydbaccess.h"
#include "digikam_export.h"
#include "iteminfo.h"

namespace Digikam
{

enum class FuzzyAlgorithm
{
    Unknown = 0,
    Haar    = 1,
    TfIdf   = 2
};


class DIGIKAM_DATABASE_EXPORT SimilarityDb
{
public:

    /**
     * Set the database setting entry given by keyword to the given value.
     * @param keyword The keyword, i.e. setting name.
     * @param value The value.
     * @return True, if the value was set and false, else..
     */
    bool setSetting(const QString& keyword, const QString& value);

    /**
     * Returns the setting with the keyword name.
     * @param keyword The setting entry name.
     * @return The setting value.
     */
    QString getSetting(const QString& keyword);

    /**
     * Returns the legacy settings with the keyword name.
     * @param keyword The setting entry name.
     * @return The setting value.
     */
    QString getLegacySetting(const QString& keyword);

    /**
     * This method returns all image ids that are present in the similarity db tables.
     * \return a set of all present image ids.
     */
    QSet<qlonglong> registeredImageIds() const;

    // ----------- Methods for fingerprint (ImageHaarMatrix) table access ----------

    /**
     * This method checks if the given image has a fingerprint for the given algorithm.
     *
     * @param imageId The Id of the image to check.
     * @param algorithm The algorithm.
     * @return True, if there is a fingerprint.
     */
    bool hasFingerprint(qlonglong imageId, FuzzyAlgorithm algorithm) const;

    /**
     * This method checks if there are any fingerprints for any algorithm present.
     * @return True, if fingerprints exist.
     */
    bool hasFingerprints();

    /**
     * This method checks if there are any fingerprints for the given algorithm.
     * @param algorithm The algorithm.
     * @return true, if there are fingerprints and false, otherwise.
     */
    bool hasFingerprints(FuzzyAlgorithm algorithm) const;

    /**
     * Checks if the given image has a dirty fingerprint or even none for the given algorithm.
     *
     * @param imageInfo The image info object representing the item.
     * @param algorithm The algorithm used for the fingerprint.
     * @return True, if the image either has no or a dirty fingerprint.
     */
    bool hasDirtyOrMissingFingerprint(const ItemInfo& imageInfo,
                                      FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar) const;

    /**
     * Returns a list of all item ids (images, videos,...) where either no fingerprint for the given
     * algorithm exists or is outdated because the file is identified as changed since
     * the generation of the fingerprint.
     * @param imageInfos The image info objects representing the items.
     * @param algorithm The algorithm.
     * @return The ids of the items whose fingerprints are dirty or missing.
     */
    QList<qlonglong> getDirtyOrMissingFingerprints(const QList<ItemInfo>& imageInfos,
                                                   FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    /**
     * Returns a list of the URLs of all items (images, videos,...) where either no fingerprint for the given
     * algorithm exists or is outdated because the file is identified as changed since
     * the generation of the fingerprint.
     * @param imageInfos The image info objects representing the items.
     * @param algorithm The algorithm.
     * @return The URLs of the items whose fingerprints are dirty or missing.
     */
    QStringList      getDirtyOrMissingFingerprintURLs(const QList<ItemInfo>& imageInfos,
                                                      FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    /**
     * This method removes the fingerprint entry for the given imageId and algorithm.
     * Also, this automatically removes the entries in the ImageSimilarities table for the
     * given algorithm and image id.
     * @param imageID The image id.
     * @param algorithm The algorithm.
     */
    void removeImageFingerprint(qlonglong imageID,
                                FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    /**
     * Copies all similarity-specific information, from image srcId to destId.
     */
    void copySimilarityAttributes(qlonglong srcId,
                                  qlonglong destId);

    // ----------- Methods for image similarity table access ----------

    /**
     * Returns the similarity value for two images.
     * A value of -1 means nonexistence.
     * A value of -2 means that there is a value that cannot be converted into a double
     */
    double getImageSimilarity(qlonglong imageID1,
                              qlonglong imageID2,
                              FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    void setImageSimilarity(qlonglong imageID1,
                            qlonglong imageID2,
                            double value,
                            FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    /**
     * This method removes the image similarity entries for the imageID and algorithm.
     * @param imageID The image id.
     * @param algorithm The algorithm.
     */
    void removeImageSimilarity(qlonglong imageID,
                               FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    /**
     * This method removes the image similarity entry for the imageIDs and algorithm.
     * @param imageID1 The first image id.
     * @param imageID2 The second image id.
     * @param algorithm The algorithm.
     */
    void removeImageSimilarity(qlonglong imageID1,
                               qlonglong imageID2,
                               FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    /**
     * This method removes all image similarity entries for the algorithm.
     * @param algorithm The algorithm.
     */
    void clearImageSimilarity(FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    /**
     * Returns the algorithms for which a similarity value exists for the given image ids.
     * @param imageID1 The first image id.
     * @param imageID2 The second image id.
     * @return a list of all algorithms for which a similarity value exists.
     */
    QList<FuzzyAlgorithm> getImageSimilarityAlgorithms(qlonglong imageID1,
                                                       qlonglong imageID2);

    // ----------- Database shrinking and integrity check methods ----------

    /**
     * This method checks the integrity of the similarity database.
     * @return true, if the integrity check was passed and false, else.
     */
    bool integrityCheck();

    /**
     * This method shrinks the database.
     */
    void vacuum();

private:

    /**
     * This private variant of getImageSimilarity assumes that imageID1 <= imageID2
     * @param imageID1 the id of the first image.
     * @param imageID2 the id of the second image.
     * @param algorithm the algorithm
     * @return An empty string, if no similarity value exists for the ids and the algorithm, and the value as string, else.
     */
    QString getImageSimilarityOrdered(qlonglong imageID1,
                                      qlonglong imageID2,
                                      FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    /**
     * This method applies a partial ordering to id1 and id2, i.e.
     * the result is a pair where the first is id1 iff id1 <= id2 and id2 otherwise.
     * @param id1 The first id.
     * @param id2 The second id.
     * @return A pair in which the ids are in partial ascending order.
     */
    QPair<qlonglong, qlonglong> orderIds(qlonglong id1, qlonglong id2);

    /**
     * The constructor of the similarity db class
     * @param backend The database backend.
     */
    explicit SimilarityDb(SimilarityDbBackend* const backend);

    /**
     * The destructor of the similarity db class.
     */
    ~SimilarityDb();

private:

    /// @note disabled
    SimilarityDb(const SimilarityDb&)            = delete;
    SimilarityDb& operator=(const SimilarityDb&) = delete;

    class Private;
    Private* const d = nullptr;

    friend class SimilarityDbAccess;
};

} // namespace Digikam
