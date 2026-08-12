/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : central place for ICC settings
 *
 * SPDX-FileCopyrightText: 2005-2006 by F.J. Cruz <fj dot cruz at supercable dot es>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"
#include "iccsettingscontainer.h"

namespace Digikam
{

class IccProfile;

class DIGIKAM_EXPORT IccSettings : public QObject
{
    Q_OBJECT

public:

    IccSettings();
    ~IccSettings() override;

    /**
     * Global container for ICC settings. All accessor methods are thread-safe.
     */
    static IccSettings* instance();

    /**
     * Returns the current ICC settings.
     */
    ICCSettingsContainer settings();

    /**
     * Returns if color management is enabled.
     */
    bool isEnabled()                const;

    /**
     * Returns if color management for previews is enabled.
     */
    bool useManagedPreviews()       const;

    /**
     * Returns the monitor profile (for color managed view).
     * If there are multiple screens, a system-wide settings specifies the monitor profile,
     * and the widget parameter is passed, the returned profile is for the widget's screen.
     * If no settings is specified, the default sRGB profile is returned.
     */
    IccProfile monitorProfile(QWidget* const widget = nullptr);

    /**
     * Returns if the monitor profile (as returned by monitorProfile())
     * is set system-wide, so that the monitorProfile field of the current settings
     * need not be set and will not be used by monitorProfile().
     */
    bool monitorProfileFromSystem() const;

    /**
     * Sets the current ICC settings and writes them to config.
     */
    void setSettings(const ICCSettingsContainer& settings);

    /**
     * Set single parts of the settings
     */
    void setUseManagedView(bool useManagedView);
    void setUseManagedPreviews(bool useManagedPreviews);
    void setIccPath(const QString& path);

    QList<IccProfile> allProfiles();

    /**
     * Get available profiles suitable as workspace profile
     */
    QList<IccProfile> workspaceProfiles();

    /**
     * Get available profiles suitable as monitor/display profile
     */
    QList<IccProfile> displayProfiles();

    /**
     * Get available profiles suitable as input profile
     */
    QList<IccProfile> inputProfiles();

    /**
     * Get available profiles suitable as proof/output profiles
     */
    QList<IccProfile> outputProfiles();

    /**
     * Returns a list of profiles with the given description()
     */
    QList<IccProfile> profilesForDescription(const QString& description);

    /**
     * IccProfile caches some of its properties (description, type)
     * when it was read once. Subsequently, to read these values no
     * opening is needed. This ensures that all profiles have these
     * values read. May imply scanning and opening all profiles.
     */
    void loadAllProfilesProperties();

Q_SIGNALS:

    void signalSettingsChanged();
    void signalICCSettingsChanged(const ICCSettingsContainer& current, const ICCSettingsContainer& previous);

private:

    explicit IccSettings(QObject*);

    void readFromConfig();

private:

    class Private;
    Private* const d = nullptr;

    friend class Private;
};

} // namespace Digikam
