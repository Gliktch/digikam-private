/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-12
 * Description : a widget to display ICC profiles descriptions
 *               in file dialog preview.
 *
 * SPDX-FileCopyrightText: 2006-2007 by Francisco J. Cruz <fj dot cruz at supercable dot es>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iccpreviewwidget.h"

// Qt includes

#include <QFileInfo>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "iccprofilewidget.h"

namespace Digikam
{

ICCPreviewWidget::ICCPreviewWidget(QWidget* const parent)
    : QScrollArea(parent)
{
    m_iccProfileWidget = new ICCProfileWidget(this);
    setWidget(m_iccProfileWidget);
    setWidgetResizable(true);
}

void ICCPreviewWidget::slotShowPreview(const QUrl& url)
{
    slotClearPreview();
    QFileInfo fInfo(url.toLocalFile());

    if (url.isLocalFile() && fInfo.isFile() && fInfo.isReadable())
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << url << " is a readable local file";
        m_iccProfileWidget->loadFromURL(url);
    }
    else
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << url << " is not a readable local file";
    }
}

void ICCPreviewWidget::slotClearPreview()
{
    m_iccProfileWidget->loadFromURL(QUrl());
}

} // namespace Digikam

#include "moc_iccpreviewwidget.cpp"
