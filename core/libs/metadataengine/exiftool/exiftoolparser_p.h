/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-11-28
 * Description : ExifTool process stream parser - private container.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "exiftoolparser.h"

// Qt includes

#include <QDir>
#include <QMutex>
#include <QThread>
#include <QLocale>
#include <QVariant>
#include <QFileInfo>
#include <QEventLoop>
#include <QJsonArray>
#include <QStringList>
#include <QDomElement>
#include <QJsonObject>
#include <QApplication>
#include <QDomDocument>
#include <QJsonDocument>
#include <QMimeDatabase>
#include <QTemporaryFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "dmetadata.h"
#include "exiftoolthread.h"
#include "metaenginesettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN ExifToolParser::Private
{
public:

    explicit Private(ExifToolParser* const q);
    ~Private();

    void       prepareProcess();
    bool       startProcess(const QByteArrayList& cmdArgs, ExifToolProcess::Action cmdAction);
    void       prepareFileAndSidecar(QByteArrayList& cmdArgs, const QFileInfo& fi);
    QByteArray filePathEncoding(const QFileInfo& fi) const;

    void       jumpToResultCommand(const ExifToolProcess::Result& result, int cmdId);

    /**
     * Returns a string for an action.
     */
    QString    actionString(int cmdAction) const;

public:

    ExifToolParser*                pp           = nullptr;
    ExifToolProcess*               proc         = nullptr;  ///< ExifTool process instance.
    QString                        currentPath;             ///< Current file path processed by ExifTool.
    QString                        errorString;             ///< Current error string from the last started ExifTool process.
    ExifToolData                   exifToolData;            ///< Current ExifTool data (input or output depending of the called method.
    QTemporaryFile                 argsFile;                ///< Temporary file to store Exiftool arg config file.

    QMutex                         mutex;

    bool                           error        = false;
    bool                           async        = false;
    QList<int>                     asyncRunning;
};

} // namespace Digikam
