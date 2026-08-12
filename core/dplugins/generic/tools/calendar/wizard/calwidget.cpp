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
 * SPDX-FileCopyrightText: 2012      by Angelo Naselli <anaselli at linux dot it>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "calwidget.h"

// Local includes

#include "calpainter.h"
#include "calsettings.h"
#include "digikam_debug.h"

namespace DigikamGenericCalendarPlugin
{

CalWidget::CalWidget(QWidget* const parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void CalWidget::setCurrent(int curr)
{
    m_current = curr;
}

int CalWidget::current() const
{
    return m_current;
}

void CalWidget::paintEvent(QPaintEvent* e)
{
    int month       = m_current;
    CalPainter painter(this);

    QUrl imgUrl     = CalSettings::instance()->image(month);
    painter.setImage(imgUrl);
    painter.paint(month);

    Q_UNUSED(e);
}

void CalWidget::recreate()
{
    CalParams& params = CalSettings::instance()->params;

    setFixedSize(params.width, params.height);
    update();
}

} // Namespace Digikam

#include "moc_calwidget.cpp"
