/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-01-17
 * Description : Haar Database interface
 *
 * SPDX-FileCopyrightText: 2016-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 * SPDX-FileCopyrightText: 2003      by Ricardo Niederberger Cabral <nieder at mail dot ru>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QSet>
#include <QString>
#include <QMap>
#include <QList>
#include <QImage>

// Local includes

#include "haar.h"
#include "iteminfo.h"
#include "digikam_export.h"

class QImage;

namespace Digikam
{

class DImg;
class ItemInfo;

class HaarProgressObserver
{
public:

    HaarProgressObserver()                                           = default;
    virtual ~HaarProgressObserver()                                  = default;

    virtual void imageProcessed(const ItemInfo&, const QImage&, int) = 0;

    virtual bool isCanceled()
    {
        return false;
    };

private:

    Q_DISABLE_COPY(HaarProgressObserver)
};

// --------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT HaarIface
{
public:

    enum SketchType
    {
        ScannedSketch   = 0,
        HanddrawnSketch = 1
    };

    enum DuplicatesSearchRestrictions
    {
        None            = 0,
        SameAlbum       = 1,
        DifferentAlbum  = 2
    };

    enum AlbumTagRelation
    {
        NoMix           = 0,
        Union           = 1,
        Intersection    = 2,
        AlbumExclusive  = 3,
        TagExclusive    = 4
    };

    /**
     * @brief The RefImageSelMethod enum
     * Selection method to determine which image
     * will be the reference in the duplicate search
     *
     * When adding method here, update also
     * HaarIface::findDuplicates()
     */
    enum class RefImageSelMethod: unsigned int
    {
        OlderOrLarger         = 0,      ///< Original
        PreferFolder          = 1,      ///< Prefer select folder to be the reference
        ExcludeFolder         = 2,      ///< Prefer image not in the selected folder
        NewerCreationDate     = 3,      ///< Prefer newer creation date image
        NewerModificationDate = 4       ///< Prefer newer modification date image
    };

    using DuplicatesResultsMap = QMap<qlonglong, QPair<double, QList<qlonglong> > >;

public:

    explicit HaarIface();
    explicit HaarIface(const QSet<qlonglong>& images2Scan);
    ~HaarIface();

    static int preferredSize();

    /**
     * @brief Adds an image to the index in the database.
     */
    bool indexImage(const QString& filename);
    bool indexImage(const QString& filename, const QImage& image);
    bool indexImage(const QString& filename, const DImg& image);
    bool indexImage(qlonglong imageid, const QImage& image);
    bool indexImage(qlonglong imageid, const DImg& image);

    QMap<qlonglong, double> bestMatchesForSignature(const QString& signature,
                                                    const QList<int>& targetAlbums,
                                                    int numberOfResults = 20,
                                                    SketchType type = ScannedSketch);

    /**
     * @brief Searches the database for the best matches for the specified query image.
     * All matches with a similarity in a given threshold interval are returned.
     * The threshold is in the range requiredPercentage..maximumPercentage.
     */
    QPair<double, QMap<qlonglong, double> > bestMatchesForImageWithThreshold(
        qlonglong imageid,
        double requiredPercentage,
        double maximumPercentage,
        const QList<int>& targetAlbums,
        DuplicatesSearchRestrictions searchResultRestriction = DuplicatesSearchRestrictions::None,
        SketchType type = ScannedSketch
    );

    /**
     * @brief Searches the database for the best matches for the specified query image.
     * All matches with a similarity in a given threshold interval are returned.
     * The threshold is in the range requiredPercentage..maximumPercentage.
     */
    QPair<double, QMap<qlonglong, double> > bestMatchesForImageWithThreshold(
        const QString& imagePath,
        double requiredPercentage,
        double maximumPercentage,
        const QList<int>& targetAlbums,
        DuplicatesSearchRestrictions searchResultRestriction = DuplicatesSearchRestrictions::None,
        SketchType type = ScannedSketch
    );

    /**
     * @brief Calculates the Haar signature, bring it in a form as stored in the DB,
     * and encode it to Ascii data. Can be used for bestMatchesForSignature.
     */
    QString signatureAsText(const QImage& image);

    /**
     * @brief Checks whether the image with the given imageId fulfills all restrictions given in
     * targetAlbums and in respect to searchResultRestriction.
     */
    bool fulfillsRestrictions(qlonglong imageId,
                              int albumId,
                              qlonglong originalImageId,
                              int originalAlbumId,
                              const QList<int>& targetAlbums,
                              DuplicatesSearchRestrictions searchResultRestriction);

    /**
     * @brief For a given signature, find out the highest and lowest possible score
     * that any other signature could reach, compared to the given signature.
     */
    void getBestAndWorstPossibleScore(Haar::SignatureData* const querySig,
                                      SketchType type,
                                      double* const lowestAndBestScore,
                                      double* const highestAndWorstScore);

    /**
     * @brief Fill a map of duplicates images found over a list of images to scan.
     * For each map item, the result values is list of candidate images which are duplicates of the key image.
     * All images are referenced by id from database.
     * The threshold is in the range 0..1, with 1 meaning identical signature.
     */
    DuplicatesResultsMap findDuplicates(
        const QSet<qlonglong>& images2Scan,
        const QSet<qlonglong>::const_iterator& rangeBegin,
        const QSet<qlonglong>::const_iterator& rangeEnd,
        RefImageSelMethod refImageSelectionMethod,
        const QSet<qlonglong>& refs,
        double requiredPercentage,
        double maximumPercentage,
        DuplicatesSearchRestrictions searchResultRestriction = DuplicatesSearchRestrictions::None,
        HaarProgressObserver* const observer = nullptr
    );

    /**
     * @brief Collects all images from the given album and tag ids according to their relation.
     */
    static QSet<qlonglong> imagesFromAlbumsAndTags(const QList<int>& albums2Scan,
                                                   const QList<int>& tags2Scan,
                                                   AlbumTagRelation relation);

    /**
     * @brief This method rebuilds the given SAlbums using the given results.
     *
     * @param results Map of duplicates images found over a list of images.
     * @param isAlbumUpdate if true update the SAlbums in the database.
     */
    static void rebuildDuplicatesAlbums(const DuplicatesResultsMap& results, bool isAlbumUpdate);

    /**
     * @brief Retrieve the Haar signature from database using image id.
     * @return true if item signature exist else false.
     */
    bool retrieveSignatureFromDB(qlonglong imageid, Haar::SignatureData& sig);

    /**
     * @brief Give a list of albumRoots to which the search shall be limited.
     * Calling with an empty list will disable filtering.
     */
    void setAlbumRootsToSearch(const QList<int>& albumRootIds);
    void setAlbumRootsToSearch(const QSet<int>& albumRootIds);

    /**
     * @brief This method loads a QImage from the given filename.
     *
     * @param filename the name of the file (path)
     *
     * @return A QImage, non-null on success.
     */
    QImage loadQImage(const QString& filename);

private:

    bool   indexImage(qlonglong imageid);

    /**
     * @brief This method writes the search results to the SearchXml structure.
     *
     * @param searchResults The results to write as XML.
     */
    static QMap<QString, QString> writeSAlbumQueries(const DuplicatesResultsMap& searchResults);

    QMultiMap<double, qlonglong> bestMatches(const Haar::SignatureData* const data,
                                             int numberOfResults,
                                             const QList<int>& targetAlbums,
                                             SketchType type);

    /**
     * @brief bestMatchesWithThreshold
     *
     * @param imageid
     * @param querySig
     * @param requiredPercentage
     * @param maximumPercentage
     * @param targetAlbums
     * @param searchResultRestriction
     * @param type
     *
     * @return (av. similarity, Maps with images and similarity)
     */
    QPair<double, QMap<qlonglong, double> > bestMatchesWithThreshold(qlonglong imageid,
                                                                     Haar::SignatureData* const querySig,
                                                                     double requiredPercentage,
                                                                     double maximumPercentage,
                                                                     const QList<int>& targetAlbums,
                                                                     DuplicatesSearchRestrictions searchResultRestriction,
                                                                     SketchType type);

    /**
     * @brief This method is the core functionality: It assigns a score to every image in the database.
     *
     * @param data The signature of the original image for score calculation.
     * @param type The type of the sketch, e.g. scanned.
     * @param searchResultRestriction restrictions to apply to the generated map, i.e. None (default), same album or different album.
     * @param originalImageId the id of the original image to compare to other images. -1 is only used for sketch search.
     * @param albumId The album which images must or must not belong to (depending on searchResultRestriction).
     *
     * @return The map of image ids and scores which fulfill the restrictions, if any.
     */
    QMap<qlonglong, double> searchDatabase(const Haar::SignatureData* const data,
                                           SketchType type,
                                           const QList<int>& targetAlbums,
                                           DuplicatesSearchRestrictions searchResultRestriction = None,
                                           qlonglong originalImageId = -1,
                                           int albumId = -1);

    double calculateScore(const Haar::SignatureData& querySig,
                          const Haar::SignatureData& targetSig,
                          const Haar::Weights& weights,
                          const std::reference_wrapper<Haar::SignatureMap>* const queryMaps);

private:

    /// @note disabled
    HaarIface(const HaarIface&)            = delete;
    HaarIface& operator=(const HaarIface&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
