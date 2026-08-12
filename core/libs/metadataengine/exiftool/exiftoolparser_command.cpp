/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-11-28
 * Description : ExifTool process stream parser.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "exiftoolparser_p.h"

namespace Digikam
{

bool ExifToolParser::load(const QString& path)
{
    QFileInfo fileInfo(path);

    if (!fileInfo.exists())
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "Cannot open source file to process with ExifTool...";
        return false;
    }

    d->prepareProcess();

    // Build command (get metadata as JSON array)

    QByteArrayList cmdArgs;
    cmdArgs << QByteArray("-json");
    cmdArgs << QByteArray("-G:0:1:2:4:6");
    cmdArgs << QByteArray("-l");

/*
    TODO: better i18n support from ExifTool.
    ExifTool Translations are limited to few languages, and passing a non supported code to ExifTool
    returns an error. So we needs a mechanism to get the complete list of current i18n code supported,
    typically "exiftool -lang".

    // Forward GUI language code to get translated string from ExifTool (typically tags descriptions)

    QStringList langs = QLocale().uiLanguages();
    QByteArray lg;

    if (!langs.isEmpty())
    {
        lg = langs.first().section(QLatin1Char('-'), 0, 0).toLatin1();
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << "UI language code:" << lg;

    if (!lg.isEmpty())
    {
        cmdArgs << QByteArray("-lang") << lg;
    }

*/

    cmdArgs << d->filePathEncoding(fileInfo);
    d->currentPath = fileInfo.filePath();

    return (d->startProcess(cmdArgs, ExifToolProcess::LOAD_METADATA));
}

bool ExifToolParser::loadChunk(const QString& path, bool copyToAll, bool dropMakernotes)
{
    QFileInfo fileInfo(path);

    if (!fileInfo.exists())
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "Cannot open source file to process with ExifTool...";
        return false;
    }

    d->prepareProcess();

    // Build command (get metadata as EXV container for Exiv2)

    QByteArrayList cmdArgs;
    cmdArgs << QByteArray("-TagsFromFile");
    cmdArgs << d->filePathEncoding(fileInfo);

    QByteArray cpyOpt("-all");

    if (!copyToAll)
    {
        cpyOpt += ":all";
    }

    cmdArgs << cpyOpt;
    cmdArgs << QByteArray("-icc_profile");

    if (dropMakernotes)
    {
        cmdArgs << QByteArray("--makernotes");
    }

    if (copyToAll)
    {
        cmdArgs << QByteArray("-api");
        cmdArgs << QByteArray("QuickTimeUTC");
        cmdArgs << QByteArray("-xmp-dc:Subject<Microsoft:Category");
        cmdArgs << QByteArray("-xmp-microsoft:RatingPercent<Microsoft:SharedUserRating");
    }

    cmdArgs << QByteArray("-GpsDateTime<Composite:GpsDateTime");
    cmdArgs << QByteArray("-GpsAltitude*<Composite:GpsAltitude");
    cmdArgs << QByteArray("-GpsLatitude*<Composite:GpsPosition");
    cmdArgs << QByteArray("-GpsLongitude*<Composite:GpsPosition");

    cmdArgs << QByteArray("-o");
    cmdArgs << QByteArray("-.exv");
    d->currentPath = fileInfo.filePath();

    return (d->startProcess(cmdArgs, ExifToolProcess::LOAD_CHUNKS));
}

bool ExifToolParser::applyChanges(const QString& path, const ExifToolData& newTags)
{
    if (newTags.isEmpty())
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "List of tags to changes with ExifTool is empty";

        return false;
    }

    QFileInfo fileInfo(path);

    if (!fileInfo.exists())
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "Cannot open source file to process with ExifTool...";
        return false;
    }

    d->prepareProcess();

    // Build command (set metadata)

    QByteArrayList cmdArgs;
    cmdArgs << QByteArray("-m");
    cmdArgs << QByteArray("-json");

    for (ExifToolParser::ExifToolData::const_iterator it = newTags.constBegin() ;
         it != newTags.constEnd() ; ++it)
    {
        QString  tagNameExifTool = it.key();
        QString  tagValue        = it.value()[0].toString();
        cmdArgs << QString::fromUtf8("-%1=%2").arg(tagNameExifTool).arg(tagValue).toUtf8();
    }

    cmdArgs << d->filePathEncoding(fileInfo);
    d->currentPath = fileInfo.filePath();

    return (d->startProcess(cmdArgs, ExifToolProcess::APPLY_CHANGES));
}

