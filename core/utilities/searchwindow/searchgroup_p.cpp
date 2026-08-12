/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "searchgroup_p.h"

namespace Digikam
{

RadioButtonHBox::RadioButtonHBox(QWidget* const left, QWidget* const right, Qt::LayoutDirection dir)
    : QHBoxLayout()
{
    if (dir == Qt::RightToLeft)
    {
        addWidget(right, Qt::AlignRight);
        addWidget(left);
    }
    else
    {
        addWidget(left);
        addWidget(right, Qt::AlignLeft);
    }

    setSpacing(0);
}

} // namespace Digikam

#include "moc_searchgroup_p.cpp"
