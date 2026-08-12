/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-12
 * Description : Web Service tool utils methods
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "wstoolutils.h"

// Qt includes

#include <QRandomGenerator>
#include <QStandardPaths>
#include <QApplication>
#include <QByteArray>
#include <QBuffer>
#include <QTime>

namespace Digikam
{

QString WSToolUtils::decodeKey(const QString& key)
{
    QByteArray keyArray = QByteArray::fromBase64(key.toLatin1());
    int pos             = 0;

    for (int i = 0 ; i < keyArray.size() ; ++i)
    {
        keyArray[i] = (keyArray[i] ^ possibleCharacters().at(pos).toLatin1());
        pos         = (++pos == possibleCharacters().length()) ? 0 : pos;
    }

    return QString::fromLatin1(keyArray);
}

QString WSToolUtils::randomString(const int& length)
{
    QString randomString;
    QRandomGenerator* const generator = QRandomGenerator::global();

    for (int i = 0 ; i < length ; ++i)
    {
        const int index = generator->bounded(possibleCharacters().length());
        QChar nextChar  = possibleCharacters().at(index);
        randomString.append(nextChar);
    }

    return randomString;
}

// ------------------------------------------------------------------------------------

QDir WSToolUtils::makeTemporaryDir(const char* prefix)
{
    QString subDir = QString::fromLatin1("digikam-%1-%2").arg(QString::fromUtf8(prefix)).arg(qApp->applicationPid());
    QString path   = QDir(QDir::tempPath()).filePath(subDir);

    if (!QDir().exists(path))
    {
        QDir().mkpath(path);
    }

    return QDir(path);
}

// ------------------------------------------------------------------------------------

void WSToolUtils::removeTemporaryDir(const char* prefix)
{
    QString subDir = QString::fromLatin1("digikam-%1-%2").arg(QString::fromUtf8(prefix)).arg(qApp->applicationPid());
    QString path   = QDir(QDir::tempPath()).filePath(subDir);

    if (QDir().exists(path))
    {
        QDir(path).removeRecursively();
    }
}

// ------------------------------------------------------------------------------------

QSettings* WSToolUtils::getOauthSettings(QObject* const parent)
{
    QString dkoauth = oauthConfigFile();

    return (new QSettings(dkoauth, QSettings::IniFormat, parent));
}

// ------------------------------------------------------------------------------------

void WSToolUtils::saveToken(const QString& name, const QString& token)
{
    QString dkoauth = oauthConfigFile();

    QSettings settings(dkoauth, QSettings::IniFormat);
    QByteArray code = token.toLatin1().toBase64();

    settings.beginGroup(name);
    settings.setValue(QLatin1String("refreshToken"), code);
    settings.endGroup();
}

QString WSToolUtils::readToken(const QString& name)
{
    QString dkoauth = oauthConfigFile();

    QSettings settings(dkoauth, QSettings::IniFormat);

    settings.beginGroup(name);
    QByteArray code = settings.value(QLatin1String("refreshToken")).toByteArray();
    settings.endGroup();

    return QString::fromLatin1(QByteArray::fromBase64(code));
}

void WSToolUtils::clearToken(const QString& name)
{
    QString dkoauth = oauthConfigFile();

    QSettings settings(dkoauth, QSettings::IniFormat);

    settings.beginGroup(name);
    settings.remove(QString());
    settings.endGroup();
}

// ------------------------------------------------------------------------------------

QString WSToolUtils::oauthConfigFile()
{
    QString dkoauth = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
                      QLatin1String("/digikam_oauthrc");

    return dkoauth;
}

// ------------------------------------------------------------------------------------

QString WSToolUtils::possibleCharacters()
{
    return QLatin1String("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
}

} // namespace Digikam
