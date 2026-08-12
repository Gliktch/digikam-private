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

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"
#include "dmetadatasettingscontainer.h"

namespace Digikam
{

class DIGIKAM_EXPORT DMetadataSettings : public QObject
{
    Q_OBJECT

public:

    DMetadataSettings();
    ~DMetadataSettings() override;

    /**
     * Global container for Metadata settings. All accessor methods are thread-safe.
     */
    static DMetadataSettings* instance();

    /**
     * Returns the current Metadata settings.
     */
    DMetadataSettingsContainer settings() const;

    /**
     * Sets the current Metadata settings and writes them to config.
     */
    void setSettings(const DMetadataSettingsContainer& settings);

Q_SIGNALS:

    void signalSettingsChanged();
    void signalDMetadataSettingsChanged(const DMetadataSettingsContainer& current,
                                        const DMetadataSettingsContainer& previous);

private:

    explicit DMetadataSettings(QObject*);

    void readFromConfig();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
