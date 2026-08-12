/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : a widget to display 2 preview image on
 *               lightable to compare pictures.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAction>
#include <QFrame>
#include <QString>

// Local includes

#include "iteminfo.h"
#include "lighttablepreview.h"

namespace Digikam
{

class PreviewSettings;

class LightTableView : public QFrame
{
    Q_OBJECT

public:

    explicit LightTableView(QWidget* const parent = nullptr);
    ~LightTableView()             override;

    void   setSyncPreview(bool sync);
    void   setNavigateByPair(bool b);

    void   setLeftItemInfo(const ItemInfo& info = ItemInfo());
    void   setRightItemInfo(const ItemInfo& info = ItemInfo());

    ItemInfo leftItemInfo()  const;
    ItemInfo rightItemInfo() const;

    void   setHostWindowActions(const HostActionsMap& actions);
    void   setPreviewSettings(const PreviewSettings& settings);

    void   checkForSelection(const ItemInfo& info);
    void   toggleFullScreen(bool set);

    double leftZoomMax()     const;
    double leftZoomMin()     const;

    double rightZoomMax()    const;
    double rightZoomMin()    const;

    bool   leftMaxZoom()     const;
    bool   leftMinZoom()     const;

    bool   rightMaxZoom()    const;
    bool   rightMinZoom()    const;

    void   leftReload();
    void   rightReload();

Q_SIGNALS:

    void signalLeftPreviewSelected(bool);
    void signalRightPreviewSelected(bool);

    void signalLeftPreviewLoaded(bool);
    void signalRightPreviewLoaded(bool);

    void signalLeftZoomFactorChanged(double);
    void signalRightZoomFactorChanged(double);

    void signalLeftDroppedItems(const ItemInfoList&);
    void signalRightDroppedItems(const ItemInfoList&);

    void signalLeftPanelLeftButtonClicked();
    void signalRightPanelLeftButtonClicked();

    void signalLeftPopupTagsView();
    void signalRightPopupTagsView();

    void signalLeftSlideShowCurrent();
    void signalRightSlideShowCurrent();

    void signalDeleteItem(const ItemInfo&);
    void signalEditItem(const ItemInfo&);
    void signalToggleOnSyncPreview(bool);

    void signalLeftPreviewStartedLoading();
    void signalLeftPreviewLoadingProgress(float progress);
    void signalLeftPreviewLoadingComplete();

    void signalRightPreviewStartedLoading();
    void signalRightPreviewLoadingProgress(float progress);
    void signalRightPreviewLoadingComplete();

public Q_SLOTS:

    void slotDecreaseLeftZoom();
    void slotIncreaseLeftZoom();
    void slotLeftZoomSliderChanged(int);
    void setLeftZoomFactor(double z);
    void slotLeftFitToWindow();
    void slotLeftZoomTo100();

    void slotDecreaseRightZoom();
    void slotIncreaseRightZoom();
    void slotRightZoomSliderChanged(int);
    void setRightZoomFactor(double z);
    void slotRightFitToWindow();
    void slotRightZoomTo100();

private Q_SLOTS:

    void slotLeftContentsMoved(int, int);
    void slotRightContentsMoved(int, int);
    void slotLeftZoomFactorChanged(double);
    void slotRightZoomFactorChanged(double);
    void slotLeftPreviewLoaded(bool);
    void slotRightPreviewLoaded(bool);
    void slotDeleteLeftItem();
    void slotDeleteRightItem();

private:

    void checkForSyncPreview();

    /// To not sync right panel during left loading
    bool leftPreviewLoading()  const;

    /// To not sync left panel during right loading.
    bool rightPreviewLoading() const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
