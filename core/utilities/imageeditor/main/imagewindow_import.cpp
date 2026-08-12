/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : digiKam image editor - Import tools
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imagewindow_p.h"

namespace Digikam
{

void ImageWindow::slotImportedImagefromScanner(const QUrl& url)
{
    ItemInfo info = ScanController::instance()->scannedInfo(url.toLocalFile());
    openImage(info);
}

} // namespace Digikam
