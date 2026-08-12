/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-08-16
 * Description : central place for Spell-check and localize settings
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "localizesettings.h"

// Qt includes

#include <QMutex>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN LocalizeSettings::Private
{
public:

    Private () = default;

    LocalizeContainer settings;
    QMutex            mutex;

    const QString     configGroup = QLatin1String("Spell Check Settings");

public:

    LocalizeContainer readFromConfig() const;
    void              writeToConfig()  const;
    void              setSettings(const LocalizeContainer& s, LocalizeSettings::ConfigPart config);
};

LocalizeContainer LocalizeSettings::Private::readFromConfig() const
{
    LocalizeContainer s;
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    s.readFromConfig(group);

    return s;
}

void LocalizeSettings::Private::writeToConfig() const
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    settings.writeToConfig(group);
}

void LocalizeSettings::Private::setSettings(const LocalizeContainer& s, LocalizeSettings::ConfigPart config)
{
    QMutexLocker lock(&mutex);

    switch (config)
    {
        case LocalizeConfig:
        {
            settings.translatorEngine = s.translatorEngine;
            settings.translatorLang   = s.translatorLang;
            settings.alternativeLang  = s.alternativeLang;
            break;
        }

        case SpellCheckConfig:
        {
            settings.enableSpellCheck = s.enableSpellCheck;
            settings.ignoredWords     = s.ignoredWords;
            break;
        }

        default:        // AllConfig
        {
            settings = s;
            break;
        }
    }
}

// -----------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN LocalizeSettingsCreator
{
public:

    LocalizeSettings object;
};

Q_GLOBAL_STATIC(LocalizeSettingsCreator, creator)

// -----------------------------------------------------------------------------------------------

LocalizeSettings::LocalizeSettings()
    : QObject(),
      d      (new Private)
{
    readFromConfig();
    qRegisterMetaType<LocalizeContainer>("LocalizeContainer");
}

LocalizeSettings::~LocalizeSettings()
{
    delete d;
}

LocalizeSettings* LocalizeSettings::instance()
{
    return &creator->object;
}

void LocalizeSettings::openLocalizeSetup()
{
    Q_EMIT signalOpenLocalizeSetup();
}

LocalizeContainer LocalizeSettings::settings() const
{
    QMutexLocker lock(&d->mutex);
    LocalizeContainer s(d->settings);

    return s;
}

void LocalizeSettings::setSettings(const LocalizeContainer& settings, ConfigPart config)
{
    d->setSettings(settings, config);

    Q_EMIT signalSettingsChanged();

    d->writeToConfig();
}

void LocalizeSettings::readFromConfig()
{
    d->settings = d->readFromConfig();

    Q_EMIT signalSettingsChanged();
}

} // namespace Digikam

#include "moc_localizesettings.cpp"
