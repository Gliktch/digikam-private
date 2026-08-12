/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-21
 * Description : widget to display a list of items
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2010 by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Luka Renko <lure at kubuntu dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "ditemslist.h"

// Qt includes

#include <QDragEnterEvent>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QMimeData>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QUrl>
#include <QTimer>
#include <QFile>
#include <QPointer>
#include <QXmlStreamAttributes>
#include <QString>
#include <QStandardPaths>
#include <QIcon>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "drawdecoder.h"
#include "imagedialog.h"
#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "dfiledialog.h"
#include "thumbnailloadthread.h"
#include "dworkingpixmap.h"

namespace Digikam
{

class Q_DECL_HIDDEN CtrlButton : public QPushButton
{
    Q_OBJECT

public:

    explicit CtrlButton(const QIcon& icon, QWidget* const parent = nullptr)
       : QPushButton(parent)
    {
        const int btnSize = 32;

        setMinimumSize(btnSize, btnSize);
        setMaximumSize(btnSize, btnSize);
        setIcon(icon);
    }

    ~CtrlButton() override = default;
};

} // namespace Digikam
