/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * SPDX-FileCopyrightText: 2008      by Adrian Schroeter <adrian at suse dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "dmetadata.h"
#include "digikam_export.h"
#include "lensfunfilter.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT LensFunCameraSelector : public QWidget
{
    Q_OBJECT

public:

    typedef QMap<QString, QString> Device;

public:

    explicit LensFunCameraSelector(QWidget* const parent = nullptr);
    ~LensFunCameraSelector()          override;

    void setEnabledUseMetadata(bool b);

    void setUseMetadata(bool b);
    bool useMetadata()          const;

    LensFunContainer settings();
    void             setSettings(const LensFunContainer& settings);

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void resetToDefault();

    /**
     * @brief Special mode used with BQM which processes multiple items at the same time.
     */
    void setPassiveMetadataUsage(bool b);

    void setMetadata(const MetaEngineData&);

    bool supportsCCA()          const;
    bool supportsVig()          const;
    bool supportsDistortion()   const;
    bool supportsGeometry()     const;

Q_SIGNALS:

    void signalLensSettingsChanged();

private Q_SLOTS:

    void slotUseMetadata(bool);
    void slotMakeSelected();
    void slotModelChanged();
    void slotModelSelected();
    void slotLensSelected();
    void slotFocalChanged();
    void slotApertureChanged();
    void slotDistanceChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
