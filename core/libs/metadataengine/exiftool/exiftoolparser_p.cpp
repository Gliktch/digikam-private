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

#include "exiftoolparser_p.h"

namespace Digikam
{

ExifToolParser::Private::Private(ExifToolParser* const q)
    : pp(q)
{
    argsFile.setAutoRemove(false);
}

ExifToolParser::Private::~Private()
{
    if (argsFile.exists())
    {
        argsFile.remove();
    }
}

void ExifToolParser::Private::prepareProcess()
{
    error = false;
    currentPath.clear();
    errorString.clear();
    exifToolData.clear();
}

bool ExifToolParser::Private::startProcess(const QByteArrayList& cmdArgs,
                                           ExifToolProcess::Action cmdAction)
{
    // Send command to ExifToolProcess

    int cmdId = proc->command(cmdArgs, cmdAction);

    if (cmdId == 0)
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "ExifTool cannot be sent:"
                                          << actionString(cmdAction);

        return false;
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << "ExifTool" << actionString(cmdAction)
                                    << cmdArgs.join(QByteArray(" "));

    if (async)
    {
        QMutexLocker locker(&mutex);

        asyncRunning << cmdId;

        return true;
    }

    ExifToolProcess::Result result = proc->getExifToolResult(cmdId);

    while (
           (result.cmdNumber != cmdId) &&
           (result.cmdStatus != ExifToolProcess::FINISH_RESULT)
          )
    {
        result = proc->waitForExifToolResult(cmdId);

        if ((result.cmdNumber == cmdId) && result.waitError)
        {
            qCWarning(DIGIKAM_METAENGINE_LOG) << "ExifTool timed out:"
                                              << actionString(cmdAction);

            return false;
        }
    }

    jumpToResultCommand(result, cmdId);

    return !error;
}

void ExifToolParser::Private::prepareFileAndSidecar(QByteArrayList& cmdArgs, const QFileInfo& fi)
{
//    MetaEngineSettingsContainer settings = MetaEngineSettings::instance()->settings();

//    if (settings.metadataWritingMode != DMetadata::WRITE_TO_SIDECAR_ONLY)
//    {
        cmdArgs << filePathEncoding(fi);
//    }

//    if (settings.metadataWritingMode != DMetadata::WRITE_TO_FILE_ONLY)
//    {
//        QFileInfo si(DMetadata::sidecarPath(fi.filePath()));

//        cmdArgs << filePathEncoding(si);
//    }
}

QByteArray ExifToolParser::Private::filePathEncoding(const QFileInfo& fi) const
{
    return (QDir::toNativeSeparators(fi.filePath()).toUtf8());
}

void ExifToolParser::Private::jumpToResultCommand(const ExifToolProcess::Result& result, int cmdId)
{
    if (result.cmdNumber != cmdId)
    {
        return;
    }

    switch (result.cmdStatus)
    {
        case ExifToolProcess::COMMAND_RESULT:
        {
            pp->cmdCompleted(result);
            break;
        }

        case ExifToolProcess::ERROR_RESULT:
        {
            pp->errorOccurred(result,
                              proc->exifToolError(),
                              proc->exifToolErrorString());
            break;
        }

        case ExifToolProcess::FINISH_RESULT:
        {
            pp->finished();
            break;
        }
    }

    if (currentPath.isEmpty())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "ExifTool complete"
                                        << actionString(result.cmdAction);
    }
    else
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "ExifTool complete"
                                        << actionString(result.cmdAction) << "for" << currentPath;
    }
}

QString ExifToolParser::Private::actionString(int cmdAction) const
{
    switch (cmdAction)
    {
        case ExifToolProcess::LOAD_METADATA:
        {
            return QLatin1String("Load Metadata");
        }

        case ExifToolProcess::LOAD_CHUNKS:
        {
            return QLatin1String("Load Chunks");
        }

        case ExifToolProcess::APPLY_CHANGES:
        {
            return QLatin1String("Apply Changes");
        }

        case ExifToolProcess::APPLY_CHANGES_EXV:
        {
            return QLatin1String("Apply Changes EXV");
        }

        case ExifToolProcess::APPLY_METADATA_FILE:
        {
            return QLatin1String("Apply Metadata File");
        }

        case ExifToolProcess::CHANGE_TIMESTAMPS:
        {
            return QLatin1String("Change Timestamps");
        }

        case ExifToolProcess::READ_FORMATS:
        {
            return QLatin1String("Readable Formats");
        }

        case ExifToolProcess::WRITE_FORMATS:
        {
            return QLatin1String("Writable Formats");
        }

        case ExifToolProcess::TRANSLATIONS_LIST:
        {
            return QLatin1String("Translations List");
        }

        case ExifToolProcess::TAGS_DATABASE:
        {
            return QLatin1String("Tags Database");
        }

        case ExifToolProcess::VERSION_STRING:
        {
            return QLatin1String("Version String");
        }

        case ExifToolProcess::COPY_TAGS:
        {
            return QLatin1String("Copy Tags");
        }

        case ExifToolProcess::TRANS_TAGS:
        {
            return QLatin1String("Translate Tags");
        }

        case ExifToolProcess::EXTRACT_MOTION_PHOTO_VIDEO:
        {
            return QLatin1String("Extract Motion Photo Video");
        }

        default: // ExifToolProcess::NO_ACTION
        {
            break;
        }
    }

    return QString();
}

} // namespace Digikam
