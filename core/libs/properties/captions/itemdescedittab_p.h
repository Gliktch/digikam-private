/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-03-09
 * Description : Captions, Tags, and Rating properties editor - private container
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "itemdescedittab.h"

// Qt includes

#include <QStyle>
#include <QGridLayout>
#include <QScrollArea>
#include <QTimer>
#include <QToolButton>
#include <QApplication>
#include <QPushButton>
#include <QMenu>
#include <QIcon>
#include <QCheckBox>
#include <QMessageBox>
#include <QPointer>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "addtagslineedit.h"
#include "applicationsettings.h"
#include "albumthumbnailloader.h"
#include "captionedit.h"
#include "collectionscanner.h"
#include "coredbtransaction.h"
#include "dnotificationwrapper.h"
#include "ddatetimeedit.h"
#include "digikamapp.h"
#include "fileactionmngr.h"
#include "ratingwidget.h"
#include "scancontroller.h"
#include "tagcheckview.h"
#include "templateselector.h"
#include "templateviewer.h"
#include "itemattributeswatch.h"
#include "statusprogressbar.h"
#include "tagmodificationhelper.h"
#include "template.h"
#include "iteminfolist.h"
#include "iteminfo.h"
#include "colorlabelwidget.h"
#include "picklabelwidget.h"
#include "fileactionprogress.h"
#include "tagsmanager.h"
#include "tagtreeview.h"
#include "searchtextbardb.h"
#include "disjointmetadata.h"
#include "altlangstredit.h"
#include "albummodel.h"
#include "metadatahub.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemDescEditTab::Private
{

public:

    explicit Private(ItemDescEditTab* const tab);

public:

    bool                 modified                   = false;
    bool                 ignoreItemAttributesWatch  = false;

    QToolButton*         revertBtn                  = nullptr;

    QMenu*               moreMenu                   = nullptr;

    QPushButton*         applyBtn                   = nullptr;
    QPushButton*         moreButton                 = nullptr;
    QPushButton*         applyToAllVersionsButton   = nullptr;

    QWidget*             lastSelectedWidget         = nullptr;

    AltLangStrEdit*      titleEdit                  = nullptr;

    CaptionEdit*         captionsEdit               = nullptr;

    DDateTimeEdit*       dateTimeEdit               = nullptr;

    QTabWidget*          tabWidget                  = nullptr;

    ItemInfoList         currInfos;

    TemplateSelector*    templateSelector           = nullptr;
    TemplateViewer*      templateViewer             = nullptr;

    RatingWidget*        ratingWidget               = nullptr;
    ColorLabelSelector*  colorLabelSelector         = nullptr;
    PickLabelSelector*   pickLabelSelector          = nullptr;

    DisjointMetadata*    hub                        = nullptr;

    QTimer*              metadataChangeTimer        = nullptr;
    QList<int>           metadataChangeIds;

    int                  spacing                    = 0;

    // ---

    bool                 togglingTagsSearchSettings = false;
    bool                 ignoreTagChanges           = false;
    QToolButton*         recentTagsBtn              = nullptr;
    QToolButton*         assignedTagsBtn            = nullptr;
    QPushButton*         openTagMngr                = nullptr;
    SearchTextBarDb*     tagsSearchBar              = nullptr;
    AddTagsLineEdit*     newTagEdit                 = nullptr;
    TagCheckView*        tagCheckView               = nullptr;
    TagModel*            tagModel                   = nullptr;

public:

    void setupConnections();
    void setupEventFilters();
    void initProgressIndicator();

    void reset();
    void resetMetadataChangeInfo();

    void setInfos(const ItemInfoList& infos);
    void setFocusToLastSelectedWidget();
    void setMetadataWidgetStatus(int status, QWidget* const widget);
    void metadataChange(qlonglong imageId);
    bool singleSelection() const;

private:

    ItemDescEditTab* const q = nullptr;
};

} // namespace Digikam
