/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Fuzzy search sidebar tab contents - private container.
 *
 * SPDX-FileCopyrightText: 2016-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "fuzzysearchview.h"

// Qt includes

#include <QFrame>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QTime>
#include <QTimer>
#include <QToolButton>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>
#include <QIcon>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "album.h"
#include "coredb.h"
#include "coredbalbuminfo.h"
#include "albummanager.h"
#include "albummodel.h"
#include "albumselectors.h"
#include "searchtreeview.h"
#include "coredbaccess.h"
#include "ddragobjects.h"
#include "editablesearchtreeview.h"
#include "findduplicatesview.h"
#include "haariface.h"
#include "iteminfo.h"
#include "itemlister.h"
#include "searchmodificationhelper.h"
#include "searchtextbardb.h"
#include "coredbsearchxml.h"
#include "sketchwidget.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "dhuesaturationselect.h"
#include "dcolorvalueselector.h"
#include "dexpanderbox.h"
#include "applicationsettings.h"
#include "drangebox.h"
#include "similaritydbaccess.h"
#include "similaritydb.h"
#include "dnotificationwidget.h"
#include "dtextedit.h"

namespace Digikam
{

class Q_DECL_HIDDEN FuzzySearchView::Private
{

public:

    enum FuzzySearchTab
    {
        DUPLICATES = 0,
        SIMILARS,
        SKETCH
    };

public:

    Private() = default;

public:

    const QString             configTabEntry                    = QLatin1String("FuzzySearch Tab");
    const QString             configPenSketchSizeEntry          = QLatin1String("Pen Sketch Size");
    const QString             configResultSketchItemsEntry      = QLatin1String("Result Sketch items");
    const QString             configPenSketchHueEntry           = QLatin1String("Pen Sketch Hue");
    const QString             configPenSketchSaturationEntry    = QLatin1String("Pen Sketch Saturation");
    const QString             configPenSkethValueEntry          = QLatin1String("Pen Sketch Value");
    const QString             configSimilarsThresholdEntry      = QLatin1String("Similars Threshold");
    const QString             configSimilarsMaxThresholdEntry   = QLatin1String("Similars Maximum Threshold");

    bool                      active                            = false;  ///< @note Initially be active to update sketch panel when the search list is restored.
    bool                      fingerprintsChecked               = false;

    QColor                    selColor;

    QToolButton*              resetButton                       = nullptr;
    QToolButton*              saveBtnSketch                     = nullptr;
    QToolButton*              undoBtnSketch                     = nullptr;
    QToolButton*              redoBtnSketch                     = nullptr;
    QToolButton*              saveBtnImage                      = nullptr;

    QSpinBox*                 penSize                           = nullptr;
    QSpinBox*                 resultsSketch                     = nullptr;

    DIntRangeBox*             similarityRange                   = nullptr;

    QLabel*                   imageWidget                       = nullptr;

    QTimer*                   timerSketch                       = nullptr;
    QTimer*                   timerImage                        = nullptr;

    DVBox*                    folderView                        = nullptr;

    DTextEdit*                nameEditSketch                    = nullptr;
    DTextEdit*                nameEditImage                     = nullptr;

    QTabWidget*               tabWidget                         = nullptr;

    DHueSaturationSelector*   hsSelector                        = nullptr;

    DColorValueSelector*      vSelector                         = nullptr;

    DAdjustableLabel*         labelFile                         = nullptr;
    DAdjustableLabel*         labelFolder                       = nullptr;

    ItemInfo                  imageInfo;
    QUrl                      imageUrl;

    SearchTextBarDb*          searchFuzzyBar                    = nullptr;

    EditableSearchTreeView*   searchTreeView                    = nullptr;

    SketchWidget*             sketchWidget                      = nullptr;

    ThumbnailLoadThread*      thumbLoadThread                   = nullptr;

    FindDuplicatesView*       findDuplicatesPanel               = nullptr;

    AlbumPointer<SAlbum>      imageSAlbum;
    AlbumPointer<SAlbum>      sketchSAlbum;

    AlbumSelectors*           fuzzySearchAlbumSelectors         = nullptr;
    AlbumSelectors*           sketchSearchAlbumSelectors        = nullptr;

    SearchModel*              searchModel                       = nullptr;
    SearchModificationHelper* searchModificationHelper          = nullptr;

    ApplicationSettings*      settings                          = nullptr;
};

} // namespace Digikam
