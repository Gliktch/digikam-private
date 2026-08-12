/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface - private containers.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "albummanager.h"

// C ANSI includes

extern "C"
{
#ifndef Q_CC_MSVC
#   include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
}

// C++ includes

#include <cerrno>
#include <clocale>
#include <cstdio>
#include <cstdlib>

// Qt includes

#include <QApplication>
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QGroupBox>
#include <QHash>
#include <QLabel>
#include <QList>
#include <QMultiHash>
#include <QRadioButton>
#include <QTimer>
#include <QComboBox>
#include <QIcon>
#include <QPointer>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QCheckBox>
#include <QSet>

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#   include <QTextCodec>
#endif

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "coredb.h"
#include "album.h"
#include "albumpointer.h"
#include "applicationsettings.h"
#include "metaenginesettings.h"
#include "metadatasynchronizer.h"
#include "metadatahubmngr.h"
#include "albumwatch.h"
#include "itemattributeswatch.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "digikam_config.h"
#include "coredbaccess.h"
#include "coredboperationgroup.h"
#include "dbengineguierrorhandler.h"
#include "dbengineparameters.h"
#include "dbsettingswidget.h"
#include "databaseserverstarter.h"
#include "coredbthumbinfoprovider.h"
#include "coredburl.h"
#include "coredbsearchxml.h"
#include "coredbwatch.h"
#include "dio.h"
#include "facetags.h"
#include "facetagseditor.h"
#include "itemlister.h"
#include "scancontroller.h"
#include "setupcollections.h"
#include "setup.h"
#include "tagscache.h"
#include "thumbsdbaccess.h"
#include "thumbnailloadthread.h"
#include "dnotificationwrapper.h"
#include "dbjobinfo.h"
#include "dbjobsmanager.h"
#include "dbjobsthread.h"
#include "similaritydb.h"
#include "similaritydbaccess.h"

namespace Digikam
{

class Q_DECL_HIDDEN PAlbumPath
{
public:

    PAlbumPath() = default;
    PAlbumPath(int albumRootId, const QString& albumPath);
    explicit PAlbumPath(const PAlbum* const album);

    bool operator==(const PAlbumPath& other) const;

public:

    int     albumRootId = -1;
    QString albumPath;
};

// -----------------------------------------------------------------------------------

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

inline size_t qHash(const PAlbumPath& id)

#else

inline uint qHash(const PAlbumPath& id)

#endif

{
    return (::qHash(id.albumRootId) ^ ::qHash(id.albumPath));
}

// -----------------------------------------------------------------------------------

class Q_DECL_HIDDEN AlbumManager::Private
{
public:

    Private() = default;

    QString labelForAlbumRootAlbum(const CollectionLocation& location);

public:

    bool                        changed                     = false;
    bool                        hasPriorizedDbPath          = false;

    bool                        showOnlyAvailableAlbums     = false;

    int                         longTimeMessageBoxResult    = -1;
    int                         askMergeMessageBoxResult    = -1;

    AlbumsDBJobsThread*         albumListJob                = nullptr;
    DatesDBJobsThread*          dateListJob                 = nullptr;
    TagsDBJobsThread*           tagListJob                  = nullptr;
    TagsDBJobsThread*           personListJob               = nullptr;


    AlbumWatch*                 albumWatch                  = nullptr;

    PAlbum*                     rootPAlbum                  = nullptr;
    TAlbum*                     rootTAlbum                  = nullptr;
    DAlbum*                     rootDAlbum                  = nullptr;
    SAlbum*                     rootSAlbum                  = nullptr;

    QHash<int, Album*>          allAlbumsIdHash;
    QHash<PAlbumPath, PAlbum*>  albumPathHash;
    QHash<int, PAlbum*>         albumRootAlbumHash;
    Album*                      currentlyMovingAlbum        = nullptr;

    QMultiHash<Album*, Album**> guardedPointers;

    /**
     * For multiple selection support
     */
    QList<Album*>               currentAlbums;

    bool                        changingDB                  = false;
    QTimer*                     scanPAlbumsTimer            = nullptr;
    QTimer*                     scanTAlbumsTimer            = nullptr;
    QTimer*                     scanSAlbumsTimer            = nullptr;
    QTimer*                     scanDAlbumsTimer            = nullptr;
    QTimer*                     updatePAlbumsTimer          = nullptr;
    QTimer*                     albumItemCountTimer         = nullptr;
    QTimer*                     tagItemCountTimer           = nullptr;
    QSet<int>                   changedPAlbums;

    QHash<int, int>             pAlbumsCount;
    QHash<int, int>             tAlbumsCount;
    QHash<int, int>             fAlbumsCount;
    QMap<YearMonth, int>        dAlbumsCount;
    /// Unconfirmed face counts
    QHash<int, int>             uAlbumsCount;
    QList<int>                  toUpdatedFaces;
};

// -----------------------------------------------------------------------------------

class Q_DECL_HIDDEN ChangingDB
{
public:

    explicit ChangingDB(AlbumManager::Private* const dd);
    ~ChangingDB();

    AlbumManager::Private* const d = nullptr;
};

} // namespace Digikam
