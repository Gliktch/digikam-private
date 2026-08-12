/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-08-20
 * Description : black frame list view tool tip
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "ditemtooltip.h"
#include "digikam_export.h"

class QTreeWidgetItem;
class QTreeWidget;

namespace Digikam
{

class DIGIKAM_EXPORT BlackFrameToolTip : public DItemToolTip
{
    Q_OBJECT

public:

    explicit BlackFrameToolTip(QTreeWidget* const view);
    ~BlackFrameToolTip()        override;

    void setToolTipString(const QString& tip);
    void setItem(QTreeWidgetItem* const item);

    void show();

protected:

    QRect   repositionRect()    override;
    QString tipContents()       override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
