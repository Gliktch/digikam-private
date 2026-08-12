/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-06-05
 * Description : Thumbnail loading - private containers
 *
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "thumbnailloadthread.h"

// Qt includes

#include <QApplication>
#include <QMessageBox>
#include <QEventLoop>
#include <QPainter>
#include <QHash>
#include <QIcon>
#include <QMimeType>
#include <QMimeDatabase>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "iccmanager.h"
#include "iccprofile.h"
#include "loadingcache.h"
#include "thumbsdbaccess.h"
#include "thumbnailsize.h"
#include "thumbnailcreator.h"

namespace Digikam
{

class Q_DECL_HIDDEN ThumbnailResult
{

public:

    explicit ThumbnailResult(const LoadingDescription& description, const QImage& image)
        : loadingDescription(description),
          image             (image)
    {
    }

    LoadingDescription loadingDescription;
    QImage             image;
};

// -------------------------------------------------------------------

class Q_DECL_HIDDEN ThumbnailLoadThreadStaticPriv
{
public:

    ThumbnailLoadThreadStaticPriv() = default;

    ~ThumbnailLoadThreadStaticPriv()
    {
        delete provider;
    }

public:

    /// @note disabled
    ThumbnailLoadThreadStaticPriv(const ThumbnailLoadThreadStaticPriv&)            = delete;
    ThumbnailLoadThreadStaticPriv& operator=(const ThumbnailLoadThreadStaticPriv&) = delete;

    bool                            firstThreadCreated  = false;

    ThumbnailCreator::StorageMethod storageMethod       = ThumbnailCreator::FreeDesktopStandard;
    ThumbnailInfoProvider*          provider            = nullptr;
    IccProfile                      profile             = IccProfile::sRGB();
};

// -------------------------------------------------------------------

class Q_DECL_HIDDEN ThumbnailLoadThread::Private
{

public:

    Private() = default;

    bool                               wantPixmap           = true;
    bool                               highlight            = true;
    bool                               sendSurrogate        = true;
    bool                               notifiedForResults   = false;

    int                                size                 = ThumbnailSize::maxThumbsSize();

    ThumbnailCreator*                  creator              = nullptr;

    QHash<QString, ThumbnailResult>    collectedResults;
    QMutex                             resultsMutex;

    QList<LoadingDescription>          lastDescriptions;

public:

    LoadingDescription        createLoadingDescription(const ThumbnailIdentifier& identifier, int size, bool setLastDescription = true);
    LoadingDescription        createLoadingDescription(const ThumbnailIdentifier& identifier, int size,
                                                       const QRect& detailRect, bool setLastDescription = true);
    bool                      checkDescription(const LoadingDescription& description);
    QList<LoadingDescription> makeDescriptions(const QList<ThumbnailIdentifier>& identifiers, int size);
    QList<LoadingDescription> makeDescriptions(const QList<QPair<ThumbnailIdentifier, QRect> >& idsAndRects, int size);
    bool                      hasHighlightingBorder()                       const;
    int                       pixmapSizeForThumbnailSize(int thumbnailSize) const;
    int                       thumbnailSizeForPixmapSize(int pixmapSize)    const;
};

// --- ThumbnailImageCatcher ---------------------------------------------------------

class Q_DECL_HIDDEN ThumbnailImageCatcher::Private
{

public:

    enum CatcherState
    {
        Inactive,
        Accepting,
        Waiting,
        Quitting
    };

public:

    class Q_DECL_HIDDEN CatcherResult
    {
    public:

        explicit CatcherResult(const LoadingDescription& d)
            : description(d)
        {
        }

        CatcherResult(const LoadingDescription& d, const QImage& image)
            : image      (image),
              description(d),
              received   (true)
        {
        }

    public:

        QImage             image;
        LoadingDescription description;
        bool               received     = false;
    };

public:

    Private() = default;

    void reset();
    void harvest(const LoadingDescription& description, const QImage& image);

public:

    CatcherState                  state     = Inactive;

    bool                          active    = true;
    ThumbnailLoadThread*          thread    = nullptr;
    QList<Private::CatcherResult> tasks;
    QList<Private::CatcherResult> intermediate;

    QMutex                        mutex;
    QWaitCondition                condVar;
};

} // namespace Digikam