bool ExifToolParser::applyChanges(const QString& path,
                                  const QString& exvTempFile,
                                  bool hasExif, bool hasXmp, bool hasCSet)
{
    if (exvTempFile.isEmpty())
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "EXV container files to apply changes with ExifTool is empty";
        return false;
    }

    QFileInfo fileInfo(path);

    if (!fileInfo.exists())
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "Cannot open source file to process with ExifTool...";
        return false;
    }

    d->prepareProcess();

    QMimeDatabase mimeDB;
    QByteArrayList cmdArgs;
    cmdArgs << QByteArray("-m");

    bool isVideo = (mimeDB.mimeTypeForFile(fileInfo).name().startsWith(QLatin1String("video/")));

    if (isVideo)
    {
        cmdArgs << QByteArray("-api");
        cmdArgs << QByteArray("QuickTimeHandler=1");
        cmdArgs << QByteArray("-ItemList:Title=");
        cmdArgs << QByteArray("-ItemList:Comment=");
        cmdArgs << QByteArray("-Microsoft:Category=");
        cmdArgs << QByteArray("-UserData:GpsCoordinates=");
        cmdArgs << QByteArray("-Microsoft:SharedUserRating=");
    }

    if (hasExif)
    {
        cmdArgs << QByteArray("-gps:all=");
        cmdArgs << QByteArray("-ifd0:xptitle=");
        cmdArgs << QByteArray("-ifd0:xpcomment=");
        cmdArgs << QByteArray("-ifd0:xpkeywords=");
        cmdArgs << QByteArray("-exififd:usercomment=");
    }
    else
    {
        cmdArgs << QByteArray("-exif:all=");
    }

    cmdArgs << QByteArray("-iptc:all=");
    cmdArgs << QByteArray("-file:comment=");

    if (hasXmp)
    {
        cmdArgs << QByteArray("-xmp-dc:all=");
        cmdArgs << QByteArray("-xmp-lr:all=");
        cmdArgs << QByteArray("-xmp-mp:all=");
        cmdArgs << QByteArray("-xmp-xmp:all=");
        cmdArgs << QByteArray("-xmp-exif:all=");
        cmdArgs << QByteArray("-xmp-tiff:all=");
        cmdArgs << QByteArray("-xmp-xmpdm:all=");
        cmdArgs << QByteArray("-xmp-acdsee:all=");
        cmdArgs << QByteArray("-xmp-mwg-rs:all=");
        cmdArgs << QByteArray("-xmp-digikam:all=");
        cmdArgs << QByteArray("-xmp-mediapro:all=");
        cmdArgs << QByteArray("-xmp-iptccore:all=");
        cmdArgs << QByteArray("-xmp-microsoft:all=");
        cmdArgs << QByteArray("-xmp-photoshop:all=");
    }
    else
    {
        cmdArgs << QByteArray("-xmp:all=");
    }

    cmdArgs << QByteArray("-TagsFromFile");
    cmdArgs << d->filePathEncoding(QFileInfo(exvTempFile));

    cmdArgs << QByteArray("-all:all");
    cmdArgs << QByteArray("-icc_profile");

    if (isVideo)
    {
        cmdArgs << QByteArray("-ItemList:Title<xmp-dc:Title");
        cmdArgs << QByteArray("-Microsoft:Category<xmp-dc:Subject");
        cmdArgs << QByteArray("-ItemList:Comment<xmp-dc:Description");
        cmdArgs << QByteArray("-UserData:GpsCoordinates<Composite:GpsPosition");
        cmdArgs << QByteArray("-Microsoft:SharedUserRating<xmp-microsoft:RatingPercent");
    }

    if (hasCSet)
    {
        cmdArgs << QByteArray("-codedcharacterset=UTF8");
    }

    cmdArgs << QByteArray("-overwrite_original");
    cmdArgs << d->filePathEncoding(fileInfo);
    d->currentPath = fileInfo.filePath();

    return (d->startProcess(cmdArgs, ExifToolProcess::APPLY_CHANGES_EXV));
}

bool ExifToolParser::applyMetadataFile(const QString& path, const QString& meta)
{
    QFileInfo metaInfo(meta);

    if (!metaInfo.exists())
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "Metadata file to apply with ExifTool not exists";
        return false;
    }

    QFileInfo fileInfo(path);

    if (!fileInfo.exists())
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "Cannot open source file to process with ExifTool...";
        return false;
    }

    d->prepareProcess();

    QByteArrayList cmdArgs;
    cmdArgs << QByteArray("-m");

    if (metaInfo.suffix().toUpper() == QLatin1String("JSON"))
    {
        cmdArgs << (QByteArray("-json=") + d->filePathEncoding(metaInfo));
    }
    else
    {
        cmdArgs << QByteArray("-TagsFromFile");
        cmdArgs << d->filePathEncoding(metaInfo);
        cmdArgs << QByteArray("-all:all");
    }

    cmdArgs << QByteArray("-overwrite_original");
    d->prepareFileAndSidecar(cmdArgs, fileInfo);
    d->currentPath = fileInfo.filePath();

    return (d->startProcess(cmdArgs, ExifToolProcess::APPLY_METADATA_FILE));
}

