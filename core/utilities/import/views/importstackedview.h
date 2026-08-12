/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-05-07
 * Description : QStackedWidget to handle different types of views
 *               (icon view, items preview, media view)
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QStackedWidget>

// Local includes

#include "digikam_config.h"
#include "importthumbnailbar.h"
#include "importpreviewview.h"
#include "thumbbardock.h"
#include "camiteminfo.h"
#include "importiconview.h"
#include "digikam_export.h"

#ifdef HAVE_MEDIAPLAYER
#   include "mediaplayerview.h"
#endif //HAVE_MEDIAPLAYER

#ifdef HAVE_GEOLOCATION
#   include "mapwidgetview.h"
#endif // HAVE_GEOLOCATION

namespace Digikam
{

class DIGIKAM_GUI_EXPORT ImportStackedView : public QStackedWidget
{
    Q_OBJECT

public:

    enum StackedViewMode
    {
        PreviewCameraMode = 0, ///< previewing the set of items on the camera
        PreviewImageMode,

#ifdef HAVE_GEOLOCATION

        MapWidgetMode,
        MediaPlayerMode

#else

        MediaPlayerMode,
        MapWidgetMode

#endif // HAVE_GEOLOCATION

    };

public:

    explicit ImportStackedView(QWidget* const parent = nullptr);
    ~ImportStackedView() override;

    void setDockArea(QMainWindow*);

    ThumbBarDock*       thumbBarDock()      const;
    ImportThumbnailBar* thumbBar()          const;
    ImportIconView*     importIconView()    const;
    ImportPreviewView*  importPreviewView() const;

#ifdef HAVE_GEOLOCATION

    MapWidgetView*      mapWidgetView()     const;

#endif // HAVE_GEOLOCATION

#ifdef HAVE_MEDIAPLAYER

    MediaPlayerView*    mediaPlayerView()   const;

#endif //HAVE_MEDIAPLAYER

    bool isInSingleFileMode()               const;
    bool isInMultipleFileMode()             const;
/*
    FIXME
    bool isInAbstractMode() const;
*/
    void setPreviewItem(const CamItemInfo& info = CamItemInfo(),
                        const CamItemInfo& previous = CamItemInfo(),
                        const CamItemInfo& next = CamItemInfo());

    StackedViewMode  viewMode()             const;
    void setViewMode(const StackedViewMode mode);
    void previewLoaded();

    void   increaseZoom();
    void   decreaseZoom();
    void   fitToWindow();
    void   toggleFitToWindowOr100();
    void   zoomTo100Percents();
    void   setZoomFactor(double z);
    void   setZoomFactorSnapped(double z);

    bool   maxZoom()                        const;
    bool   minZoom()                        const;
    double zoomFactor()                     const;
    double zoomMin()                        const;
    double zoomMax()                        const;

Q_SIGNALS:

    void signalNextItem();
    void signalPrevItem();
    void signalViewModeChanged();
    void signalEscapePreview();
    void signalZoomFactorChanged(double);
/*
    FIXME
    void signalGotoAlbumAndItem(const CamItemInfo&);
    void signalGotoDateAndItem(const CamItemInfo&);
    void signalGotoTagAndItem(int);
*/

private Q_SLOTS:

    void slotPreviewLoaded(bool);
    void slotZoomFactorChanged(double);
    void slotThumbBarSelectionChanged();
    void slotIconViewSelectionChanged();

private:

    void readSettings();
    void syncSelection(ImportCategorizedView* const from, ImportCategorizedView* const to);

    /// Used to return the category for a specified camera item.
    QString identifyCategoryforMime(const QString& mime) const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
