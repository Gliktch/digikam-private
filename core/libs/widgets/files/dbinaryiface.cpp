/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-23
 * Description : Autodetect binary program and version
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbinaryiface.h"

// Qt includes

#include <QProcess>
#include <QMessageBox>
#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "dfiledialog.h"
#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_globals.h"

namespace Digikam
{

DBinaryIface::DBinaryIface(const QString& binaryName,
                           const QString& projectName,
                           const QString& url,
                           const QString& toolName,
                           const QStringList& args,
                           const QString& desc)
    : m_checkVersion        (false),
      m_headerStarts        (QLatin1String("")),
      m_headerLine          (0),
      m_minimalVersion      (QLatin1String("")),
      m_configGroup         (!toolName.isEmpty() ? QString::fromLatin1("%1 Settings").arg(toolName) : QLatin1String("")),
      m_binaryBaseName      (goodBaseName(binaryName)),
      m_binaryArguments     (args),
      m_projectName         (projectName),
      m_url                 (QUrl(url)),
      m_description         (desc)
{
}

DBinaryIface::DBinaryIface(const QString& binaryName,
                           const QString& minimalVersion,
                           const QString& header,
                           const int headerLine,
                           const QString& projectName,
                           const QString& url,
                           const QString& toolName,
                           const QStringList& args,
                           const QString& desc)
    : m_checkVersion        (true),
      m_headerStarts        (header),
      m_headerLine          (headerLine),
      m_minimalVersion      (minimalVersion),
      m_configGroup         (!toolName.isEmpty() ? QString::fromLatin1("%1 Settings").arg(toolName) : QLatin1String("")),
      m_binaryBaseName      (goodBaseName(binaryName)),
      m_binaryArguments     (args),
      m_projectName         (projectName),
      m_url                 (QUrl(url)),
      m_description         (desc)
{
}

const QString& DBinaryIface::version() const
{
    return m_version;
}

bool DBinaryIface::versionIsRight() const
{
    if (!m_checkVersion)
    {
        return true;
    }

    QRegularExpression verRegExp(QLatin1String("^(\\d*[.]\\d*)"));
    float floatVersion = verRegExp.match(version()).captured(0).toFloat();

    return (
            !version().isNull() &&
            isFound()           &&
            (floatVersion >= minimalVersion().toFloat())
           );
}

bool DBinaryIface::versionIsRight(const float customVersion) const
{
    if (!m_checkVersion)
    {
        return true;
    }

    QRegularExpression verRegExp(QLatin1String("^(\\d*[.]\\d*)"));
    float floatVersion = verRegExp.match(version()).captured(0).toFloat();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Found (" << isFound()
                                 << ") :: Version : " << version()
                                 << "(" << floatVersion
                                 << ")  [" << customVersion << "]";

    return (
            !version().isNull() &&
            isFound()           &&
            (floatVersion >= customVersion)
           );
}

QString DBinaryIface::findHeader(const QStringList& output, const QString& header) const
{
    for (const QString& s : std::as_const(output))
    {
        if (s.startsWith(header))
        {   // cppcheck-suppress useStlAlgorithm
            return s;
        }
    }

    return QString();
}

bool DBinaryIface::parseHeader(const QString& output)
{
    QString firstLine = output.section(QLatin1Char('\n'), m_headerLine, m_headerLine);
    qCDebug(DIGIKAM_GENERAL_LOG) << path() << " help header line: \n" << firstLine;

    if (firstLine.startsWith(m_headerStarts))
    {
        QString ver = firstLine.remove(0, m_headerStarts.length());

        if (ver.startsWith(QLatin1String("Pre-Release ")))
        {
            ver.remove(QLatin1String("Pre-Release "));            // Special case with Hugin beta.
            m_developmentVersion = true;
        }

        setVersion(ver);

        return true;
    }

    return false;
}

void DBinaryIface::setVersion(QString& version)
{
    QRegularExpression verRegExp(QLatin1String("\\d*(\\.\\d+)*"));
    m_version = verRegExp.match(version).captured(0);
}

void DBinaryIface::slotNavigateAndCheck()
{
    QUrl start;

    if (isValid() && !m_pathDir.isEmpty())
    {
        start = QUrl::fromLocalFile(m_pathDir);
    }
    else
    {

#if defined Q_OS_MACOS

        start = QUrl::fromLocalFile(QLatin1String("/Applications/"));

#elif defined Q_OS_WIN

        start = QUrl::fromLocalFile(QLatin1String("C:/Program Files/"));

#else

        start = QUrl::fromLocalFile(QLatin1String("/usr/bin/"));

#endif

    }

    QString f   = DFileDialog::getOpenFileName(nullptr, i18nc("@title:window", "Navigate to %1", m_binaryBaseName),
                                               start.toLocalFile(),
                                               m_binaryBaseName);

    QString dir = QUrl::fromLocalFile(f).adjusted(QUrl::RemoveFilename).toLocalFile();
    m_searchPaths << dir;

    if (checkDirForPath(dir))
    {
        Q_EMIT signalSearchDirectoryAdded(dir);
    }
}

void DBinaryIface::slotAddPossibleSearchDirectory(const QString& dir)
{
    if (!isValid())
    {
        m_searchPaths << dir;
        checkDirForPath(dir);
    }
    else
    {
        m_searchPaths << dir;
    }
}

void DBinaryIface::slotAddSearchDirectory(const QString& dir)
{
    m_searchPaths << dir;
    checkDirForPath(dir);       // Forces the use of that directory
}

QString DBinaryIface::readConfig()
{
    if (m_configGroup.isEmpty())
    {
        return QLatin1String("");
    }

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(m_configGroup);

    return group.readPathEntry(QString::fromUtf8("%1Binary").arg(m_binaryBaseName), QLatin1String(""));
}

void DBinaryIface::writeConfig()
{
    if (m_configGroup.isEmpty())
    {
        return;
    }

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(m_configGroup);
    group.writePathEntry(QString::fromUtf8("%1Binary").arg(m_binaryBaseName), m_pathDir);
}

QString DBinaryIface::path(const QString& dir) const
{
    if (dir.isEmpty())
    {
        return baseName();
    }

    if (dir.endsWith(QLatin1Char('/')))
    {
        return QString::fromUtf8("%1%2").arg(dir).arg(baseName());
    }

    return QString::fromUtf8("%1/%2").arg(dir).arg(baseName());
}

void DBinaryIface::setup(const QString& prev)
{
    QString previousDir = prev;

    if (!previousDir.isEmpty())
    {
        m_searchPaths << previousDir;
        checkDirForPath(previousDir);

        return;
    }

    previousDir         = readConfig();
    m_searchPaths << previousDir;
    checkDirForPath(previousDir);

    if ((!previousDir.isEmpty()) && !isValid())
    {
        m_searchPaths << QLatin1String("");
        checkDirForPath(QLatin1String(""));
    }
}

bool DBinaryIface::checkDirForPath(const QString& possibleDir)
{
    bool ret             = false;
    QString possiblePath = path(possibleDir);

    qCDebug(DIGIKAM_GENERAL_LOG) << "Testing " << possiblePath << "...";

    if (m_binaryArguments.isEmpty())
    {
        if (QFile::exists(possiblePath))
        {
            m_pathDir = possibleDir;
            m_isFound = true;
            writeConfig();

            qCDebug(DIGIKAM_GENERAL_LOG) << "Found " << path();
            ret       = true;
        }
    }
    else
    {
        QProcess process;
        process.setProcessChannelMode(QProcess::MergedChannels);
        process.setProcessEnvironment(adjustedEnvironmentForAppImage());
        process.start(possiblePath, m_binaryArguments);

        bool val = process.waitForFinished();

        if (val && (process.error() != QProcess::FailedToStart))
        {
            m_isFound = true;

            if (m_checkVersion)
            {
                QString stdOut = QString::fromUtf8(process.readAllStandardOutput());

                if (parseHeader(stdOut))
                {
                    m_pathDir = possibleDir;
                    writeConfig();

                    qCDebug(DIGIKAM_GENERAL_LOG) << "Found " << path() << " version: " << version();
                    ret       = true;
                }
                else
                {
                    m_hasError = true;
                }
            }
            else
            {
                m_pathDir = possibleDir;
                writeConfig();

                qCDebug(DIGIKAM_GENERAL_LOG) << "Found " << path();
                ret       = true;
            }
        }
        else if (QFile::exists(possiblePath))
        {
            m_hasError = true;
        }
    }

    Q_EMIT signalBinaryValid();

    return ret;
}

bool DBinaryIface::recheckDirectories()
{
    if (isValid())
    {
        // No need for recheck if it is already valid...

        return true;
    }

    for (const QString& dir : std::as_const(m_searchPaths))
    {
        checkDirForPath(dir);

        if (isValid())
        {
            return true;
        }
    }

    return false;
}

QString DBinaryIface::goodBaseName(const QString& b)
{

#ifdef Q_OS_WIN

    if (b.endsWith(QLatin1String(".jar")))
    {
        // Special case if we check a java archive.

        return b;
    }
    else
    {
        return b + QLatin1String(".exe");
    }

#else

    return b;

#endif // Q_OS_WIN

}

} // namespace Digikam

#include "moc_dbinaryiface.cpp"
