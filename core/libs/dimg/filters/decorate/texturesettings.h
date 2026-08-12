/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-10
 * Description : Texture settings view.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include <QWidget>

// Local includes

#include "digikam_export.h"
#include "texturecontainer.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT TextureSettings : public QWidget
{
    Q_OBJECT

public:

    explicit TextureSettings(QWidget* const parent);
    ~TextureSettings() override;

    TextureContainer defaultSettings() const;
    void resetToDefault();

    TextureContainer settings()        const;
    void setSettings(const TextureContainer& settings);

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

Q_SIGNALS:

    void signalSettingsChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
