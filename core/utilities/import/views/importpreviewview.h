/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-14-07
 * Description : An embedded view to show the cam item preview widget.
 *
 * SPDX-FileCopyrightText: 2012 by Islam Wazery  <wazery at ubuntu dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "graphicsdimgview.h"
#include "camiteminfo.h"

namespace Digikam
{

class ImportPreviewView : public GraphicsDImgView
{
    Q_OBJECT

public:

    enum Mode
    {
        IconViewPreview
    };

public:

    explicit ImportPreviewView(QWidget* const parent, Mode mode = IconViewPreview);
    ~ImportPreviewView()                   override;

    void setCamItemInfo(const CamItemInfo& info     = CamItemInfo(),
                        const CamItemInfo& previous = CamItemInfo(),
                        const CamItemInfo& next     = CamItemInfo());

    CamItemInfo getCamItemInfo()     const;

    void reload();
    void setCamItemPath(const QString& path = QString());
    void setPreviousNextPaths(const QString& previous, const QString& next);

    void showContextMenu(const CamItemInfo& info, QGraphicsSceneContextMenuEvent* event);

private:

    void setupOverlays();
    QString identifyCategoryforMime(const QString& mime);

Q_SIGNALS:

    void signalNextItem();
    void signalPrevItem();
    void signalDeleteItem();
    void signalPreviewLoaded(bool success);
    void signalEscapePreview();
/*
    FIXME
    void signalAddToExistingQueue(int);
    void signalGotoFolderAndItem(const CamItemInfo&);
    void signalGotoDateAndItem(const CamItemInfo&);
    void signalGotoTagAndItem(int);
    void signalPopupTagsView();
*/
    void signalAssignPickLabel(int);
    void signalAssignColorLabel(int);
    void signalAssignRating(int);


protected:

    bool acceptsMouseClick(QMouseEvent* e) override;
    void leaveEvent(QEvent* e)             override;
    void showEvent(QShowEvent* e)          override;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    void enterEvent(QEnterEvent*)          override;

#else

    void enterEvent(QEvent*)               override;

#endif

private Q_SLOTS:

    void camItemLoaded();
    void camItemLoadingFailed();
/*
    TODO: Implement Tags and Labels in Import Tool
    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);
    void slotAssignPickLabel(int pickId);
    void slotAssignColorLabel(int colorId);
*/
    void slotThemeChanged();
    void slotSetupChanged();

    void slotRotateLeft();
    void slotRotateRight();
    void slotDeleteItem();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
