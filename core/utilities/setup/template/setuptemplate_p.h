/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-07-04
 * Description : metadata template setup page.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "setuptemplate.h"

// Qt includes

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QMessageBox>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "templatelist.h"
#include "templatepanel.h"
#include "altlangstredit.h"
#include "dtextedit.h"

namespace Digikam
{

class Q_DECL_HIDDEN SetupTemplate::Private
{
public:

    Private() = default;

public:

    QPushButton*   addButton    = nullptr;
    QPushButton*   delButton    = nullptr;
    QPushButton*   repButton    = nullptr;
    QCheckBox*     mergeCBox    = nullptr;

    DTextEdit*     titleEdit    = nullptr;

    TemplateList*  listView     = nullptr;

    TemplatePanel* tview        = nullptr;
};

} // namespace Digikam
