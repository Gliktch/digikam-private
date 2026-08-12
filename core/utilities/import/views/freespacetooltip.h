/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-01-19
 * Description : free space widget tool tip
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "ditemtooltip.h"

namespace Digikam
{

class FreeSpaceToolTip : public DItemToolTip
{
    Q_OBJECT

public:

    explicit FreeSpaceToolTip(QWidget* const parent);
    ~FreeSpaceToolTip()         override;

    void setToolTip(const QString& tip);

    void show();

protected:

    QRect   repositionRect()    override;
    QString tipContents()       override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
