/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-12-08
 * Description : ICC Settings Container.
 *
 * SPDX-FileCopyrightText: 2005-2007 by F.J. Cruz <fj dot cruz at supercable dot es>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QColor>
#include <QFlags>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "icctransform.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT ICCSettingsContainer
{

public:

    enum BehaviorEnum
    {
        /// Note: Values are stored in config - keep them constant

        InvalidBehavior          = 0,

        /// Interpretation of the image data

        UseEmbeddedProfile       = 1 << 0,
        UseSRGB                  = 1 << 1,
        UseWorkspace             = 1 << 2,
        UseDefaultInputProfile   = 1 << 3,
        UseSpecifiedProfile      = 1 << 4,
        AutomaticColors          = 1 << 5,
        DoNotInterpret           = 1 << 6,

        /// Transformation / target profile

        KeepProfile              = 1 << 10,
        ConvertToWorkspace       = 1 << 11,

        /// Special flags and values

        LeaveFileUntagged        = 1 << 18,

        AskUser                  = 1 << 20,
        SafestBestAction         = 1 << 21,

        /// ready combinations for convenience

        PreserveEmbeddedProfile  = UseEmbeddedProfile     | KeepProfile,
        EmbeddedToWorkspace      = UseEmbeddedProfile     | ConvertToWorkspace,
        SRGBToWorkspace          = UseSRGB                | ConvertToWorkspace,
        AutoToWorkspace          = AutomaticColors        | ConvertToWorkspace,
        InputToWorkspace         = UseDefaultInputProfile | ConvertToWorkspace,
        SpecifiedToWorkspace     = UseSpecifiedProfile    | ConvertToWorkspace,
        NoColorManagement        = DoNotInterpret         | LeaveFileUntagged
    };
    Q_DECLARE_FLAGS(Behavior, BehaviorEnum)

public:

    ICCSettingsContainer()  = default;
    ~ICCSettingsContainer() = default;

    void readFromConfig(KConfigGroup& group);
    void writeToConfig(KConfigGroup& group)                 const;
    void writeManagedViewToConfig(KConfigGroup& group)      const;
    void writeManagedPreviewsToConfig(KConfigGroup& group)  const;

public:

    bool     enableCM                       = true;

    QString  iccFolder;

    QString  workspaceProfile;

    Behavior defaultMismatchBehavior        = EmbeddedToWorkspace;
    Behavior defaultMissingProfileBehavior  = SRGBToWorkspace;
    Behavior defaultUncalibratedBehavior    = AutoToWorkspace;

    Behavior lastMismatchBehavior           = EmbeddedToWorkspace;
    Behavior lastMissingProfileBehavior     = SRGBToWorkspace;
    Behavior lastUncalibratedBehavior       = AutoToWorkspace;
    QString  lastSpecifiedAssignProfile;
    QString  lastSpecifiedInputProfile;

    bool     useManagedView                 = true;
    bool     useManagedPreviews             = true;
    QString  monitorProfile;

    QString  defaultInputProfile;
    QString  defaultProofProfile;

    bool     useBPC                         = true;
    int      renderingIntent                = IccTransform::Perceptual;

    /// Settings specific for soft proofing
    int      proofingRenderingIntent        = IccTransform::AbsoluteColorimetric;
    bool     doGamutCheck                   = false;
    QColor   gamutCheckMaskColor            = QColor(126, 255, 255);

private:

    QString getProfilePath(KConfigGroup& group, const char* key) const;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::ICCSettingsContainer::Behavior)
