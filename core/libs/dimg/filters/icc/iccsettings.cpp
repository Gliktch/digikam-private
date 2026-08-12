/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : central place for ICC settings - Common implementations.
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

class Q_DECL_HIDDEN IccSettingsCreator
{
public:

    IccSettings object;
};

Q_GLOBAL_STATIC(IccSettingsCreator, creator)

// -----------------------------------------------------------------------------------------------

IccSettings::IccSettings()
    : QObject(),
      d      (new Private)
{
    IccTransform::init();
    readFromConfig();
    qRegisterMetaType<ICCSettingsContainer>("ICCSettingsContainer");
}

IccSettings::~IccSettings()
{
    delete d;
}

IccSettings* IccSettings::instance()
{
    return &creator->object;
}

ICCSettingsContainer IccSettings::settings()
{
    QMutexLocker lock(&d->mutex);
    ICCSettingsContainer s(d->settings);

    return s;
}

IccProfile IccSettings::monitorProfile(QWidget* const widget)
{
    // system-wide profile set?

    IccProfile profile = d->profileFromDesktop(widget);

    if (!profile.isNull())
    {
        return profile;
    }

    QMutexLocker lock(&d->mutex);

    if (!d->settings.monitorProfile.isNull())
    {
        return IccProfile(d->settings.monitorProfile);
    }
    else
    {
        return IccProfile::sRGB();
    }
}

bool IccSettings::monitorProfileFromSystem() const
{
    // First, look into cache

    {
        QMutexLocker lock(&d->mutex);

        for (const IccProfile& profile : std::as_const(d->screenProfiles))
        {
            if (!profile.isNull())
            {   // cppcheck-suppress useStlAlgorithm
                return true;
            }
        }
    }

    // Second, check all toplevel widgets

    QList<QWidget*> topLevels = qApp->topLevelWidgets();

    for (QWidget* const widget : std::as_const(topLevels))
    {
        if (!d->profileFromDesktop(widget).isNull())
        {
            return true;
        }
    }

    return false;
}

void IccSettings::loadAllProfilesProperties()
{
    allProfiles();
    const int size = d->profiles.size();

    for (int i = 0 ; i < size ; ++i)
    {
        IccProfile& profile = d->profiles[i];

        if (!profile.isOpen())
        {
            profile.description();
            profile.type();
            profile.close();
        }
        else
        {
            profile.description();
            profile.type();
        }
    }
}

QList<IccProfile> IccSettings::profilesForDescription(const QString& description)
{
    QList<IccProfile> profiles;

    if (description.isEmpty())
    {
        return profiles;
    }

    const auto pros = allProfiles();

    for (IccProfile profile : pros)
    {
        if (profile.description() == description)
        {
            profiles << profile;
        }
    }

    return profiles;
}

QList<IccProfile> IccSettings::outputProfiles()
{
    QList<IccProfile> profiles;

    const auto pros = allProfiles();

    for (IccProfile profile : pros)
    {
        if (profile.type() == IccProfile::Output)
        {
            profiles << profile;
        }
    }

    return profiles;
}

QList<IccProfile> IccSettings::inputProfiles()
{
    QList<IccProfile> profiles;
    const auto pros = allProfiles();

    for (IccProfile profile : pros)
    {
        switch (profile.type())
        {
            case IccProfile::Input:
            case IccProfile::ColorSpace:
            {
                profiles << profile;
                break;
            }

            default:
            {
                break;
            }
        }
    }

    return profiles;
}

QList<IccProfile> IccSettings::displayProfiles()
{
    QList<IccProfile> profiles;
    const auto pros = allProfiles();

    for (IccProfile profile : pros)
    {
        if (profile.type() == IccProfile::Display)
        {
            profiles << profile;
        }
    }

    return profiles;
}

QList<IccProfile> IccSettings::workspaceProfiles()
{
    QList<IccProfile> profiles;
    const auto pros = allProfiles();

    for (IccProfile profile : pros)
    {
        switch (profile.type())
        {
            case IccProfile::Display:
            case IccProfile::ColorSpace:
            {
                profiles << profile;
                break;
            }

            default:
            {
                break;
            }
        }
    }

    return profiles;
}

QList<IccProfile> IccSettings::allProfiles()
{
    QString extraPath;
    {
        QMutexLocker lock(&d->mutex);

        if (!d->profiles.isEmpty())
        {
            return d->profiles;
        }

        extraPath = d->settings.iccFolder;
    }

    QList<IccProfile> profiles;

    // get system paths, e.g. /usr/share/color/icc

    QStringList paths = IccProfile::defaultSearchPaths();

    // add user-specified path

    if (!extraPath.isEmpty() && !paths.contains(extraPath))
    {
        paths << extraPath;
    }

    // check search directories

    profiles << d->scanDirectories(paths);

    // load profiles that come with RawEngine

    profiles << IccProfile::defaultProfiles();

    QMutexLocker lock(&d->mutex);
    d->profiles = profiles;

    return d->profiles;
}

void IccSettings::setIccPath(const QString& path)
{
    ICCSettingsContainer old, current;

    {
        QMutexLocker lock(&d->mutex);

        if (path == d->settings.iccFolder)
        {
            return;
        }

        d->profiles.clear();
        old                   = d->settings;
        d->settings.iccFolder = path;
        current               = d->settings;
    }

    d->writeManagedViewToConfig();

    Q_EMIT signalSettingsChanged();
    Q_EMIT signalICCSettingsChanged(current, old);
}

bool IccSettings::isEnabled() const
{
    return d->settings.enableCM;
}

bool IccSettings::useManagedPreviews() const
{
    return (isEnabled() && d->settings.useManagedPreviews);
}

void IccSettings::readFromConfig()
{
    ICCSettingsContainer old, s;
    s = d->readFromConfig();

    {
        QMutexLocker lock(&d->mutex);
        old         = d->settings;
        d->settings = s;
    }

    Q_EMIT signalSettingsChanged();
    Q_EMIT signalICCSettingsChanged(s, old);
}

void IccSettings::setSettings(const ICCSettingsContainer& settings)
{
    ICCSettingsContainer old;

    {
        QMutexLocker lock(&d->mutex);

        if (settings.iccFolder != d->settings.iccFolder)
        {
            d->profiles.clear();
        }

        old         = d->settings;
        d->settings = settings;
    }

    d->writeToConfig();

    Q_EMIT signalSettingsChanged();
    Q_EMIT signalICCSettingsChanged(settings, old);
}

void IccSettings::setUseManagedView(bool useManagedView)
{
    ICCSettingsContainer old, current;

    {
        QMutexLocker lock(&d->mutex);
        old                        = d->settings;
        d->settings.useManagedView = useManagedView;
        current                    = d->settings;
    }

    d->writeManagedViewToConfig();

    Q_EMIT signalSettingsChanged();
    Q_EMIT signalICCSettingsChanged(current, old);
}

void IccSettings::setUseManagedPreviews(bool useManagedPreviews)
{
    ICCSettingsContainer old, current;

    {
        QMutexLocker lock(&d->mutex);
        old                            = d->settings;
        d->settings.useManagedPreviews = useManagedPreviews;
        current                        = d->settings;
    }

    d->writeManagedPreviewsToConfig();

    Q_EMIT signalSettingsChanged();
    Q_EMIT signalICCSettingsChanged(current, old);
}

} // namespace Digikam

#include "moc_iccsettings.cpp"
