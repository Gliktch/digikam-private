/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-01-24
 * Description : Tags Action Manager - Private internal container
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "tagsactionmngr.h"

// Qt includes

#include <QList>
#include <QShortcut>
#include <QIcon>
#include <QKeySequence>
#include <QApplication>
#include <QAction>

// KDE includes

#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Restore warnings
#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "album.h"
#include "coredb.h"
#include "albummanager.h"
#include "coredbaccess.h"
#include "coredbconstants.h"
#include "coredbwatch.h"
#include "coredbinfocontainers.h"
#include "digikamapp.h"
#include "dxmlguiwindow.h"
#include "itemiconview.h"
#include "imagewindow.h"
#include "surveywindow.h"
#include "lighttablewindow.h"
#include "picklabelwidget.h"
#include "colorlabelwidget.h"
#include "tagscache.h"
#include "tagproperties.h"
#include "ratingwidget.h"
#include "syncjob.h"

namespace Digikam
{

class Q_DECL_HIDDEN TagsActionMngr::Private
{
public:

    Private() = default;

public:

    QMultiMap<int, QAction*>  tagsActionMap;
    QList<KActionCollection*> actionCollectionList;
    QMap<int, QString>        colorLabelNames;

    const QString             ratingShortcutPrefix   = QLatin1String("rateshortcut");
    const QString             tagShortcutPrefix      = QLatin1String("tagshortcut");
    const QString             pickShortcutPrefix     = QLatin1String("pickshortcut");
    const QString             colorShortcutPrefix    = QLatin1String("colorshortcut");
    const QString             noToggleShortcutPrefix = QLatin1String("notoggle");

    const QString             configColorNameEntry   = QLatin1String("ColorName_");
    const QString             configColorNamesGroup  = QLatin1String("Color Names");
};


} // namespace Digikam
