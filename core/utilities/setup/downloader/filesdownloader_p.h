/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-11-14
 * Description : Files downloader
 *
 * SPDX-FileCopyrightText: 2020-2024 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "filesdownloader.h"

// Qt includes

#include <QDir>
#include <QUrl>
#include <QLabel>
#include <QTimer>
#include <QThread>
#include <QPointer>
#include <QFileInfo>
#include <QCheckBox>
#include <QByteArray>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QApplication>
#include <QStandardPaths>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QCryptographicHash>
#include <QNetworkAccessManager>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dfileselector.h"
#include "dxmlguiwindow.h"
#include "systemsettings.h"
#include "dnnmodelmanager.h"
#include "itempropertiestab.h"
#include "downloadinfo.h"

namespace Digikam
{

class Q_DECL_HIDDEN FilesDownloader::Private
{
public:

    Private() = default;

public:

    QList<DownloadInfo>    files;
    DownloadInfo           currentInfo;

    int                    redirects        = 0;
    int                    total            = 0;
    int                    count            = 0;

    bool                   fallback         = false;

    QDialogButtonBox*      buttons          = nullptr;
    QProgressBar*          progress         = nullptr;
    QCheckBox*             aiAutoToolsCheck = nullptr;
    QCheckBox*             faceEngineCheck  = nullptr;
    QCheckBox*             aestheticCheck   = nullptr;
    QCheckBox*             autoTagsCheck    = nullptr;
    QLabel*                nameLabel        = nullptr;
    QLabel*                infoLabel        = nullptr;
    QLabel*                loadLabel        = nullptr;
    QLabel*                sizeLabel        = nullptr;
    QLabel*                pathLabel        = nullptr;

    DFileSelector*         selector         = nullptr;

    QNetworkReply*         reply            = nullptr;
    QNetworkAccessManager* netMngr          = nullptr;

    QString                error;
    const QString          downloadUrl      = QLatin1String("https://files.kde.org/digikam/");
    const QString          fallbackUrl      = QLatin1String("https://cdn.files.kde.org/digikam/");
    const QString          userAgent        = QLatin1String("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
                                                            "(KHTML, like Gecko) Chrome/130.0.0.0 Safari/537.36");
};

} // namespace Digikam
