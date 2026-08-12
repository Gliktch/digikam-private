/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-17
 * Description : Visibility states controller for assignment and confirmation of names for faces
 *
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "itemvisibilitycontroller.h"

namespace Digikam
{

class FaceItem;

class AssignNameWidgetStates : public HidingStateChanger
{
    Q_OBJECT

public:

    explicit AssignNameWidgetStates(FaceItem* const item);
    ~AssignNameWidgetStates() override = default;

protected Q_SLOTS:

    void slotStateChanged();

private:

    explicit AssignNameWidgetStates(QObject*);
};

} // namespace Digikam
