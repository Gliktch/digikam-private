/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Listing information from database - SAlbum helpers.
 *
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer  <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2018      by Mario Frank    <mario dot frank at uni minus potsdam dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemlister_p.h"

namespace Digikam
{

void ItemLister::listSearch(ItemListerReceiver* const receiver,
                            const QString& xml,
                            int limit,
                            qlonglong referenceImageId)
{
    if (xml.isEmpty())
    {
        return;
    }

    QList<QVariant> boundValues;
    QList<QVariant> values;
    QString sqlQuery;
    QString errMsg;

    // query head

    sqlQuery = QString::fromUtf8(
               "SELECT DISTINCT Images.id, Images.name, Images.album, "
               "       Albums.albumRoot, "
               "       ImageInformation.rating, Images.category, "
               "       ImageInformation.format, ImageInformation.creationDate, "
               "       Images.modificationDate, Images.fileSize, "
               "       ImageInformation.width, ImageInformation.height, "
               "       ImagePositions.latitudeNumber, ImagePositions.longitudeNumber "
               " FROM Images "
               "       LEFT JOIN ImageInformation ON Images.id=ImageInformation.imageid "
               "       LEFT JOIN ImageMetadata    ON Images.id=ImageMetadata.imageid "
               "       LEFT JOIN VideoMetadata    ON Images.id=VideoMetadata.imageid "
               "       LEFT JOIN ImagePositions   ON Images.id=ImagePositions.imageid "
               "       LEFT JOIN ImageProperties  ON Images.id=ImageProperties.imageid "
               "       INNER JOIN Albums          ON Albums.id=Images.album "
               "WHERE Images.status=1 AND ( ");

    // query body

    ItemQueryBuilder   builder;
    ItemQueryPostHooks hooks;

    sqlQuery += builder.buildQuery(xml, &boundValues, &hooks);

    if (limit > 0)
    {
        sqlQuery += QString::fromUtf8(" ) LIMIT %1;").arg(limit);
    }
    else
    {
        sqlQuery += QString::fromUtf8(" );");
    }

    qCDebug(DIGIKAM_DATABASE_LOG) << "Search query:\n" << sqlQuery << "\n" << boundValues;

    bool executionSuccess;
    {
        CoreDbAccess access;
        executionSuccess = access.backend()->execSql(sqlQuery, boundValues, &values);

        if (!executionSuccess)
        {
            errMsg = access.backend()->lastError();
        }
    }

    if (!executionSuccess)
    {
        receiver->error(errMsg);
        return;
    }

    qCDebug(DIGIKAM_DATABASE_LOG) << "Search result:" << values.size() / 14;

    QSet<int> albumRoots = albumRootsToList();
    int       width      = 0;
    int       height     = 0;
    double    lat        = 0.0;
    double    lon        = 0.0;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ;)
    {
        ItemListerRecord record;

        record.imageID               = (*it).toLongLong();
        ++it;
        record.name                  = (*it).toString();
        ++it;
        record.albumID               = (*it).toInt();
        ++it;
        record.albumRootID           = (*it).toInt();
        ++it;
        record.rating                = (*it).toInt();
        ++it;
        record.category              = (DatabaseItem::Category)(*it).toInt();
        ++it;
        record.format                = (*it).toString();
        ++it;
        record.creationDate          = asDateTimeUTC((*it).toDateTime());
        ++it;
        record.modificationDate      = asDateTimeUTC((*it).toDateTime());
        ++it;
        record.fileSize              = (*it).toLongLong();
        ++it;
        width                        = (*it).toInt();
        ++it;
        height                       = (*it).toInt();
        ++it;
        lat                          = (*it).toDouble();
        ++it;
        lon                          = (*it).toDouble();
        ++it;

        record.currentSimilarity     = 0.0;
        record.currentReferenceImage = referenceImageId;

        if (referenceImageId != -1)
        {
            double current = SimilarityDbAccess().db()->getImageSimilarity(record.imageID,
                                                                           referenceImageId);
            if (current > 0.0)
            {
                record.currentSimilarity = current;
            }
        }

        if (d->listOnlyAvailableImages && !albumRoots.contains(record.albumRootID))
        {
            continue;
        }

        if (!hooks.checkPosition(lat, lon))
        {
            continue;
        }

        record.imageSize = QSize(width, height);

        receiver->receive(record);
    }
}

