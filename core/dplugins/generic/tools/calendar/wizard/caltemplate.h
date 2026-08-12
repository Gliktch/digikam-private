/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : template selection for calendar.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Angelo Naselli <anaselli at linux dot it>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QList>
#include <QUrl>

// Local include

#include "ui_caltemplate.h"

namespace DigikamGenericCalendarPlugin
{

class CalTemplate : public QWidget
{
    Q_OBJECT

public:

    explicit CalTemplate(const QList<QUrl>& urlList, QWidget* const parent);
    ~CalTemplate() override;

private Q_SLOTS:

    void yearChanged(int year);
    void monthChanged(int);

private:

    class Private;
    Private* const d = nullptr;
};

} // Namespace Digikam
