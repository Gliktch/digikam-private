/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-07
 * Description : a tool to print images
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QtGlobal>
#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QMap>
#include <QColor>
#include <QPrinter>
#include <QFont>

// Local includes

#include "filesaveconflictbox.h"

using namespace Digikam;

class KConfigGroup;

namespace DigikamGenericPrintCreatorPlugin
{

class AdvPrintPhoto;
class AdvPrintPhotoSize;

class AdvPrintSettings
{

public:

    /// Images selection mode
    enum Selection
    {
        IMAGES = 0,
        ALBUMS
    };

    /// Print output destination, outside real printers configured
    enum Output
    {
        FILES = 0,

#ifndef Q_OS_WIN

        PDF,

#endif

        GIMP
    };

    /// Image format for Output::FILES
    enum ImageFormat
    {
        JPEG = 0,
        PNG,
        TIFF
    };

    /// Caption type to print over the images
    enum CaptionType
    {
        NONE = 0,
        FILENAME,
        DATETIME,
        COMMENT,
        CUSTOM
    };

public:

    AdvPrintSettings() = default;
    ~AdvPrintSettings();

    /// Read and write settings in config file between sessions.
    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    QString format()                                 const;
    QString outputName(Output out)                   const;
    QRect*  getLayout(int photoIndex, int sizeIndex) const;

    /// Helper methods to fill combobox from GUI.
    static QMap<Output,      QString> outputNames();
    static QMap<ImageFormat, QString> imageFormatNames();
    static QMap<CaptionType, QString> captionTypeNames();

public:

    /// Items selection mode
    Selection                         selMode            = IMAGES;

    QList<QUrl>                       inputImages;

    QString                           printerName        = outputName(FILES);

    /// Page Size in mm
    /// Select a different page to force a refresh in initPhotoSizes.
    QSizeF                            pageSize           = QSizeF(-1.0, -1.0);

    QList<AdvPrintPhoto*>             photos;
    QList<AdvPrintPhotoSize*>         photosizes;

    /// Caption management.
    CaptionType                       captionType        = NONE;
    QColor                            captionColor       = Qt::yellow;
    QFont                             captionFont        = QFont(QLatin1String("Sans Serif"));
    int                               captionSize        = 4;

    /// String use to customize caption with CUSTOM mode.
    QString                           captionTxt;

    int                               currentPreviewPage = 0;

    /// Crop management
    int                               currentCropPhoto   = 0;
    bool                              disableCrop        = false;

    /// For Print to Gimp only
    QString                           tempPath;
    QStringList                       gimpFiles;
    QString                           gimpPath;
    QString                           savedPhotoSize;

    /// For print to image files only.
    ImageFormat                       imageFormat        = JPEG;

    /// Rule if output image files already exists.
    FileSaveConflictBox::ConflictRule conflictRule       = FileSaveConflictBox::OVERWRITE;

    /// Directory where to store output images.
    QUrl                              outputDir;

    bool                              openInFileBrowser  = true;

    /// Generic data used by printing thread.
    AdvPrintPhotoSize*                outputLayouts      = nullptr;
    QPrinter*                         outputPrinter      = nullptr;
    QString                           outputPath;

private:

    /// @note disabled
    AdvPrintSettings(const AdvPrintSettings&)            = delete;
    AdvPrintSettings& operator=(const AdvPrintSettings&) = delete;
};

} // namespace DigikamGenericPrintCreatorPlugin
