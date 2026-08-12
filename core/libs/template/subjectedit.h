/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-09
 * Description : Subjects panel.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "subjectwidget.h"

namespace Digikam
{

class SubjectEdit : public SubjectWidget
{
    Q_OBJECT

public:

    explicit SubjectEdit(QWidget* const parent);
    ~SubjectEdit()        override;

private Q_SLOTS:

    void slotRefChanged() override;
};

} // namespace Digikam
