/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : album icon view tool tip
 *
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
class ItemInfo;
class PAlbum;

namespace ToolTipFiller
{

    QString imageInfoTipContents(const ItemInfo& info);
    QString albumTipContents(PAlbum* const album, int count);
    QString filterActionTipContents(const FilterAction& action);

} // namespace ToolTipFiller

} // namespace Digikam
