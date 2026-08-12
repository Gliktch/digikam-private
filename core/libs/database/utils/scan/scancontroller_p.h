/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-10-28
 * Description : scan item controller - private containers.
 *
 * SPDX-FileCopyrightText: 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "scancontroller.h"

// Qt includes

#include <QStringList>
#include <QFileInfo>
#include <QPixmap>
#include <QIcon>
#include <QTime>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QTimer>
#include <QEventLoop>
#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "coredbaccess.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "filereadwritelock.h"
#include "coredbwatch.h"
#include "dprogressdlg.h"
#include "dmetadata.h"
#include "coredb.h"
#include "albummanager.h"
#include "album.h"
#include "coredbschemaupdater.h"
#include "iteminfo.h"

namespace Digikam
{

/*
 * This class is derived from the LoadingCacheFileWatch,
 * which means it has the full functionality of the class
 * and only extends it by listening to CollectionScanner information
 */
class Q_DECL_HIDDEN ScanControllerLoadingCacheFileWatch : public LoadingCacheFileWatch
{
    Q_OBJECT

public:

    ScanControllerLoadingCacheFileWatch();

private Q_SLOTS:

    void slotImageChanged(const ImageChangeset& changeset);

private:

    /// @note disabled
    explicit ScanControllerLoadingCacheFileWatch(QObject*) = delete;
};

// ------------------------------------------------------------------------------

class Q_DECL_HIDDEN SimpleCollectionScannerObserver : public CollectionScannerObserver
{
public:

    explicit SimpleCollectionScannerObserver(bool* const var);

    bool continueQuery() override;

public:

    bool* m_continue = nullptr;
};

// ------------------------------------------------------------------------------

class Q_DECL_HIDDEN ScanController::Private
{
public:

    Private() = default;

    QPixmap albumPixmap();
    QPixmap rootPixmap();
    QPixmap actionPixmap();
    QPixmap errorPixmap();
    QPixmap restartPixmap();

public:

    bool                            running                 = false;
    bool                            needsInitialization     = false;
    bool                            needsCompleteScan       = false;
    bool                            needsUpdateUniqueHash   = false;
    bool                            idle                    = false;

    int                             scanSuspended           = 0;

    QStringList                     scanTasks;

    QStringList                     completeScanDeferredAlbums;
    bool                            deferFileScanning       = false;
    bool                            finishScanAllowed       = true;

    QMutex                          mutex;
    QWaitCondition                  condVar;

    bool                            continueInitialization  = false;
    bool                            continueScan            = false;
    bool                            continuePartialScan     = false;

    bool                            fileWatchInstalled      = false;

    QEventLoop*                     eventLoop               = nullptr;

    QTimer*                         showTimer               = nullptr;
    QTimer*                         relaxedTimer            = nullptr;
    QTimer*                         externalTimer           = nullptr;

    QPixmap                         albumPix;
    QPixmap                         rootPix;
    QPixmap                         actionPix;
    QPixmap                         errorPix;

    CollectionScannerHintContainer* hints                   = CollectionScanner::createHintContainer();

    DProgressDlg*                   progressDialog          = nullptr;

    ScanController::Advice          advice                  = ScanController::Success;

    bool                            needTotalFiles          = false;
    bool                            performFastScan         = true;
    int                             totalFilesToScan        = 0;

    QList<qlonglong>                newIdsList;
};

} // namespace Digikam
