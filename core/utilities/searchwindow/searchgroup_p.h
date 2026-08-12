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

#pragma once

#include "searchgroup.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QRadioButton>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "searchfieldgroup.h"
#include "searchfields.h"
#include "searchutilities.h"
#include "searchview.h"

namespace Digikam
{

class Q_DECL_HIDDEN RadioButtonHBox : public QHBoxLayout
{
    Q_OBJECT

public:

    RadioButtonHBox(QWidget* const left, QWidget* const right, Qt::LayoutDirection dir);
};

} // namespace Digikam
