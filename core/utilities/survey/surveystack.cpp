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
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "surveystack.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "applicationsettings.h"
#include "digikamitemview.h"
#include "itemiconview.h"
#include "surveywindow.h"
#include "itemalbummodel.h"
#include "itemalbumfiltermodel.h"
#include "itempreviewview.h"
#include "itemcategorizedview.h"
#include "itemthumbnailbar.h"
#include "loadingcacheinterface.h"
#include "singlephotopreviewlayout.h"
#include "dimg.h"

#ifdef HAVE_MEDIAPLAYER
#   include "itempreviewvideo.h"
#endif // HAVE_MEDIAPLAYER

namespace Digikam
{

class Q_DECL_HIDDEN SurveyStack::Private
{
public:

    Private() = default;

public:

    bool              needUpdateBar     = false;
    bool              syncingSelection  = false;

    QMainWindow*      dockArea          = nullptr;

    DigikamItemView*  imageIconView     = nullptr;
    ItemThumbnailBar* thumbBar          = nullptr;
    ItemPreviewView*  imagePreviewView  = nullptr;
    ThumbBarDock*     thumbBarDock      = nullptr;

    QMap<int, int>    stackMap;

#ifdef HAVE_MEDIAPLAYER

    ItemPreviewVideo* mediaPlayerView   = nullptr;

#endif // HAVE_MEDIAPLAYER

};

SurveyStack::SurveyStack(DigikamItemView* const iconView, QWidget* const parent)
    : QStackedWidget(parent),
      d             (new Private)
{
    d->imageIconView    = iconView;
    d->imagePreviewView = new ItemPreviewView(this);

    d->thumbBarDock     = new ThumbBarDock();
    d->thumbBar         = new ItemThumbnailBar(d->thumbBarDock);
    d->thumbBar->setModelsFiltered(d->imageIconView->itemModel(),
                                   d->imageIconView->itemFilterModel());
    d->thumbBar->installOverlays();
    d->thumbBarDock->setWidget(d->thumbBar);
    d->thumbBarDock->setObjectName(QLatin1String("survey_thumbbar"));

#ifdef HAVE_MEDIAPLAYER

    d->mediaPlayerView  = new ItemPreviewVideo(this);

#endif // HAVE_MEDIAPLAYER

    d->stackMap[addWidget(d->imagePreviewView)] = PreviewImageMode;

#ifdef HAVE_MEDIAPLAYER

    d->stackMap[addWidget(d->mediaPlayerView)]  = MediaPlayerMode;

#endif // HAVE_MEDIAPLAYER

    setViewMode(PreviewImageMode);

    // -----------------------------------------------------------------

    StackedView* const stackedView = static_cast<StackedView*>(d->imageIconView->parent());

    connect(d->imagePreviewView, SIGNAL(signalPopupTagsView()),
            stackedView, SIGNAL(signalPopupTagsView()));

    connect(d->imagePreviewView, SIGNAL(signalGotoAlbumAndItem(ItemInfo)),
            stackedView, SIGNAL(signalGotoAlbumAndItem(ItemInfo)));

    connect(d->imagePreviewView, SIGNAL(signalGotoDateAndItem(ItemInfo)),
            stackedView, SIGNAL(signalGotoDateAndItem(ItemInfo)));

    connect(d->imagePreviewView, SIGNAL(signalGotoTagAndItem(int)),
            stackedView, SIGNAL(signalGotoTagAndItem(int)));

    connect(d->imagePreviewView, SIGNAL(signalNextItem()),
            stackedView, SIGNAL(signalNextItem()));

    connect(d->imagePreviewView, SIGNAL(signalPrevItem()),
            stackedView, SIGNAL(signalPrevItem()));

    connect(d->imagePreviewView, SIGNAL(signalDeleteItem()),
            stackedView, SIGNAL(signalDeleteItem()));

    connect(d->imagePreviewView, SIGNAL(signalEscapePreview()),
            stackedView, SIGNAL(signalEscapePreview()));

    connect(d->imagePreviewView->layout(), SIGNAL(zoomFactorChanged(double)),
            this, SLOT(slotZoomFactorChanged(double)));

    connect(d->imagePreviewView, SIGNAL(signalAddToExistingQueue(int)),
            stackedView, SIGNAL(signalAddToExistingQueue(int)));

    connect(d->thumbBar, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotThumbBarSelectionChanged()));

    connect(d->imageIconView, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotIconViewSelectionChanged()));

    connect(d->thumbBarDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            d->thumbBar, SLOT(slotDockLocationChanged(Qt::DockWidgetArea)));

    connect(d->imagePreviewView, SIGNAL(signalPreviewLoaded(bool)),
            this, SLOT(slotPreviewLoaded(bool)));

