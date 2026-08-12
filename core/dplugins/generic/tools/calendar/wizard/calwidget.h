/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : calendar widget used to preview the active template.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

class QPaintEvent;

namespace DigikamGenericCalendarPlugin
{

class CalWidget : public QWidget
{
    Q_OBJECT

public:

    explicit CalWidget(QWidget* const parent);
    ~CalWidget()                          override = default;

    void setCurrent(int curr);
    int current()                   const;

public Q_SLOTS:

    void recreate();

protected:

    void paintEvent(QPaintEvent* e) override;

protected:

    int m_current = 1;
};

} // Namespace Digikam
