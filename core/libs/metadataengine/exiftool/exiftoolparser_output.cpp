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

void ExifToolParser::printExifToolOutput(const QByteArray& stdOut)
{
    qCDebug(DIGIKAM_METAENGINE_LOG) << "ExifTool output:";
    qCDebug(DIGIKAM_METAENGINE_LOG) << "---";
    qCDebug(DIGIKAM_METAENGINE_LOG).noquote() << stdOut;
    qCDebug(DIGIKAM_METAENGINE_LOG) << "---";

    d->error = stdOut.contains(QByteArray("1 files were not updated due to errors"));
}

void ExifToolParser::cmdCompleted(const ExifToolProcess::Result& result)
{
    qCDebug(DIGIKAM_METAENGINE_LOG) << "ExifTool complete command for action"
                                    << d->actionString(result.cmdAction)
                                    << "with elapsed time (ms):"
                                    << result.elapsed;

    ExifToolData exifToolData;

    switch (result.cmdAction)
    {
        case ExifToolProcess::LOAD_METADATA:
        {
            // Convert JSON array as QVariantMap

            QJsonDocument jsonDoc     = QJsonDocument::fromJson(result.output);
            QJsonArray    jsonArray   = jsonDoc.array();

            if (jsonArray.size() == 0)
            {
                qCDebug(DIGIKAM_METAENGINE_LOG) << "Json Array size is null";

                Q_EMIT signalExifToolDataAvailable();

                if (d->async)
                {
                    Q_EMIT signalExifToolAsyncData(exifToolData);
                }

                return;
            }

            QJsonObject jsonObject    = jsonArray.at(0).toObject();
            QVariantMap metadataMap   = jsonObject.toVariantMap();

            qCDebug(DIGIKAM_METAENGINE_LOG) << "ExifTool Json map size:" << metadataMap.size();

            for (QVariantMap::const_iterator it = metadataMap.constBegin() ;
                it != metadataMap.constEnd() ; ++it)
            {
                QString     tagNameExifTool;
                QString     tagType;
                QStringList sections  = it.key().split(QLatin1Char(':'));

                if      (sections.size() == 6)      // With ExifTool > 12.00 (at least under Windows or macOS), groups are return with 6 sections.
                {
                    tagNameExifTool = QString::fromLatin1("%1.%2.%3.%4")
                                          .arg(sections[0])
                                          .arg(sections[1])
                                          .arg(sections[2])
                                          .arg(sections[5]);
                    tagType         = sections[4];
                }
                else if (sections.size() == 5)      // ExifTool 12.00 under Linux return 5 or 4 sections.
                {
                    tagNameExifTool = QString::fromLatin1("%1.%2.%3.%4")
                                          .arg(sections[0])
                                          .arg(sections[1])
                                          .arg(sections[2])
                                          .arg(sections[4]);
                    tagType         = sections[3];
                }
                else if (sections.size() == 4)
                {
                    tagNameExifTool = QString::fromLatin1("%1.%2.%3.%4")
                                          .arg(sections[0])
                                          .arg(sections[1])
                                          .arg(sections[2])
                                          .arg(sections[3]);
                }
                else if (sections[0] == QLatin1String("SourceFile"))
                {
                    d->currentPath = it.value().toString();
                    continue;
                }
                else
                {
                    continue;
                }

                QVariantMap propsMap = it.value().toMap();
                QString data;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                if (propsMap.find(QLatin1String("val")).value().typeId() == QMetaType::QVariantList)

#else

                if (propsMap.find(QLatin1String("val")).value().type() == QVariant::List)

#endif

                {
                    QStringList list = propsMap.find(QLatin1String("val")).value().toStringList();
                    data             = list.join(QLatin1String(", "));
                }
                else
                {
                    data             = propsMap.find(QLatin1String("val")).value().toString();
                }

                QString desc              = propsMap.find(QLatin1String("desc")).value().toString();

                // Optional numerical value extraction, if any

                QString num;
                QVariantMap::iterator it2 = propsMap.find(QLatin1String("num"));

                if (it2 != propsMap.end())
                {
                    num = it2.value().toString();
                }
/*
                qCDebug(DIGIKAM_METAENGINE_LOG) << "ExifTool json property:" << tagNameExifTool << data;
*/

                if (
                    data.startsWith(QLatin1String("(Binary data ")) &&
                    data.endsWith(QLatin1String(", use -b option to extract)"))
                   )
                {
                    data = data.section(QLatin1Char(','), 0, 0);
                    data.remove(QLatin1Char('('));
                }

                if (exifToolData.contains(tagNameExifTool))
                {
                    QString existData = exifToolData[tagNameExifTool][0].toString();
                    existData        += QLatin1String(", ") + data;
                    exifToolData[tagNameExifTool][0] = existData;
                }
                else
                {
                    exifToolData.insert(tagNameExifTool, QVariantList()
                                                            << data        // ExifTool Raw data as string.
                                                            << tagType     // ExifTool data type.
                                                            << desc        // ExifTool tag description.
                                                            << num);       // ExifTool numeral value if any.
                }
            }

            break;
        }

        case ExifToolProcess::LOAD_CHUNKS:
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "EXV chunk size:" << result.output.size();

            exifToolData.insert(QLatin1String("EXV"), QVariantList() << result.output);     // Exv chunk as bytearray.
            break;
        }

        case ExifToolProcess::EXTRACT_MOTION_PHOTO_VIDEO:
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Motion photo video size:" << result.output.size();

            exifToolData.insert(QLatin1String("MOTION_PHOTO_VIDEO"), QVariantList() << result.output);
            break;
        }

        case ExifToolProcess::APPLY_CHANGES:
        {
            printExifToolOutput(result.output);
            break;
        }

        case ExifToolProcess::APPLY_CHANGES_EXV:
        {
            printExifToolOutput(result.output);
            break;
        }

        case ExifToolProcess::APPLY_METADATA_FILE:
        {
            printExifToolOutput(result.output);
            break;
        }

        case ExifToolProcess::CHANGE_TIMESTAMPS:
        {
            printExifToolOutput(result.output);
            break;
        }

        case ExifToolProcess::COPY_TAGS:
        {
            printExifToolOutput(result.output);
            break;
        }

        case ExifToolProcess::TRANS_TAGS:
        {
            printExifToolOutput(result.output);

            if (!d->argsFile.isOpen() && d->argsFile.exists())
            {
                d->argsFile.remove();
            }

            break;
        }

        case ExifToolProcess::READ_FORMATS:
        {
            // Remove first line

            QString out       = QString::fromUtf8(result.output).section(QLatin1Char('\n'), 1, -1);

            // Get extensions and descriptions as pair of strings

            QStringList lines = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
            QStringList lst;
            QString s;

            for (const QString& ln : EXIV2_AS_CONST(lines))
            {
                s            = ln.simplified();
                QString ext  = s.section(QLatin1Char(' '), 0, 0);
                QString desc = s.section(QLatin1Char(' '), 1, -1);
                lst << ext << desc;
            }

            exifToolData.insert(QLatin1String("READ_FORMATS"), QVariantList() << lst);
            break;
        }

        case ExifToolProcess::WRITE_FORMATS:
        {
            // Remove first line

            QString out       = QString::fromUtf8(result.output).section(QLatin1Char('\n'), 1, -1);

            // Get extensions and descriptions as pair of strings

            QStringList lines = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
            QStringList lst;
            QString s;

            for (const QString& ln : EXIV2_AS_CONST(lines))
            {
                s            = ln.simplified();
                QString ext  = s.section(QLatin1Char(' '), 0, 0);
                QString desc = s.section(QLatin1Char(' '), 1, -1);
                lst << ext << desc;
            }

            exifToolData.insert(QLatin1String("WRITE_FORMATS"), QVariantList() << lst);
            break;
        }

        case ExifToolProcess::TRANSLATIONS_LIST:
        {
            // Remove first line

            QString out       = QString::fromUtf8(result.output).section(QLatin1Char('\n'), 1, -1);

            // Get i18n list

            QStringList lines = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
            QStringList lst;

            for (const QString& ln : EXIV2_AS_CONST(lines))
            {
                lst << ln.simplified().section(QLatin1String(" - "), 0, 0);
            }

            exifToolData.insert(QLatin1String("TRANSLATIONS_LIST"), QVariantList() << lst);
            break;
        }

        case ExifToolProcess::TAGS_DATABASE:
        {
            QString xml = QString::fromUtf8(result.output);

            QDomDocument doc;

            if (doc.setContent(xml))
            {
                QDomElement docElem = doc.documentElement();

                if (docElem.tagName() == QLatin1String("taginfo"))
                {
                    for (QDomNode n1 = docElem.firstChild() ; !n1.isNull() ; n1 = n1.nextSibling())
                    {
                        QDomElement e1 = n1.toElement();

                        if (!e1.isNull())
                        {
                            if (e1.tagName() == QLatin1String("table"))                           // Top level group
                            {
                                QString g0       = e1.attribute(QLatin1String("g0"));
                                QString g1       = e1.attribute(QLatin1String("g1"));
                                QString g2       = e1.attribute(QLatin1String("g2"));
                                QString type;
                                QString writable;
                                QString tag;
                                QString mainDesc;
                                QString desc;

                                for (QDomNode n2 = e1.firstChild() ; !n2.isNull() ; n2 = n2.nextSibling())
                                {
                                    QDomElement e2 = n2.toElement();

                                    if (!e2.isNull())
                                    {
                                        if      (e2.tagName() == QLatin1String("desc"))          // Main description of group
                                        {
                                            if (e2.attribute(QLatin1String("lang")) == QLatin1String("en"))
                                            {
                                                mainDesc = e2.text();
                                            }

                                            continue;
                                        }
                                        else if (e2.tagName() == QLatin1String("tag"))           // One tag from group
                                        {
                                            QString a1   = e2.attribute(QLatin1String("g1"));
                                            QString a2   = e2.attribute(QLatin1String("g2"));
                                            QString name = e2.attribute(QLatin1String("name"));
                                            tag          = QString::fromLatin1("%1.%2.%3.%4").arg(g0)
                                                                                             .arg(a1.isNull() ? g1 : a1)
                                                                                             .arg(a2.isNull() ? g2 : a2)
                                                                                             .arg(name);
                                            type         = e2.attribute(QLatin1String("type"));
                                            writable     = e2.attribute(QLatin1String("writable"));

                                            for (QDomNode n3 = e2.firstChild() ; !n3.isNull() ; n3 = n3.nextSibling())
                                            {
                                                QDomElement e3 = n3.toElement();

                                                if (!e3.isNull())
                                                {
                                                    if (e3.tagName() == QLatin1String("desc"))  // Description of tag
                                                    {
                                                        if (e3.attribute(QLatin1String("lang")) == QLatin1String("en"))
                                                        {
                                                            desc = e3.text();
                                                            break;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    exifToolData.insert(tag,
                                                        QVariantList()
                                                            << QString::fromLatin1("%1 - %2").arg(mainDesc).arg(desc)
                                                            << type
                                                            << writable
                                    );
                                }
                            }
                        }
                    }
                }
            }

            break;
        }

        case ExifToolProcess::VERSION_STRING:
        {
            QString out       = QString::fromUtf8(result.output);
            QStringList lines = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

            if (!lines.isEmpty())
            {
                exifToolData.insert(QLatin1String("VERSION_STRING"), QVariantList() << lines.first());
            }

            break;
        }

        default:
        {
            break;
        }
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << "ExifTool parsed command for action"
                                    << d->actionString(result.cmdAction)
                                    << exifToolData.count()
                                    << "properties decoded";

    d->exifToolData = exifToolData;

    Q_EMIT signalExifToolDataAvailable();

    if (d->async)
    {
        Q_EMIT signalExifToolAsyncData(exifToolData);
    }
}

void ExifToolParser::errorOccurred(const ExifToolProcess::Result& result,
                                   QProcess::ProcessError error,
                                   const QString& description)
{
    qCWarning(DIGIKAM_METAENGINE_LOG) << "ExifTool process for action"
                                      << d->actionString(result.cmdAction)
                                      << "exited with error:"
                                      << error;

    d->error       = true;
    d->errorString = description;

    Q_EMIT signalExifToolDataAvailable();
}

void ExifToolParser::finished()
{
    Q_EMIT signalExifToolDataAvailable();
}

void ExifToolParser::slotExifToolResult(int cmdId)
{
    {
        QMutexLocker locker(&d->mutex);

        if (!d->asyncRunning.contains(cmdId))
        {
            return;
        }

        d->asyncRunning.removeAll(cmdId);
    }

    d->jumpToResultCommand(d->proc->getExifToolResult(cmdId), cmdId);
}

void ExifToolParser::setOutputStream(int cmdAction,
                                     const QByteArray& cmdOutputChannel,
                                     const QByteArray& /*cmdErrorChannel*/)
{
    ExifToolProcess::Result result;

    result.cmdAction = cmdAction;
    result.output    = cmdOutputChannel;

    cmdCompleted(result);
}

} // namespace Digikam
