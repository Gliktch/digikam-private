/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : central place for ICC settings - Private implementations.
 *
 * SPDX-FileCopyrightText: 2005-2006 by F.J. Cruz <fj dot cruz at supercable dot es>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iccsettings_p.h"

namespace Digikam
{

IccSettings::Private::Private()
    : settings   (ICCSettingsContainer()),
      configGroup(QLatin1String("Color Management"))
{
}

void IccSettings::Private::scanDirectory(const QString& path, const QStringList& filter, QList<IccProfile>* const profiles)
{
    if ((path.count(QLatin1Char('/')) - dirDepth) > 2)
    {
        return;
    }

    QDir          dir(path);
    QFileInfoList infos;
    infos << dir.entryInfoList(filter, QDir::Files | QDir::Readable);
    infos << dir.entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);

    for (const QFileInfo& info : std::as_const(infos))
    {
        if      (info.isFile())
        {
/*
            qCDebug(DIGIKAM_DIMG_LOG) << info.filePath() << (info.exists() && info.isReadable());
*/
            IccProfile profile(info.filePath());

            if (profile.open())
            {
                *profiles << profile;

                if (info.fileName() == QLatin1String("AdobeRGB1998.icc"))
                {
                    IccProfile::considerOriginalAdobeRGB(info.filePath());
                }
            }
        }
        else if (info.isDir() && !info.isSymLink())
        {
            scanDirectory(info.filePath(), filter, profiles);
        }
    }
}

QList<IccProfile> IccSettings::Private::scanDirectories(const QStringList& dirs)
{
    QList<IccProfile> profiles;
    QStringList       filters;
    filters << QLatin1String("*.icc") << QLatin1String("*.icm");
    qCDebug(DIGIKAM_DIMG_LOG) << dirs;

    for (const QString& dirPath : std::as_const(dirs))
    {
        QDir dir(dirPath);

        if (!dir.exists())
        {
            continue;
        }

        dirDepth = dir.path().count(QLatin1Char('/'));
        scanDirectory(dir.path(), filters, &profiles);
    }

    return profiles;
}

ICCSettingsContainer IccSettings::Private::readFromConfig() const
{
    ICCSettingsContainer s;
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    s.readFromConfig(group);

    return s;
}

void IccSettings::Private::writeToConfig() const
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    settings.writeToConfig(group);
}

void IccSettings::Private::writeManagedViewToConfig() const
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    settings.writeManagedViewToConfig(group);
}

void IccSettings::Private::writeManagedPreviewsToConfig() const
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    settings.writeManagedPreviewsToConfig(group);
}

} // namespace Digikam
