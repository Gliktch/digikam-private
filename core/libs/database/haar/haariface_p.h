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

#include "haariface.h"

// C++ includes

#include <fstream>
#include <cmath>
#include <cstring>
#include <functional>

// Qt includes

#include <QByteArray>
#include <QDataStream>
#include <QImage>
#include <QImageReader>
#include <QMap>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals_p.h"
#include "jpegutils.h"
#include "dimg.h"
#include "iteminfo.h"
#include "coredbaccess.h"
#include "coredbtransaction.h"
#include "coredb.h"
#include "coredbbackend.h"
#include "coredbsearchxml.h"
#include "similaritydb.h"
#include "similaritydbaccess.h"

using namespace std;

// TODO: Always store similarities in the similaritydb

namespace Digikam
{

typedef QMap<qlonglong, Haar::SignatureData> SignatureCache;
typedef QMap<qlonglong, int>                 AlbumCache;

/**
 * @brief This class encapsulates the Haar signature in a QByteArray
 * that can be stored as a BLOB in the database.
 *
 * Reading and writing is done in a platform-independent manner, which
 * induces a certain overhead, but which is necessary IMO.
 */
class Q_DECL_HIDDEN DatabaseBlob
{
public:

    enum
    {
        Version = 1
    };

public:

    DatabaseBlob() = default;

    /**
     * @brief Read the QByteArray into the Haar::SignatureData.
     */
    void read(const QByteArray& array, Haar::SignatureData& data);

    QByteArray write(const Haar::SignatureData& data);
};

// -----------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN HaarIface::Private
{
public:

    Private();
    ~Private() = default;

public:

    void rebuildSignatureCache(const QSet<qlonglong>& imageIds = {});
    bool hasSignatureCache()              const;

    bool retrieveSignatureFromCache(qlonglong imageId, Haar::SignatureData& data);

    void setImageDataFromImage(const QImage& image);
    void setImageDataFromImage(const DImg& image);

    SignatureCache*  signatureCache()     const;
    AlbumCache*      albumCache()         const;
    Haar::ImageData* imageData()          const;

    void setAlbumRootsToSearch(const QSet<int>& albumRootIds);
    const QSet<int>& albumRootsToSearch() const;

public:

    const QString                   signatureQuery = QString::fromUtf8("SELECT imageid, matrix FROM ImageHaarMatrix;");
    const Haar::WeightBin           weightBin;

private:

    QScopedPointer<SignatureCache>  m_signatureCache;
    QScopedPointer<AlbumCache>      m_albumCache;

    QScopedPointer<Haar::ImageData> m_data;

    QSet<int>                       m_albumRootsToSearch;
};

} // namespace Digikam
