/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "FileStoragePolicy.h"

// Qt includes

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>

// Local includes

#include "MarbleGlobal.h"
#include "MarbleDirs.h"
#include "digikam_debug.h"

namespace Marble
{

FileStoragePolicy::FileStoragePolicy(const QString& dataDirectory, QObject* parent)
    : StoragePolicy(parent),
      m_dataDirectory(dataDirectory)
{
    if (m_dataDirectory.isEmpty())
    {
        m_dataDirectory = MarbleDirs::localPath() + QLatin1String("/cache/");
    }

    if (!QDir(m_dataDirectory).exists())
    {
        QDir::root().mkpath(m_dataDirectory);
    }
}

FileStoragePolicy::~FileStoragePolicy()
{
}

bool FileStoragePolicy::fileExists(const QString& fileName) const
{
    const QString fullName = m_dataDirectory + QLatin1Char('/') + fileName;
    return QFile::exists(fullName);
}

bool FileStoragePolicy::updateFile(const QString& fileName, const QByteArray& data)
{
    QFileInfo const dirInfo(fileName);
    QString const fullName = dirInfo.isAbsolute() ? fileName : m_dataDirectory + QLatin1Char('/') + fileName;

    // Create directory if it doesn't exist yet...
    QFileInfo info(fullName);

    const QDir localFileDir = info.dir();
    const QString localFileDirPath = localFileDir.absolutePath();

    if (!QDir(localFileDirPath).exists())
    {
        QDir::root().mkpath(localFileDirPath);
    }

    // ... and save the file content
    QFile file(fullName);

    if (!file.open(QIODevice::WriteOnly))
    {
        m_errorMsg = fullName + QLatin1String(": ") + file.errorString();
        qCCritical(DIGIKAM_GEOENGINE_LOG) << "file.open" << m_errorMsg;
        return false;
    }

    quint64 oldSize = file.size();

    if (!file.write(data))
    {
        m_errorMsg = fullName + QLatin1String(": ") + file.errorString();
        qCCritical(DIGIKAM_GEOENGINE_LOG) << "file.write" << m_errorMsg;
        Q_EMIT sizeChanged(file.size() - oldSize);
        return false;
    }

    Q_EMIT sizeChanged(file.size() - oldSize);
    file.close();

    return true;
}

void FileStoragePolicy::clearCache()
{
    qCDebug(DIGIKAM_GEOENGINE_LOG) << Q_FUNC_INFO;

    if (m_dataDirectory.isEmpty()
        || !(m_dataDirectory.endsWith(QLatin1String("data"))     // on Windows
             ||   m_dataDirectory.endsWith(QLatin1String("marble"))   // on all other OSes
            ))
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG) << "Data Directory:" << m_dataDirectory;
        qCDebug(DIGIKAM_GEOENGINE_LOG) << "Error: Refusing to erase files under unknown conditions for safety reasons!";
        return;
    }

    const QString cachedMapsDirectory = m_dataDirectory + QLatin1String("/maps");

    QDirIterator it(cachedMapsDirectory, QDir::NoDotAndDotDot | QDir::Dirs);
    qCDebug(DIGIKAM_GEOENGINE_LOG) << cachedMapsDirectory;

    while (it.hasNext())
    {
        it.next();
        QString planetDirectory = it.filePath();
        QDirIterator itPlanet(planetDirectory, QDir::NoDotAndDotDot | QDir::Dirs);

        while (itPlanet.hasNext())
        {
            itPlanet.next();
            QString themeDirectory = itPlanet.filePath();
            QDirIterator itTheme(themeDirectory, QDir::NoDotAndDotDot | QDir::Dirs);

            while (itTheme.hasNext())
            {
                itTheme.next();
                QString tileDirectory = itTheme.filePath();

                if (itTheme.fileName().toInt() <= maxBaseTileLevel)
                {
                    continue;
                }

                QDirIterator itTile(tileDirectory, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);

                while (itTile.hasNext())
                {
                    itTile.next();
                    QString filePath = itTile.filePath();
                    QString lowerCase = filePath.toLower();

                    // We try to be very careful and just delete images
                    // FIXME, when vectortiling I suppose also vector tiles will have
                    // to be deleted
                    if (lowerCase.endsWith(QLatin1String(".jpg"))
                        || lowerCase.endsWith(QLatin1String(".png"))
                        || lowerCase.endsWith(QLatin1String(".gif"))
                        || lowerCase.endsWith(QLatin1String(".svg"))
                        || lowerCase.endsWith(QLatin1String(".o5m"))
                       )
                    {
                        // We cannot Q_EMIT clear, because we don't make a full clear
                        QFile file(filePath);
                        Q_EMIT sizeChanged(-file.size());
                        file.remove();
                    }
                }
            }
        }
    }
}

QString FileStoragePolicy::lastErrorMessage() const
{
    return m_errorMsg;
}

} // namespace Marble

#include "moc_FileStoragePolicy.cpp"
