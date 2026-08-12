/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-29
 * Description : extension to IccManager providing UI
 *
 * SPDX-FileCopyrightText: 2005-2006 by F.J. Cruz <fj dot cruz at supercable dot es>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "iccmanager.h"

namespace Digikam
{

class DIGIKAM_EXPORT IccPostLoadingManager : public IccManager
{

public:

    /**
     * Constructs an IccPostLoadingManager object.
     * The DImg will be edited. The filePath is for display only.
     */
    explicit IccPostLoadingManager(DImg& image, const QString& filePath = QString(),
                                   const ICCSettingsContainer& settings = IccSettings::instance()->settings());

    /**
     * Carries out color management asking the user for his decision.
     * Afterwards, needsPostLoadingManagement will return false.
     */
    IccTransform postLoadingManage(QWidget* const parent = nullptr);

protected:

    QString m_filePath;
};

} // namespace Digikam