void ItemLister::listHaarSearch(ItemListerReceiver* const receiver,
                                const QString& xml)
{
/*
    qCDebug(DIGIKAM_GENERAL_LOG) << "Query: " << xml;
*/
    // ------------------------------------------------
    // read basic info

    SearchXmlReader reader(xml);
    reader.readToFirstField();

    if (reader.fieldName() != QLatin1String("similarity"))
    {
        receiver->error(QLatin1String("Unsupported field name \"") +
                        reader.fieldName() + QLatin1String("\" in Haar search"));
        return;
    }

    QStringView type                  = reader.attributes().value(QLatin1String("type"));
    QStringView numResultsString      = reader.attributes().value(QLatin1String("numberofresults"));
    QStringView thresholdString       = reader.attributes().value(QLatin1String("threshold"));
    QStringView maxThresholdString    = reader.attributes().value(QLatin1String("maxthreshold"));
    QStringView sketchTypeString      = reader.attributes().value(QLatin1String("sketchtype"));

    double threshold                 = 0.9;
    double maxThreshold              = 1.0;
    int numberOfResults              = 20;
    HaarIface::SketchType sketchType = HaarIface::ScannedSketch;

    QList<int> targetAlbums;

    // ------------------------------------------------
    // read target albums

    SearchXmlReader albumsReader(xml);
    SearchXml::Element element;

    while ((element = albumsReader.readNext()) != SearchXml::End)
    {
        // Get the target albums, i.e. the albums in which the similar images must be located.

        if (
            (element == SearchXml::Field) &&
            (albumsReader.fieldName().compare(QLatin1String("noeffect_targetAlbums")) == 0)
           )
        {
            targetAlbums = albumsReader.valueToIntList();
/*
            qCDebug(DIGIKAM_GENERAL_LOG) << "Searching in " << targetAlbums.size() << " Albums";
*/
            break;
        }
    }

    // -------------------------------------------------

    if (!numResultsString.isNull())
    {
        numberOfResults = numResultsString.toString().toInt();
/*
        qCDebug(DIGIKAM_GENERAL_LOG) << "Returning " << numberOfResults << " results";
*/
    }

    if (!thresholdString.isNull())
    {
        threshold = qMax(thresholdString.toString().toDouble(), 0.1);
/*
        qCDebug(DIGIKAM_GENERAL_LOG) << "Minimum threshold: " << threshold;
*/
    }

    if (!maxThresholdString.isNull())
    {
        maxThreshold = qMax(maxThresholdString.toString().toDouble(), threshold);
/*
        qCDebug(DIGIKAM_GENERAL_LOG) << "Maximum threshold: " << maxThreshold;
*/
    }

    if (!sketchTypeString.isNull() && sketchTypeString == QLatin1String("handdrawn"))
    {
        sketchType = HaarIface::HanddrawnSketch;
    }

    QMap<qlonglong, double> imageSimilarityMap;
    qlonglong referenceImageId = -1;

    if      (type == QLatin1String("signature"))
    {
        QString sig = reader.value();
        HaarIface iface;

        if (d->listOnlyAvailableImages)
        {
            iface.setAlbumRootsToSearch(albumRootsToList());
        }

        imageSimilarityMap = iface.bestMatchesForSignature(sig, targetAlbums, numberOfResults, sketchType);
    }
    else if (type == QLatin1String("imageid"))
    {
        referenceImageId = reader.valueToLongLong();
        HaarIface iface;

        if (d->listOnlyAvailableImages)
        {
            iface.setAlbumRootsToSearch(albumRootsToList());
        }

        imageSimilarityMap = iface.bestMatchesForImageWithThreshold(referenceImageId, threshold, maxThreshold, targetAlbums,
                                                                    HaarIface::DuplicatesSearchRestrictions::None, sketchType).second;
    }
    else if (type == QLatin1String("image"))
    {
        // If the given SAlbum contains a dropped image, get all images which are similar to this one.

        QString path = reader.value();
        HaarIface iface;

        if (d->listOnlyAvailableImages)
        {
            iface.setAlbumRootsToSearch(albumRootsToList());
        }

        imageSimilarityMap = iface.bestMatchesForImageWithThreshold(path, threshold,maxThreshold, targetAlbums,
                                                                    HaarIface::DuplicatesSearchRestrictions::None, sketchType).second;
    }

    listFromHaarSearch(receiver, imageSimilarityMap, referenceImageId);
}