bool ExifToolParser::changeTimestamps(const QString& path, const QDateTime& dateTime)
{
    QFileInfo fileInfo(path);

    if (!fileInfo.exists())
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "Cannot open source file to process with ExifTool...";
        return false;
    }

    d->prepareProcess();

    QMimeDatabase mimeDB;
    QByteArrayList cmdArgs;
    cmdArgs << QByteArray("-m");
    cmdArgs << QByteArray("-wm");
    cmdArgs << QByteArray("w");

    bool isVideo = (mimeDB.mimeTypeForFile(fileInfo).name().startsWith(QLatin1String("video/")));

    if (isVideo)
    {
        cmdArgs << QByteArray("-api");
        cmdArgs << QByteArray("QuickTimeUTC");
/*
        cmdArgs << (QByteArray("-time:all=") +
                    dateTime.toString(QLatin1String("yyyy-MM-ddThh:mm:ssttt")).toLatin1());
*/
    }

    cmdArgs << (QByteArray("-time:all=") +
                dateTime.toString(QLatin1String("yyyy-MM-ddThh:mm:ss")).toLatin1());

    cmdArgs << QByteArray("-TagsFromFile");
    cmdArgs << QByteArray("@");
    cmdArgs << QByteArray("-xmp:CaptionsDateTimeStamps<xmp:CaptionsDateTimeStamps");

    cmdArgs << QByteArray("-overwrite_original");
    d->prepareFileAndSidecar(cmdArgs, fileInfo);
    d->currentPath = fileInfo.filePath();

    return (d->startProcess(cmdArgs, ExifToolProcess::CHANGE_TIMESTAMPS));
}

bool ExifToolParser::readableFormats()
{
    d->prepareProcess();

    // Build command

    QByteArrayList cmdArgs;
    cmdArgs << QByteArray("-l");
    cmdArgs << QByteArray("-listr");

    d->currentPath.clear();

    return (d->startProcess(cmdArgs, ExifToolProcess::READ_FORMATS));
}

bool ExifToolParser::writableFormats()
{
    d->prepareProcess();

    // Build command

    QByteArrayList cmdArgs;
    cmdArgs << QByteArray("-l");
    cmdArgs << QByteArray("-listwf");

    d->currentPath.clear();

    return (d->startProcess(cmdArgs, ExifToolProcess::WRITE_FORMATS));
}

bool ExifToolParser::translationsList()
{
    d->prepareProcess();

    // Build command

    QByteArrayList cmdArgs;
    cmdArgs << QByteArray("-lang");

    d->currentPath.clear();

    return (d->startProcess(cmdArgs, ExifToolProcess::TRANSLATIONS_LIST));
}

bool ExifToolParser::tagsDatabase()
{
    d->prepareProcess();

    // Build command

    QByteArrayList cmdArgs;
    cmdArgs << QByteArray("-listx");

    d->currentPath.clear();

    return (d->startProcess(cmdArgs, ExifToolProcess::TAGS_DATABASE));
}

bool ExifToolParser::version()
{
    d->prepareProcess();

    // Build command

    QByteArrayList cmdArgs;
    cmdArgs << QByteArray("-ver");

    d->currentPath.clear();

    return (d->startProcess(cmdArgs, ExifToolProcess::VERSION_STRING));
}

bool ExifToolParser::extractMotionPhotoVideo(const QString& path)
{
    QFileInfo fileInfo(path);

    if (!fileInfo.exists())
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "Cannot open source file to process with ExifTool...";
        return false;
    }

    d->prepareProcess();

    QByteArrayList cmdArgs;
    cmdArgs << QByteArray("-b");
    cmdArgs << QByteArray("-MotionPhotoVideo");
    cmdArgs << QByteArray("-EmbeddedVideoFile");
    cmdArgs << d->filePathEncoding(fileInfo);
    d->currentPath = fileInfo.filePath();

    return (d->startProcess(cmdArgs, ExifToolProcess::EXTRACT_MOTION_PHOTO_VIDEO));
}

