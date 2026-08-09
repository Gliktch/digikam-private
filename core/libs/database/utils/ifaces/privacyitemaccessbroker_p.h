/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dinfointerface.h"

namespace Digikam
{

QSharedPointer<DItemAccessHandle> preparePrivacyItemAccess(
    const DItemAccessRequest& request);

} // namespace Digikam
