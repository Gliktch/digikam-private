/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-06-13
 * Description : digiKam Survey tool - The stacked view.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMainWindow>
#include <QStackedWidget>

// Local includes

#include "digikam_config.h"
#include "digikam_export.h"
#include "iteminfo.h"
#include "thumbbardock.h"

namespace Digikam
{

class StackedView;
class DigikamItemView;
class ItemPreviewView;
class ItemThumbnailBar;
class ItemCategorizedView;

#ifdef HAVE_MEDIAPLAYER

class ItemPreviewVideo;

#endif // HAVE_MEDIAPLAYER

class SurveyStack : public QStackedWidget
{
    Q_OBJECT

public:

    enum SurveyStackMode
    {
        SurveyStackModeFirst = 0,
        PreviewImageMode     = 0,
        MediaPlayerMode      = 1,
        SurveyStackModeLast  = 1
    };

public:

    explicit SurveyStack(DigikamItemView* const iconView, QWidget* const parent = nullptr);
    ~SurveyStack() override;

    /**
     * Attach the thumbnail dock widget to the specified QMainWindow.
     */
    void setDockArea(QMainWindow*);

    ThumbBarDock*     thumbBarDock()     const;
    ItemThumbnailBar* thumbBar()         const;
    ItemPreviewView*  imagePreviewView() const;

#ifdef HAVE_MEDIAPLAYER

    ItemPreviewVideo* mediaPlayerView()  const;

#endif // HAVE_MEDIAPLAYER

    void setPreviewItem(const ItemInfo& info = ItemInfo(),
                        const ItemInfo& previous = ItemInfo(),
                        const ItemInfo& next = ItemInfo());
    SurveyStackMode viewMode()           const;
    void setViewMode(const SurveyStackMode mode);
    void previewLoaded();

    bool   maxZoom();
    bool   minZoom();
    double zoomFactor();
    double zoomMin();
    double zoomMax();

public Q_SLOTS:

    void increaseZoom();
    void decreaseZoom();
    void fitToWindow();
    void toggleFitToWindowOr100();
    void zoomTo100Percents();
    void setZoomFactor(double z);
    void setZoomFactorSnapped(double z);
    void slotThumbBarSelectionChanged();

Q_SIGNALS:

    void signalNextItem();
    void signalPrevItem();
    void signalDeleteItem();
    void signalViewModeChanged();
    void signalEscapePreview();
    void signalZoomFactorChanged(double);
    void signalPopupTagsView();
    void signalAddToExistingQueue(int);

    void signalGotoAlbumAndItem(const ItemInfo&);
    void signalGotoDateAndItem(const ItemInfo&);
    void signalGotoTagAndItem(int);

private Q_SLOTS:

    void slotPreviewLoaded(bool);
    void slotZoomFactorChanged(double);
    void slotIconViewSelectionChanged();

private:

    void syncSelection(ItemCategorizedView* const from, ItemCategorizedView* const to);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