bool ExifToolParser::copyTags(const QString& src, const QString& dst,
                              unsigned char copyOps,
                              unsigned char writeModes)
{
    QFileInfo sfi(src);

    if (!sfi.exists())
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "Cannot open source file to process with ExifTool...";
        return false;
    }

    QFileInfo dfi(src);

    if (!dfi.exists())
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "Cannot open destination file to process with ExifTool...";
        return false;
    }

    // ---

    QByteArray wrtCmds;

    qCDebug(DIGIKAM_METAENGINE_LOG) << "Copy Tags Modes:"
                                    << writeModes
                                    << "("
                                    << QString::fromLatin1("%1").arg(writeModes, 0, 2)
                                    << ")";

    if (writeModes & ExifToolProcess::WRITE_EXISTING_TAGS)
    {
        wrtCmds.append(QByteArray("w"));
    }

    if (writeModes & ExifToolProcess::CREATE_NEW_TAGS)
    {
        wrtCmds.append(QByteArray("c"));
    }

    if (writeModes & ExifToolProcess::CREATE_NEW_GROUPS)
    {
        wrtCmds.append(QByteArray("g"));
    }

    if (wrtCmds.isEmpty())
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "Copy tags writing modes list is empty!";
        return false;
    }

    // ---

    QByteArrayList copyCmds;

    qCDebug(DIGIKAM_METAENGINE_LOG) << "Copy Tags Operations:"
                                    << copyOps
                                    << "("
                                    << QString::fromLatin1("%1").arg(copyOps, 0, 2)
                                    << ")";

    if (!(copyOps & ExifToolProcess::COPY_NONE))
    {
        if (copyOps & ExifToolProcess::COPY_ALL)
        {
            copyCmds << QByteArray("-all:all");
        }
        else
        {
            if (copyOps & ExifToolProcess::COPY_EXIF)
            {
                copyCmds << QByteArray("-exif");
            }
            else
            {
                copyCmds << QByteArray("--exif");
            }

            if (copyOps & ExifToolProcess::COPY_MAKERNOTES)
            {
                copyCmds << QByteArray("-makernotes");
            }
            else
            {
                copyCmds << QByteArray("--makernotes");
            }

            if (copyOps & ExifToolProcess::COPY_IPTC)
            {
                copyCmds << QByteArray("-iptc");
            }
            else
            {
                copyCmds << QByteArray("--iptc");
            }

            if (copyOps & ExifToolProcess::COPY_XMP)
            {
                copyCmds << QByteArray("-xmp");
            }
            else
            {
                copyCmds << QByteArray("--xmp");
            }

            if (copyOps & ExifToolProcess::COPY_ICC)
            {
                copyCmds << QByteArray("-icc_profile");
            }
            else
            {
                copyCmds << QByteArray("--icc_profile");
            }

            if (copyOps & ExifToolProcess::RESTORE_PREVIEW)
            {
                copyCmds << QByteArray("-TagsFromFile");
                copyCmds << QByteArray("@");
                copyCmds << QByteArray("-PreviewImage");
            }
        }
    }

    // ---

    d->prepareProcess();

    QByteArrayList cmdArgs;
    cmdArgs << QByteArray("-m");

    cmdArgs << QByteArray("-wm") << wrtCmds;
    cmdArgs << QByteArray("-TagsFromFile");
    cmdArgs << d->filePathEncoding(QFileInfo(src));
    cmdArgs << copyCmds;
    cmdArgs << QByteArray("-overwrite_original");
    cmdArgs << d->filePathEncoding(QFileInfo(dst));
    d->currentPath = sfi.filePath();

    return (d->startProcess(cmdArgs, ExifToolProcess::COPY_TAGS));
}

bool ExifToolParser::translateTags(const QString& path, unsigned char transOps)
{
    QFileInfo fi(path);

    if (!fi.exists())
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "Cannot open source file to process with ExifTool...";
        return false;
    }

    // ---

    if (!d->argsFile.isOpen() && d->argsFile.exists())
    {
        d->argsFile.remove();
    }

    if (!d->argsFile.open())
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "Cannot open temporary file to write ExifTool tags translate config file...";
        return false;
    }

    QTextStream out(&d->argsFile);
    bool dirty = false;

    qCDebug(DIGIKAM_METAENGINE_LOG) << "Translate Tags:"
                                    << transOps
                                    << "("
                                    << QString::fromLatin1("%1").arg(transOps, 0, 2)
                                    << ")";

    if (transOps & ExifToolProcess::TRANS_ALL_XMP)
    {
        out << QLatin1String("-xmp:all<all:all") << Qt::endl;
        dirty = true;
    }

    if (transOps & ExifToolProcess::TRANS_ALL_IPTC)
    {
        out << QLatin1String("-iptc:all<all:all") << Qt::endl;
        dirty = true;
    }

    if (transOps & ExifToolProcess::TRANS_ALL_EXIF)
    {
        out << QLatin1String("-exif:all<all:all") << Qt::endl;
        dirty = true;
    }

    if (!dirty)
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "Translate tags operations list is empty!";
        return false;
    }

    // ---

    d->prepareProcess();

    QByteArrayList cmdArgs;

    cmdArgs << QByteArray("-@") << d->filePathEncoding(QFileInfo(d->argsFile.fileName()));
    cmdArgs << QByteArray("-overwrite_original");
    cmdArgs << d->filePathEncoding(QFileInfo(path));
    d->currentPath = fi.filePath();

    return (d->startProcess(cmdArgs, ExifToolProcess::TRANS_TAGS));
}

} // namespace Digikam
