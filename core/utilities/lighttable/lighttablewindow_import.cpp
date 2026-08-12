/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : digiKam light table - Import tools
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lighttablewindow_p.h"

namespace Digikam
{

void LightTableWindow::slotImportedImagefromScanner(const QUrl& url)
{
    ItemInfo info = ScanController::instance()->scannedInfo(url.toLocalFile());
    loadItemInfos(ItemInfoList(QList<ItemInfo>() << info), info, true);
}

} // namespace Digikam
