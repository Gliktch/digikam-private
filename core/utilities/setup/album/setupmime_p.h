/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-05-03
 * Description : mime types setup tab
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "setupmime.h"

// Qt includes

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWhatsThis>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>
#include <QIcon>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "applicationsettings.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "dlayoutbox.h"
#include "scancontroller.h"
#include "setup.h"

namespace Digikam
{

class Q_DECL_HIDDEN SetupMime::Private
{
public:

    Private() = default;

public:

    QWidget*   panel                = nullptr;

    QLabel*    imageFileFilterLabel = nullptr;
    QLabel*    movieFileFilterLabel = nullptr;
    QLabel*    audioFileFilterLabel = nullptr;

    QLineEdit* imageFileFilterEdit  = nullptr;
    QLineEdit* movieFileFilterEdit  = nullptr;
    QLineEdit* audioFileFilterEdit  = nullptr;
};

} // namespace Digikam
