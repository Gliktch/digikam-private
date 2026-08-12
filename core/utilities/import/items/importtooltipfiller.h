/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-28-07
 * Description : Import icon view tool tip
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

namespace Digikam
{

class FilterAction;
class CamItemInfo;

namespace ImportToolTipFiller
{
    QString CamItemInfoTipContents(const CamItemInfo& info);

} // namespace ImportToolTipFiller

} // namespace Digikam
