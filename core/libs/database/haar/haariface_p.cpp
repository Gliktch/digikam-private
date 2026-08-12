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

#include "haariface_p.h"

namespace Digikam
{

void DatabaseBlob::read(const QByteArray& array, Haar::SignatureData& data)
{
    QDataStream stream(array);

    // check version

    qint32 version;
    stream >> version;

    if (version != Version)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Unsupported binary version of Haar Blob in database";
        return;
    }

    stream.setVersion(QDataStream::Qt_4_3);

    // read averages

    for (int i = 0 ; i < 3 ; ++i)
    {
        stream >> data.avg[i];
    }

    // read coefficients

    for (int i = 0 ; i < 3 ; ++i)
    {
        for (int j = 0 ; j < Haar::NumberOfCoefficients ; ++j)
        {
            stream >> data.sig[i][j];
        }
    }
}

QByteArray DatabaseBlob::write(const Haar::SignatureData& data)
{
    QByteArray array;
    array.reserve(sizeof(qint32) + 3*sizeof(double) + 3*sizeof(qint32)*Haar::NumberOfCoefficients);
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_3);

    // write version

    stream << (qint32)Version;

    // write averages

    for (int i = 0 ; i < 3 ; ++i)
    {
        stream << data.avg[i];
    }

    // write coefficients

    for (int i = 0 ; i < 3 ; ++i)
    {
        for (int j = 0 ; j < Haar::NumberOfCoefficients ; ++j)
        {
            stream << data.sig[i][j];
        }
    }

    return array;
}

// -----------------------------------------------------------------------------------------------------

HaarIface::Private::Private()
    : m_data(new Haar::ImageData)
{
}

void HaarIface::Private::rebuildSignatureCache(const QSet<qlonglong>& imageIds)
{
    m_signatureCache.reset(new SignatureCache);
    m_albumCache.reset(new AlbumCache);

    // Variables for data read from DB

    DatabaseBlob        blob;
    qlonglong           imageid;
    int                 albumid;
    Haar::SignatureData targetSig;

    // reference for easier access

    SignatureCache& sigCache = *m_signatureCache;
    AlbumCache&     albCache = *m_albumCache;

    QSqlQuery query          = SimilarityDbAccess().backend()->prepareQuery(signatureQuery);

    if (!SimilarityDbAccess().backend()->exec(query))
    {
        return;
    }

    QHash<qlonglong, QPair<int, int> > itemAlbumHash = CoreDbAccess().db()->getAllItemsWithAlbum();

    // Remove all ids from the fully created itemAlbumHash that are not needed for the duplicates search.
    // This is usually faster then starting a query for every single id in imageIds.

    if (!imageIds.isEmpty())
    {
        for (auto it = itemAlbumHash.begin() ; it != itemAlbumHash.end() ; )
        {
            if (!imageIds.contains(it.key()))
            {
                it = itemAlbumHash.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    const bool filterByAlbumRoots = !m_albumRootsToSearch.isEmpty();

    while (query.next())
    {
        imageid = query.value(0).toLongLong();

        if (itemAlbumHash.contains(imageid))
        {
            // Pair storage of <albumroootid, albumid>

            const QPair<int, int>& albumPair = itemAlbumHash.value(imageid);

            if (filterByAlbumRoots)
            {
                if (!m_albumRootsToSearch.contains(albumPair.first))
                {
                    continue;
                }
            }

            blob.read(query.value(1).toByteArray(), targetSig);
            albumid           = albumPair.second;
            sigCache[imageid] = targetSig;
            albCache[imageid] = albumid;
        }
    }
}

bool HaarIface::Private::hasSignatureCache() const
{
    return !(m_signatureCache.isNull() || m_signatureCache->isEmpty());
}

bool HaarIface::Private::retrieveSignatureFromCache(qlonglong imageId, Haar::SignatureData& data)
{
    if (!hasSignatureCache())
    {
        return false;
    }

    if (m_signatureCache->contains(imageId))
    {
        data = m_signatureCache->value(imageId);

        return true;
    }

    return false;
}

void HaarIface::Private::setImageDataFromImage(const QImage& image)
{
    m_data->fillPixelData(image);
}

void HaarIface::Private::setImageDataFromImage(const DImg& image)
{
    m_data->fillPixelData(image);
}

SignatureCache* HaarIface::Private::signatureCache() const
{
    return m_signatureCache.data();
}

AlbumCache* HaarIface::Private::albumCache() const
{
    return m_albumCache.data();
}

Haar::ImageData* HaarIface::Private::imageData() const
{
    return m_data.data();
}

void HaarIface::Private::setAlbumRootsToSearch(const QSet<int>& albumRootIds)
{
    m_albumRootsToSearch = albumRootIds;
}

const QSet<int>& HaarIface::Private::albumRootsToSearch() const
{
    return m_albumRootsToSearch;
}

} // namespace Digikam