#ifdef HAVE_MEDIAPLAYER

    connect(d->mediaPlayerView, SIGNAL(signalNextItem()),
            stackedView, SIGNAL(signalNextItem()));

    connect(d->mediaPlayerView, SIGNAL(signalPrevItem()),
            stackedView, SIGNAL(signalPrevItem()));

    connect(d->mediaPlayerView, SIGNAL(signalEscapePreview()),
            stackedView, SIGNAL(signalEscapePreview()));

    connect(d->mediaPlayerView, SIGNAL(signalDeleteItem()),
            stackedView, SIGNAL(signalDeleteItem()));

    connect(d->mediaPlayerView, SIGNAL(signalPopupTagsView()),
            stackedView, SIGNAL(signalPopupTagsView()));

#endif // HAVE_MEDIAPLAYER

}

SurveyStack::~SurveyStack()
{
    delete d;
}

void SurveyStack::setDockArea(QMainWindow* dockArea)
{
    // Attach the thumbbar dock to the given dock area and place it initially on top.

    d->dockArea = dockArea;
    d->thumbBarDock->setParent(d->dockArea);
    d->dockArea->addDockWidget(Qt::BottomDockWidgetArea, d->thumbBarDock);
    d->thumbBarDock->setFloating(false);
}

ThumbBarDock* SurveyStack::thumbBarDock() const
{
    return d->thumbBarDock;
}

ItemThumbnailBar* SurveyStack::thumbBar() const
{
    return d->thumbBar;
}

ItemPreviewView* SurveyStack::imagePreviewView() const
{
    return d->imagePreviewView;
}


#ifdef HAVE_MEDIAPLAYER

ItemPreviewVideo* SurveyStack::mediaPlayerView() const
{
    return d->mediaPlayerView;
}

#endif // HAVE_MEDIAPLAYER

void SurveyStack::setPreviewItem(const ItemInfo& info, const ItemInfo& previous, const ItemInfo& next)
{
    if (SurveyWindow::surveyWindow()->isSleeping())
    {
        return;
    }

    if (info.isNull())
    {
        if      (viewMode() == MediaPlayerMode)
        {

#ifdef HAVE_MEDIAPLAYER

            d->mediaPlayerView->setCurrentItem();

#endif // HAVE_MEDIAPLAYER

        }
        else if (viewMode() == PreviewImageMode)
        {
            d->imagePreviewView->setItemInfo();
        }
    }
    else
    {
        // Special case for animated image as GIF or NMG

        if (
            (info.category() == DatabaseItem::Audio)      ||
            (info.category() == DatabaseItem::Video)      ||
            DImg::isAnimatedImage(info.fileUrl().toLocalFile())
           )
        {
            // Stop image viewer

            if (viewMode() == PreviewImageMode)
            {
                d->imagePreviewView->setItemInfo();
            }

#ifdef HAVE_MEDIAPLAYER

            setViewMode(MediaPlayerMode);
            d->mediaPlayerView->setItemInfo(info, previous, next);

#endif // HAVE_MEDIAPLAYER

        }
        else // Static image or Raw image.
        {
            // Stop media player if running...

            if (viewMode() == MediaPlayerMode)
            {

#ifdef HAVE_MEDIAPLAYER

                d->mediaPlayerView->setCurrentItem();

#endif // HAVE_MEDIAPLAYER

            }

            d->imagePreviewView->setItemInfo(info, previous, next);

            // NOTE: No need to toggle immediately in PreviewImageMode here,
            // because we will receive a signal for that when the image preview will be loaded.
            // This will prevent a flicker effect with the old image preview loaded in stack.
        }

        // do not touch the selection, only adjust current info

        QModelIndex currentIndex = d->thumbBar->itemSortFilterModel()->indexForItemInfo(info);
        d->thumbBar->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::NoUpdate);
    }
}

SurveyStack::SurveyStackMode SurveyStack::viewMode() const
{
    return SurveyStackMode(d->stackMap.value(currentIndex()));
}

void SurveyStack::setViewMode(const SurveyStackMode mode)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Survey Stack View Mode : " << mode;

#ifdef HAVE_MEDIAPLAYER

    if ((viewMode() == MediaPlayerMode) && (mode != MediaPlayerMode))
    {
        d->mediaPlayerView->escapePreview();
    }

