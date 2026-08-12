/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-21
 * Description : MJPEG Stream Server configuration dialog - Private container
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Quoc Hưng Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QStyle>
#include <QIcon>
#include <QCheckBox>
#include <QGroupBox>
#include <QTabWidget>
#include <QDesktopServices>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "mjpegstreamdlg.h"
#include "dinfointerface.h"
#include "ditemslist.h"
#include "dnuminput.h"
#include "dlayoutbox.h"
#include "dxmlguiwindow.h"
#include "workingwidget.h"
#include "mjpegservermngr.h"
#include "vidslidesettings.h"
#include "transitionpreview.h"
#include "effectpreview.h"
#include "dcombobox.h"
#include "dfontselect.h"
#include "frameosdwidget.h"

namespace DigikamGenericMjpegStreamPlugin
{

class Q_DECL_HIDDEN MjpegStreamDlg::Private
{

public:

    enum TabView
    {
        Server = 0,
        Stream,
        Transition,
        Effect,
        OSD             ///< On Screen Display options
    };

public:

    Private()  = default;
    ~Private() = default;

public:

    bool                dirty           = false;
    MjpegServerMngr*    mngr            = MjpegServerMngr::instance();
    QPushButton*        srvButton       = nullptr;
    QLabel*             srvStatus       = nullptr;
    QPushButton*        srvPreview      = nullptr;
    WorkingWidget*      progress        = nullptr;
    QLabel*             aStats          = nullptr;
    QLabel*             separator       = nullptr;
    QLabel*             iStats          = nullptr;
    QCheckBox*          startOnStartup  = nullptr;
    int                 spacing         = 0;
    bool                albumSupport    = false;
    QWidget*            albumSelector   = nullptr;
    DItemsList*         listView        = nullptr;
    QDialogButtonBox*   buttons         = nullptr;
    QWidget*            streamSettings  = nullptr;
    DIntNumInput*       srvPort         = nullptr;
    DIntNumInput*       delay           = nullptr;
    DIntNumInput*       rate            = nullptr;
    DIntNumInput*       quality         = nullptr;
    QCheckBox*          streamLoop      = nullptr;
    DComboBox*          typeVal         = nullptr;
    DComboBox*          effVal          = nullptr;
    EffectPreview*      effPreview      = nullptr;
    DComboBox*          transVal        = nullptr;
    TransitionPreview*  transPreview;
    QTabWidget*         tabView         = nullptr;

    FrameOsdWidget*     frameOsdWidget  = nullptr;

    MjpegStreamSettings settings;
};

} // namespace DigikamGenericMjpegStreamPlugin