void ItemLister::listFromHaarSearch(ItemListerReceiver* const receiver,
                                    const QMap<qlonglong, double>& imageSimilarityMap,
                                    qlonglong referenceImageId)
{
    QList<QVariant> values;
    QString         errMsg;
    bool            executionSuccess = true;

    {
        // Generate the query that returns the similarity as constant for a given image id.

        CoreDbAccess access;
        QSqlQuery query = access.backend()->prepareQuery(QString::fromUtf8(
                              "SELECT DISTINCT Images.id, Images.name, Images.album, "
                              "       Albums.albumRoot, "
                              "       ImageInformation.rating, Images.category, "
                              "       ImageInformation.format, ImageInformation.creationDate, "
                              "       Images.modificationDate, Images.fileSize, "
                              "       ImageInformation.width, ImageInformation.height "
                              " FROM Images "
                              "       LEFT JOIN ImageInformation ON Images.id=ImageInformation.imageid "
                              "       LEFT JOIN Albums ON Albums.id=Images.album "
                              " WHERE Images.status=1 AND Images.id = ?;"));

        qlonglong imageId;
        double similarity;

        // Iterate over the image similarity map and bind the image id and similarity to the query.

        for (QMap<qlonglong, double>::const_iterator it = imageSimilarityMap.constBegin() ;
             it != imageSimilarityMap.constEnd() ; ++it)
        {
            similarity = it.value();
            imageId    = it.key();

            query.bindValue(0, imageId);
            executionSuccess = access.backend()->exec(query);

            if (!executionSuccess)
            {
                errMsg = access.backend()->lastError();
                break;
            }

            // Add the similarity to the table row.

            QList<QVariant> tableRow = access.backend()->readToList(query);
            tableRow.append(similarity);

            // append results to list

            values << tableRow;
        }
    }

    if (!executionSuccess)
    {
        receiver->error(errMsg);
        return;
    }

    int width  = 0;
    int height = 0;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        ItemListerRecord record;

        record.imageID               = (*it).toLongLong();
        ++it;
        record.name                  = (*it).toString();
        ++it;
        record.albumID               = (*it).toInt();
        ++it;
        record.albumRootID           = (*it).toInt();
        ++it;
        record.rating                = (*it).toInt();
        ++it;
        record.category              = (DatabaseItem::Category)(*it).toInt();
        ++it;
        record.format                = (*it).toString();
        ++it;
        record.creationDate          = asDateTimeUTC((*it).toDateTime());
        ++it;
        record.modificationDate      = asDateTimeUTC((*it).toDateTime());
        ++it;
        record.fileSize              = (*it).toLongLong();
        ++it;
        width                        = (*it).toInt();
        ++it;
        height                       = (*it).toInt();
        ++it;

        record.imageSize             = QSize(width, height);

        record.currentSimilarity     = (*it).toDouble();
        ++it;

        record.currentReferenceImage = referenceImageId;

        receiver->receive(record);
    }
}

void ItemLister::listAreaRange(ItemListerReceiver* const receiver,
                               double lat1,
                               double lat2,
                               double lon1,
                               double lon2)
{
    QList<QVariant> values;
    QList<QVariant> boundValues;
    boundValues << lat1 << lat2 << lon1 << lon2;

    qCDebug(DIGIKAM_DATABASE_LOG) << "Listing area" << lat1 << lat2 << lon1 << lon2;

    {
        CoreDbAccess access;
        access.backend()->execSql(QString::fromUtf8("SELECT DISTINCT Images.id, "
                                          "       Albums.albumRoot, ImageInformation.rating, ImageInformation.creationDate, "
                                          "       ImagePositions.latitudeNumber, ImagePositions.longitudeNumber "
                                          " FROM Images "
                                          "       LEFT JOIN ImageInformation ON Images.id=ImageInformation.imageid "
                                          "       INNER JOIN Albums ON Albums.id=Images.album "
                                          "       INNER JOIN ImagePositions   ON Images.id=ImagePositions.imageid "
                                          " WHERE Images.status=1 "
                                          "   AND (ImagePositions.latitudeNumber>? AND ImagePositions.latitudeNumber<?) "
                                          "   AND (ImagePositions.longitudeNumber>? AND ImagePositions.longitudeNumber<?);"),
                                  boundValues,
                                  &values);
    }

    qCDebug(DIGIKAM_DATABASE_LOG) << "Results:" << values.size() / 14;

    QSet<int> albumRoots = albumRootsToList();
    double    lat        = 0.0;
    double    lon        = 0.0;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        ItemListerRecord record;

        record.imageID           = (*it).toLongLong();
        ++it;
        record.albumRootID       = (*it).toInt();
        ++it;

        record.rating            = (*it).toInt();
        ++it;
        record.creationDate      = asDateTimeUTC((*it).toDateTime());
        ++it;
        lat                      = (*it).toDouble();
        ++it;
        lon                      = (*it).toDouble();
        ++it;

        if (d->listOnlyAvailableImages && !albumRoots.contains(record.albumRootID))
        {
            continue;
        }

        record.extraValues << lat << lon;

        receiver->receive(record);
    }
}

} // namespace Digikam
