/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : core image editor GUI implementation private data.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "editorwindow.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QApplication>
#include <QByteArray>
#include <QCursor>
#include <QDir>
#include <QEasingCurve>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImageReader>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QPointer>
#include <QProgressBar>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QButtonGroup>
#include <QLineEdit>
#include <QKeySequence>
#include <QPushButton>
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QMenu>
#include <QIcon>
#include <QList>
#include <QString>

// KDE includes

// Pragma directives to reduce warnings from KDE header files.
#if defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wmismatched-tags"
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <klocalizedstring.h>
#include <kactioncategory.h>
#include <kactioncollection.h>
#include <ktoolbarpopupaction.h>
#include <kxmlguifactory.h>
#include <kconfiggroup.h>
#include <kservice.h>
#include <ksharedconfig.h>

#ifdef HAVE_KIO
#   include <kopenwithdialog.h>
#endif

// Restore warnings
#if defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dmessagebox.h"
#include "actioncategorizedview.h"
#include "canvas.h"
#include "colorcorrectiondlg.h"
#include "editorcore.h"
#include "dlogoaction.h"
#include "dmetadata.h"
#include "dzoombar.h"
#include "drawdecoderwidget.h"
#include "editorstackview.h"
#include "editortool.h"
#include "editortoolsettings.h"
#include "editortooliface.h"
#include "exposurecontainer.h"
#include "dfileoperations.h"
#include "dservicemenu.h"
#include "filereadwritelock.h"
#include "filesaveoptionsbox.h"
#include "filesaveoptionsdlg.h"
#include "iccpostloadingmanager.h"
#include "iccsettings.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "imagedialog.h"
#include "iofilesettings.h"
#include "metaenginesettings.h"
#include "libsinfodlg.h"
#include "loadingcacheinterface.h"
#include "savingcontext.h"
#include "sidebar.h"
#include "softproofdialog.h"
#include "statusprogressbar.h"
#include "thememanager.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"
#include "versioningpromptusersavedlg.h"
#include "undostate.h"
#include "dfiledialog.h"
#include "dexpanderbox.h"
#include "imageiface.h"
#include "versionmanager.h"
#include "dnotificationpopup.h"
#include "dimgloader.h"
#include "categorizeditemmodel.h"
#include "autorotator.h"
#include "systemsettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN EditorWindow::Private
{
public:

    Private()  = default;
    ~Private() = default;

public:

    void plugNewVersionInFormatAction(EditorWindow* const q,
                                      QMenu* const menuAction,
                                      const QString& text,
                                      const QString& format);

public:

    const QString                configAutoZoomEntry                = QLatin1String("AutoZoom");
    const QString                configBackgroundColorEntry         = QLatin1String("BackgroundColor");
    const QString                configJpegCompressionEntry         = QLatin1String("JPEGCompression");
    const QString                configJpegSubSamplingEntry         = QLatin1String("JPEGSubSampling");
    const QString                configPngCompressionEntry          = QLatin1String("PNGCompression");
    const QString                configTiffCompressionEntry         = QLatin1String("TIFFCompression");
    const QString                configJpeg2000CompressionEntry     = QLatin1String("JPEG2000Compression");
    const QString                configJpeg2000LossLessEntry        = QLatin1String("JPEG2000LossLess");
    const QString                configPgfCompressionEntry          = QLatin1String("PGFCompression");
    const QString                configPgfLossLessEntry             = QLatin1String("PGFLossLess");
    const QString                configHeifCompressionEntry         = QLatin1String("HEIFCompression");
    const QString                configHeifLossLessEntry            = QLatin1String("HEIFLossLess");
    const QString                configJxlCompressionEntry          = QLatin1String("JXLCompression");
    const QString                configJxlLossLessEntry             = QLatin1String("JXLLossLess");
    const QString                configWebpCompressionEntry         = QLatin1String("WEBPCompression");
    const QString                configWebpLossLessEntry            = QLatin1String("WEBPLossLess");
    const QString                configAvifCompressionEntry         = QLatin1String("AVIFCompression");
    const QString                configAvifLossLessEntry            = QLatin1String("AVIFLossLess");
    const QString                configUnderExposureColorEntry      = QLatin1String("UnderExposureColor");
    const QString                configUnderExposureIndicatorEntry  = QLatin1String("UnderExposureIndicator");
    const QString                configUnderExposurePercentsEntry   = QLatin1String("UnderExposurePercentsEntry");
    const QString                configOverExposureColorEntry       = QLatin1String("OverExposureColor");
    const QString                configOverExposureIndicatorEntry   = QLatin1String("OverExposureIndicator");
    const QString                configOverExposurePercentsEntry    = QLatin1String("OverExposurePercentsEntry");
    const QString                configExpoIndicatorModeEntry       = QLatin1String("ExpoIndicatorMode");
    const QString                configUseRawImportToolEntry        = QLatin1String("UseRawImportTool");
    const QString                configRawImportToolIidEntry        = QLatin1String("RawImportToolIid");
    const QString                configUseThemeBackgroundColorEntry = QLatin1String("UseThemeBackgroundColor");
    const QString                configVerticalSplitterStateEntry   = QLatin1String("Vertical Splitter State");

    QToolButton*                 cmViewIndicator                    = nullptr;
    QToolButton*                 underExposureIndicator             = nullptr;
    QToolButton*                 overExposureIndicator              = nullptr;

    DAdjustableLabel*            infoLabel                          = nullptr;

    QAction*                     copyAction                         = nullptr;
    QAction*                     cropAction                         = nullptr;
    QAction*                     flipHorizAction                    = nullptr;
    QAction*                     flipVertAction                     = nullptr;
    QAction*                     rotateLeftAction                   = nullptr;
    QAction*                     rotateRightAction                  = nullptr;
    QAction*                     rotateAutoAction                   = nullptr;
    QAction*                     selectAllAction                    = nullptr;
    QAction*                     selectNoneAction                   = nullptr;
    QAction*                     softProofOptionsAction             = nullptr;
    QAction*                     zoomFitToSelectAction              = nullptr;
    QAction*                     zoomMinusAction                    = nullptr;
    QAction*                     zoomPlusAction                     = nullptr;
    QAction*                     zoomTo100percents                  = nullptr;
    QAction*                     openWithAction                     = nullptr;

    QEventLoop*                  waitingLoop                        = nullptr;
    QDialog*                     currentWindowModalDialog           = nullptr;

    QAction*                     zoomFitToWindowAction              = nullptr;
    QAction*                     viewCMViewAction                   = nullptr;
    QAction*                     viewSoftProofAction                = nullptr;
    QAction*                     viewUnderExpoAction                = nullptr;
    QAction*                     viewOverExpoAction                 = nullptr;

    ActionCategorizedView*       selectToolsActionView              = nullptr;

    ICCSettingsContainer*        ICCSettings                        = nullptr;

    DZoomBar*                    zoomBar                            = nullptr;
    PreviewToolBar*              previewToolBar                     = nullptr;

    ExposureSettingsContainer*   exposureSettings                   = nullptr;

    EditorToolIface*             toolIface                          = nullptr;

    VersionManager               defaultVersionManager;

    QList<int>                   fullscreenSizeBackup;

    QMap<QString, KService::Ptr> servicesMap;
    QMap<QString, DServiceInfo>  newServicesMap;
};

void EditorWindow::Private::plugNewVersionInFormatAction(EditorWindow* const q,
                                                         QMenu* const menuAction,
                                                         const QString& text,
                                                         const QString& format)
{
    QAction* const action = new QAction(text, q);

    connect(action, &QAction::triggered,
            q, [q, format]()
        {
                q->saveNewVersionInFormat(format);
        }
    );

    menuAction->addAction(action);
}

} // namespace Digikam
