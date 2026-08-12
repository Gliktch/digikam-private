/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-22
 * Description : central place for metadata settings
 *
 * SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmetadatasettings.h"

// Qt includes

#include <QDir>
#include <QFileInfo>
#include <QMutex>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN DMetadataSettings::Private
{
public:

    Private() = default;

    DMetadataSettingsContainer settings;
    QMutex                     mutex;

    const QString              configGroup = QLatin1String("DMetadata Settings");

public:

    DMetadataSettingsContainer readFromConfig() const;
    void                       writeToConfig()  const;
    DMetadataSettingsContainer setSettings(const DMetadataSettingsContainer& s);
};

DMetadataSettingsContainer DMetadataSettings::Private::readFromConfig() const
{
    DMetadataSettingsContainer s;
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    s.readFromConfig(group);

    return s;
}

void DMetadataSettings::Private::writeToConfig() const
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    settings.writeToConfig(group);
}

DMetadataSettingsContainer DMetadataSettings::Private::setSettings(const DMetadataSettingsContainer& s)
{
    QMutexLocker lock(&mutex);
    DMetadataSettingsContainer old;
    old      = settings;
    settings = s;

    return old;
}

// -----------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DMetadataSettingsCreator
{
public:

    DMetadataSettings object;
};

Q_GLOBAL_STATIC(DMetadataSettingsCreator, creator)

// -----------------------------------------------------------------------------------------------

DMetadataSettings::DMetadataSettings()
    : QObject(),
      d      (new Private)
{
    readFromConfig();
    qRegisterMetaType<DMetadataSettingsContainer>("DMetadataSettingsContainer");
}

DMetadataSettings::~DMetadataSettings()
{
    delete d;
}

DMetadataSettings* DMetadataSettings::instance()
{
    return &creator->object;
}

DMetadataSettingsContainer DMetadataSettings::settings() const
{
    QMutexLocker lock(&d->mutex);
    DMetadataSettingsContainer s(d->settings);

    return s;
}

void DMetadataSettings::setSettings(const DMetadataSettingsContainer& settings)
{
    DMetadataSettingsContainer old = d->setSettings(settings);

    Q_EMIT signalDMetadataSettingsChanged(settings, old);
    Q_EMIT signalSettingsChanged();

    d->writeToConfig();
}

void DMetadataSettings::readFromConfig()
{
    DMetadataSettingsContainer s   = d->readFromConfig();
    DMetadataSettingsContainer old = d->setSettings(s);

    Q_EMIT signalDMetadataSettingsChanged(s, old);
    Q_EMIT signalSettingsChanged();
}

} // namespace Digikam

#include "moc_dmetadatasettings.cpp"