#endif // HAVE_MEDIAPLAYER

    if ((mode < SurveyStackModeFirst) || (mode > SurveyStackModeLast))
    {
        return;
    }

    // cppcheck-suppress knownConditionTrueFalse
    if ((mode == PreviewImageMode) || (mode == MediaPlayerMode))
    {
        d->thumbBarDock->restoreVisibility();
        syncSelection(d->imageIconView, d->thumbBar);
    }

    setCurrentIndex(d->stackMap.key(mode));

    Q_EMIT signalViewModeChanged();
}

void SurveyStack::syncSelection(ItemCategorizedView* const from, ItemCategorizedView* const to)
{
    const ImageSortFilterModel* const fromModel = from->itemSortFilterModel();
    const ImageSortFilterModel* const toModel   = to->itemSortFilterModel();
    QModelIndex currentIndex                    = toModel->indexForItemInfo(from->currentInfo());

    // sync selection

    QItemSelection selection                    = from->selectionModel()->selection();
    QItemSelection newSelection;

    for (const QItemSelectionRange& range : std::as_const(selection))
    {
        QModelIndex topLeft     = toModel->indexForItemInfo(fromModel->imageInfo(range.topLeft()));
        QModelIndex bottomRight = toModel->indexForItemInfo(fromModel->imageInfo(range.bottomRight()));
        newSelection.select(topLeft, bottomRight);
    }

    d->syncingSelection = true;

    if (currentIndex.isValid())
    {
        // set current info

        to->setCurrentIndex(currentIndex);
    }

    to->selectionModel()->select(newSelection, QItemSelectionModel::ClearAndSelect);
    d->syncingSelection = false;
}

void SurveyStack::slotThumbBarSelectionChanged()
{
    if (
        (viewMode() != MediaPlayerMode)  &&
        (viewMode() != PreviewImageMode)
       )
    {
        return;
    }

    if (d->syncingSelection)
    {
        return;
    }

    syncSelection(d->thumbBar, d->imageIconView);

    ItemInfo info = d->thumbBar->currentInfo();
    ItemInfo prev = d->thumbBar->previousInfo(info);
    ItemInfo next = d->thumbBar->nextInfo(info);

    setPreviewItem(info, prev, next);
}

void SurveyStack::slotIconViewSelectionChanged()
{
    if (
        (viewMode() != MediaPlayerMode)  &&
        (viewMode() != PreviewImageMode)
       )
    {
        return;
    }

    if (d->syncingSelection)
    {
        return;
    }

    syncSelection(d->imageIconView, d->thumbBar);

    ItemInfo info = d->thumbBar->currentInfo();
    ItemInfo prev = d->thumbBar->previousInfo(info);
    ItemInfo next = d->thumbBar->nextInfo(info);

    setPreviewItem(info, prev, next);
}

void SurveyStack::previewLoaded()
{
    Q_EMIT signalViewModeChanged();
}

void SurveyStack::slotZoomFactorChanged(double z)
{
    if (viewMode() == PreviewImageMode)
    {
        Q_EMIT signalZoomFactorChanged(z);
    }
}

void SurveyStack::increaseZoom()
{
    d->imagePreviewView->layout()->increaseZoom();
}

void SurveyStack::decreaseZoom()
{
    d->imagePreviewView->layout()->decreaseZoom();
}

void SurveyStack::zoomTo100Percents()
{
    d->imagePreviewView->layout()->setZoomFactor(1.0, QPoint());
}

void SurveyStack::fitToWindow()
{
    d->imagePreviewView->layout()->fitToWindow();
}

void SurveyStack::toggleFitToWindowOr100()
{
    d->imagePreviewView->layout()->toggleFitToWindowOr100();
}

bool SurveyStack::maxZoom()
{
    return d->imagePreviewView->layout()->atMaxZoom();
}

bool SurveyStack::minZoom()
{
    return d->imagePreviewView->layout()->atMinZoom();
}

void SurveyStack::setZoomFactor(double z)
{
    // Giving a null anchor means to use the current view center

    d->imagePreviewView->layout()->setZoomFactor(z, QPoint());
}

void SurveyStack::setZoomFactorSnapped(double z)
{
    d->imagePreviewView->layout()->setZoomFactor(z, QPoint(), SinglePhotoPreviewLayout::SnapZoomFactor);
}

double SurveyStack::zoomFactor()
{
    return d->imagePreviewView->layout()->zoomFactor();
}

double SurveyStack::zoomMin()
{
    return d->imagePreviewView->layout()->minZoomFactor();
}

double SurveyStack::zoomMax()
{
    return d->imagePreviewView->layout()->maxZoomFactor();
}

void SurveyStack::slotPreviewLoaded(bool)
{
    setViewMode(SurveyStack::PreviewImageMode);
    previewLoaded();
}

} // namespace Digikam

#include "moc_surveystack.cpp"
