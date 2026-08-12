/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-02-05
 * Description : undo cache manager for image editor
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005      by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "undocache.h"

// Qt includes

#include <QApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN UndoCache::Private
{
public:

    Private() = default;

    QString cacheFile(int level) const
    {
        return QString::fromUtf8("%1-%2.bin").arg(cachePrefix).arg(level);
    }

public:

    QString   cacheDir;
    QString   cachePrefix;
    QSet<int> cachedLevels;

    bool      cacheError = false;
};

UndoCache::UndoCache()
    : d(new Private)
{
    d->cacheDir    = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1Char('/');

    d->cachePrefix = QString::fromUtf8("%1undocache-%2")
                     .arg(d->cacheDir)
                     .arg(QCoreApplication::applicationPid());

    // remove any remnants

    QDir dir(d->cacheDir);
    const auto infs = dir.entryInfoList(QStringList() << QLatin1String("undocache-*"));

    for (const QFileInfo& info : infs)
    {
        QFile(info.filePath()).remove();
    }
}

UndoCache::~UndoCache()
{
    clear();
    delete d;
}

void UndoCache::clear()
{
    for (int level : std::as_const(d->cachedLevels))
    {
        QFile(d->cacheFile(level)).remove();
    }

    d->cachedLevels.clear();
}

void UndoCache::clearFrom(int fromLevel)
{
    for (int level : std::as_const(d->cachedLevels))
    {
        if (level >= fromLevel)
        {
            QFile(d->cacheFile(level)).remove();
            d->cachedLevels.remove(level);
        }
    }
}

bool UndoCache::putData(int level, const DImg& img) const
{
    if (d->cacheError)
    {
        return false;
    }

    QStorageInfo info(d->cacheDir);

    qint64 fspace = (info.bytesAvailable() / 1024 / 1024);
    qCDebug(DIGIKAM_GENERAL_LOG) << "Free space available in Editor cache [" << d->cacheDir << "] in Mbytes:" << fspace;

    if (fspace < 2048)              // Check if free space is over 2 GiB to put data in cache.
    {
        if (!qApp->activeWindow())  // Special case for the Jenkins build server.
        {
            return false;
        }

        QApplication::restoreOverrideCursor();

        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("The free disk space in the path \"%1\" for the undo "
                                   "cache file is less than to 2 GiB! Undo cache is now disabled!",
                                   QDir::toNativeSeparators(d->cacheDir)));
        d->cacheError = true;

        return false;
    }

    QFile file(d->cacheFile(level));

    if (file.exists() || !file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QDataStream ds(&file);
    ds << (quint32)img.width();
    ds << (quint32)img.height();
    ds << img.numBytes();
    ds << img.hasAlpha();
    ds << img.sixteenBit();

    file.write(reinterpret_cast<const char*>(img.bits()), img.numBytes());

    if (file.error() != QFileDevice::NoError)
    {
        file.close();
        file.remove();

        return false;
    }

    d->cachedLevels << level;

    file.close();

    return true;
}

DImg UndoCache::getData(int level) const
{
    quint32 w          = 0;
    quint32 h          = 0;
    quint64 numBytes   = 0;
    bool    hasAlpha   = false;
    bool    sixteenBit = false;

    QFile file(d->cacheFile(level));

    if (!file.open(QIODevice::ReadOnly))
    {
        return DImg();
    }

    QDataStream ds(&file);
    ds >> w;
    ds >> h;
    ds >> numBytes;
    ds >> hasAlpha;
    ds >> sixteenBit;

    if (ds.status() != QDataStream::Ok || ds.atEnd())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "The undo cache file is corrupt";

        file.close();

        return DImg();
    }

    DImg img(w, h, sixteenBit, hasAlpha);

    if (img.isNull() || (numBytes != img.numBytes()))
    {
        file.close();

        return DImg();
    }

    qint64 readBytes = file.read(reinterpret_cast<char*>(img.bits()), numBytes);

    if ((file.error() != QFileDevice::NoError) || ((quint64)readBytes != numBytes))
    {
        file.close();

        return DImg();
    }

    file.close();

    return img;
}

} // namespace Digikam
