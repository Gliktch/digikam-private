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

#include "advprintsettings.h"

// Qt includes

#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "advprintphoto.h"

namespace DigikamGenericPrintCreatorPlugin
{

AdvPrintSettings::~AdvPrintSettings()
{
    for (int i = 0 ; i < photos.count() ; ++i)
    {
        delete photos.at(i);
    }

    photos.clear();
}

void AdvPrintSettings::readSettings(const KConfigGroup& group)
{
    selMode           = (Selection)group.readEntry("SelMode",
                        (int)IMAGES);
    imageFormat       = (ImageFormat)group.readEntry("ImageFormat",
                        (int)JPEG);
    savedPhotoSize    = group.readEntry("PhotoSize",
                        QString());
    printerName       = group.readEntry("Printer",
                        outputName(FILES));
    captionType       = (CaptionType)group.readEntry(QLatin1String("CaptionType"),
                        (int)NONE);
    captionColor      = group.readEntry(QLatin1String("CaptionColor"),
                        QColor(Qt::yellow));
    captionFont       = group.readEntry(QLatin1String("CaptionFont"),
                        QFont(QLatin1String("Sans Serif")));
    captionSize       = group.readEntry(QLatin1String("CaptionSize"),
                        4);
    captionTxt        = group.readEntry(QLatin1String("CustomCaption"),
                        QString());
    outputDir         = group.readEntry("OutputPath",
                        QUrl::fromLocalFile(QStandardPaths::writableLocation
                            (QStandardPaths::DocumentsLocation)));
    conflictRule      = (FileSaveConflictBox::ConflictRule)group.readEntry("ConflictRule",
                        (int)FileSaveConflictBox::OVERWRITE);
    openInFileBrowser = group.readEntry("OpenInFileBrowser",
                        true);
    imageFormat       = (ImageFormat)group.readEntry("ImageFormat",
                        (int)JPEG);
}

void AdvPrintSettings::writeSettings(KConfigGroup& group)
{
    group.writeEntry("SelMode",           (int)selMode);
    group.writeEntry("ImageFormat",       (int)imageFormat);
    group.writeEntry("PhotoSize",         savedPhotoSize);
    group.writeEntry("Printer",           printerName);
    group.writeEntry("CaptionType",       (int)captionType);
    group.writeEntry("CaptionColor",      captionColor);
    group.writeEntry("CaptionFont",       captionFont);
    group.writeEntry("CaptionSize",       captionSize);
    group.writeEntry("CustomCaption",     captionTxt);
    group.writeEntry("OutputPath",        outputDir);
    group.writeEntry("ConflictRule",      (int)conflictRule);
    group.writeEntry("OpenInFileBrowser", openInFileBrowser);
    group.writeEntry("ImageFormat",       (int)imageFormat);
}

QString AdvPrintSettings::outputName(Output out) const
{
    QMap<Output, QString> outputs = outputNames();

    if (outputs.contains(out))
    {
        return outputs[out];
    }

    return QString();
}

QMap<AdvPrintSettings::Output, QString> AdvPrintSettings::outputNames()
{
    QMap<Output, QString> out;

    out[FILES] = i18nc("Output: FILE", "Print to Image File");

#ifndef Q_OS_WIN

    out[PDF]   = i18nc("Output: PDF",  "Print to PDF");

#endif

    out[GIMP]  = i18nc("Output: GIMP", "Print with Gimp");

    return out;
}

QString AdvPrintSettings::format() const
{
    if      (imageFormat == JPEG)
    {
        return QLatin1String("JPEG");
    }
    else if (imageFormat == TIFF)
    {
        return QLatin1String("TIF");
    }

    return QLatin1String("PNG");
}

QMap<AdvPrintSettings::ImageFormat, QString> AdvPrintSettings::imageFormatNames()
{
    QMap<ImageFormat, QString> frms;

    frms[JPEG] = i18nc("Image format: JPEG", "JPEG");
    frms[PNG]  = i18nc("Image format: PNG",  "PNG");
    frms[TIFF] = i18nc("Image format: TIFF", "TIFF");

    return frms;
}

QMap<AdvPrintSettings::CaptionType, QString> AdvPrintSettings::captionTypeNames()
{
    QMap<CaptionType, QString> types;

    types[NONE]     = i18nc("Caption type: NONE",      "No caption");
    types[FILENAME] = i18nc("Caption type: FILENAME",  "Image file names");
    types[DATETIME] = i18nc("Caption type: DATETIME",  "Exif date-time");
    types[COMMENT]  = i18nc("Caption type: COMMENT",   "Item comments");
    types[CUSTOM]   = i18nc("Caption type: CUSTOM",    "Custom format");

    return types;
}

QRect* AdvPrintSettings::getLayout(int photoIndex, int sizeIndex) const
{
    AdvPrintPhotoSize* const s = photosizes.at(sizeIndex);

    // how many photos would actually be printed, including copies?

    int photoCount             = (photoIndex + 1);

    // how many pages?  Recall that the first layout item is the paper size

    int photosPerPage          = s->m_layouts.count() - 1;
    int remainder              = photoCount % photosPerPage;
    int retVal                 = (remainder == 0) ? photosPerPage : remainder;

    return s->m_layouts.at(retVal);
}

} // namespace DigikamGenericPrintCreatorPlugin
