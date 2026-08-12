/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-07-03
 * Description : Tag Manager main class
 *
 * SPDX-FileCopyrightText:      2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText:      2014 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "tagsmanager.h"

// C++ includes

#include <iterator>

// Qt includes

#include <QQueue>
#include <QTreeView>
#include <QLabel>
#include <QWidgetAction>
#include <QHBoxLayout>
#include <QSplitter>
#include <QApplication>
#include <QStandardPaths>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QScreen>
#include <QWindow>
#include <QAction>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QMenu>
#include <QIcon>

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#   include <QTextCodec>
#endif

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_globals.h"
#include "dmessagebox.h"
#include "dxmlguiwindow.h"
#include "tagpropwidget.h"
#include "tagmngrtreeview.h"
#include "taglist.h"
#include "tagfolderview.h"
#include "ddragobjects.h"
#include "searchtextbardb.h"
#include "tageditdlg.h"
#include "coredb.h"
#include "facetags.h"
#include "dlogoaction.h"
#include "metadatasynchronizer.h"
#include "fileactionmngr.h"
#include "metaenginesettings.h"
#include "dfiledialog.h"
#include "tagscache.h"

#ifdef HAVE_AKONADICONTACT
#   include "akonadiiface.h"
#endif

namespace Digikam
{

class Q_DECL_HIDDEN TagsManager::Private
{
public:

    Private(TagsManager* const parent);

    QString joinTagNamesToList(const QStringList& stringList);

    /**
     * @brief enable or disable options when only root tag is selected.
     */
    void enableRootTagActions(bool value);

    void setHelpText(QAction* const action, const QString& text);

    void setupActions();

public:

    TagsManager*     dd                 = nullptr;
    TagMngrTreeView* tagMngrView        = nullptr;
    QLabel*          tagPixmap          = nullptr;
    SearchTextBarDb* searchBar          = nullptr;

    QSplitter*       splitter           = nullptr;
    QWidget*         treeWindow         = nullptr;
    QToolBar*        mainToolbar        = nullptr;
    QPushButton*     organizeButton     = nullptr;
    QPushButton*     syncexportButton   = nullptr;
    QAction*         tagProperties      = nullptr;
    QAction*         addAction          = nullptr;
    QAction*         delAction          = nullptr;
    QAction*         titleEdit          = nullptr;

    /**
     * @note Options unavailable for root tag
     */
    QList<QAction*>  rootDisabledOptions;

    TagList*         listView           = nullptr;
    TagPropWidget*   tagPropWidget      = nullptr;
    TagModel*        tagModel           = nullptr;

    bool             tagPropVisible     = false;
    bool             firstShowEvent     = true;
};

} // namespace Digikam
