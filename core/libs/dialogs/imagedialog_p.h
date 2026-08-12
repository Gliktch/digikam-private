/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-13
 * Description : Image files selection dialog - private declarations.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "imagedialog.h"

// Qt includes

#include <QLabel>
#include <QPointer>
#include <QVBoxLayout>
#include <QApplication>
#include <QMimeDatabase>
#include <QStyle>
#include <QLocale>
#include <QPixmap>
#include <QPainter>
#include <QScopedPointer>
#include <QMutexLocker>
#include <QToolTip>
#include <QDir>
#include <QTimer>
#include <QWheelEvent>
#include <QKeyEvent>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "drawdecoder.h"
#include "digikam_debug.h"
#include "digikam_globals.h"
#include "thumbnailloadthread.h"
#include "ditemtooltip.h"
#include "dmetadata.h"
#include "loadingdescription.h"
#include "thumbnailsize.h"
#include "dfiledialog.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImageDialog::Private
{

public:

    Private() = default;

public:

    QStringList               fileFormats;
    QList<QUrl>               urls;
    DFileDialog*              dlg           = nullptr;
    ImageDialogIconProvider*  provider      = nullptr;
    bool                      showToolTips  = true;
    QTimer*                   toolTipTimer  = nullptr;
    ImageDialogToolTip*       toolTip       = nullptr;
    QAbstractItemView*        toolTipView   = nullptr;
    QModelIndex               toolTipIndex;
    QUrl                      toolTipUrl;
};

} // namespace Digikam
