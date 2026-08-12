/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam Survey tool GUI
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "surveywindow_p.h"

namespace Digikam
{

SurveyWindow* SurveyWindow::m_instance = nullptr;

SurveyWindow* SurveyWindow::surveyWindow()
{
    if (!m_instance)
    {
        new SurveyWindow();
    }

    return m_instance;
}

bool SurveyWindow::surveyWindowCreated()
{
    return m_instance;
}

SurveyWindow::SurveyWindow()
    : DXmlGuiWindow(nullptr),
      d            (new Private)
{
    setObjectName(QLatin1String("Survey"));
    setConfigGroupName(QLatin1String("Survey Settings"));
    setXMLFile(QLatin1String("surveywindowui5.rc"));

#if (KXMLGUI_VERSION >= QT_VERSION_CHECK(5, 88, 0))

    setStateConfigGroup(configGroupName());

#endif

    setCaption(i18n("Survey"));

    m_instance = this;

    // We don't want to be deleted on close

    setAttribute(Qt::WA_DeleteOnClose, false);
}

void SurveyWindow::init(DigikamItemView* const iconView)
{
    setFullScreenOptions(FS_LIGHTTABLE);

    // -- Build the GUI -------------------------------

    setupUserArea(iconView);
    setupActions();
    setupStatusBar();

    // ------------------------------------------------

    setupConnections();
    slotColorManagementOptionsChanged();

    readSettings();

    d->sideBar->populateTags();

    applySettings();

    setAutoSaveSettings(configGroupName(), true);

    showMenuBarAction()->setChecked(!menuBar()->isHidden());  // NOTE: workaround for bug #171080
}

SurveyWindow::~SurveyWindow()
{
    m_instance = nullptr;

    delete d->sideBar;
    delete d;
}

void SurveyWindow::refreshView()
{
    d->sideBar->refreshTagsView();
}

void SurveyWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    // There is one nasty habit with the thumbnail bar if it is floating: it
    // doesn't close when the parent window does, so it needs to be manually
    // closed. If the light table is opened again, its original state needs to
    // be restored.
    // This only needs to be done when closing a visible window and not when
    // destroying a closed window, since the latter case will always report that
    // the thumbnail bar isn't visible.

    if (isVisible())
    {
        d->stack->thumbBarDock()->hide();
    }

#ifdef HAVE_MEDIAPLAYER

    if (d->stack->viewMode() == SurveyStack::MediaPlayerMode)
    {
        d->stack->mediaPlayerView()->escapePreview();
    }

#endif // HAVE_MEDIAPLAYER

    writeSettings();

    DXmlGuiWindow::closeEvent(e);
}

void SurveyWindow::showEvent(QShowEvent* e)
{
    // Restore the visibility of the thumbbar and start autosaving again.

    d->stack->thumbBarDock()->restoreVisibility();

    DXmlGuiWindow::showEvent(e);

    QTimer::singleShot(100, this, [this]()
        {
            slotItemSelected();
            d->stack->slotThumbBarSelectionChanged();
        }
    );
}

void SurveyWindow::changeEvent(QEvent* e)
{

#ifdef HAVE_MEDIAPLAYER

    if (windowState() & Qt::WindowMinimized)
    {
        if (d->stack->viewMode() == SurveyStack::MediaPlayerMode)
        {
            d->stack->mediaPlayerView()->escapePreview();
        }
    }

#endif // HAVE_MEDIAPLAYER

    DXmlGuiWindow::changeEvent(e);
}

bool SurveyWindow::isSleeping() const
{
    return (isMinimized() || isHidden());
}

bool SurveyWindow::isEmpty() const
{
    return (d->stack->thumbBar()->allItemInfos().count() == 0);
}

void SurveyWindow::toggleTag(int tagID)
{
    ItemInfo info = d->stack->thumbBar()->currentInfo();

    if (!info.isNull())
    {
        if (!info.tagIds().contains(tagID))
        {
            FileActionMngr::instance()->assignTag(info, tagID);
        }
        else
        {
            FileActionMngr::instance()->removeTag(info, tagID);
        }
    }
}

void SurveyWindow::slotAssignPickLabel(int pickId)
{
    Q_EMIT signalAssignPickLabel(pickId);
}

void SurveyWindow::slotAssignColorLabel(int colorId)
{
    Q_EMIT signalAssignColorLabel(colorId);
}

void SurveyWindow::slotAssignRating(int rating, bool toggle)
{
    Q_EMIT signalAssignRating(rating, toggle);
}

void SurveyWindow::slotItemSelected()
{
    if (isSleeping())
    {
        return;
    }

    ItemInfo info = d->stack->thumbBar()->currentInfo();

    bool hasInfo  = !info.isNull();

    d->editItemAction->setEnabled(hasInfo);
    d->fileDeleteAction->setEnabled(hasInfo);
    d->fileDeleteFinalAction->setEnabled(hasInfo);
    d->backwardAction->setEnabled(hasInfo);
    d->forwardAction->setEnabled(hasInfo);
    d->firstAction->setEnabled(hasInfo);
    d->lastAction->setEnabled(hasInfo);
    d->fileName->setText(info.name());

    if (hasInfo)
    {
        QModelIndex curr = d->stack->thumbBar()->indexForInfo(info);

        if (curr.isValid())
        {
            if (!d->stack->thumbBar()->previousIndex(curr).isValid())
            {
                d->firstAction->setEnabled(false);
            }

            if (!d->stack->thumbBar()->nextIndex(curr).isValid())
            {
                d->lastAction->setEnabled(false);
            }
        }
    }

    d->sideBar->itemChanged(info);
}

void SurveyWindow::slotDeleteItem()
{
    Q_EMIT d->stack->imagePreviewView()->signalDeleteItem();
}

void SurveyWindow::slotDeleteFinalItem()
{
    // TODO
}

void SurveyWindow::slotZoomFactorChanged(double zoom)
{
    double zmin = d->stack->zoomMin();
    double zmax = d->stack->zoomMax();
    d->zoomBar->setZoom(zoom, zmin, zmax);

    d->zoomPlusAction->setEnabled(!d->stack->maxZoom());
    d->zoomMinusAction->setEnabled(!d->stack->minZoom());
}

void SurveyWindow::slotBackward()
{
    d->stack->thumbBar()->toPreviousIndex();
}

void SurveyWindow::slotForward()
{
    d->stack->thumbBar()->toNextIndex();
}

void SurveyWindow::slotFirst()
{
    d->stack->thumbBar()->toFirstIndex();
}

void SurveyWindow::slotLast()
{
    d->stack->thumbBar()->toLastIndex();
}

void SurveyWindow::slotComponentsInfo()
{
    showDigikamComponentsInfo();
}

void SurveyWindow::slotDBStat()
{
    showDigikamDatabaseStat();
}

void SurveyWindow::slotOnlineVersionCheck()
{
    Setup::onlineVersionCheck();
}

void SurveyWindow::moveEvent(QMoveEvent* e)
{
    Q_UNUSED(e)

    Q_EMIT signalWindowHasMoved();
}

void SurveyWindow::showSideBars(bool visible)
{
    if (visible)
    {
        d->sideBar->restore();
    }
    else
    {
        d->sideBar->backup();
    }
}

void SurveyWindow::slotToggleRightSideBar()
{
    d->sideBar->isExpanded() ? d->sideBar->shrink()
                             : d->sideBar->expand();
}

void SurveyWindow::slotPreviousRightSideBarTab()
{
    d->sideBar->activePreviousTab();
}

void SurveyWindow::slotNextRightSideBarTab()
{
    d->sideBar->activeNextTab();
}

void SurveyWindow::customizedFullScreenMode(bool set)
{
    showStatusBarAction()->setEnabled(!set);
    toolBarMenuAction()->setEnabled(!set);
    showMenuBarAction()->setEnabled(!set);
    d->showBarAction->setEnabled(!set);

//    d->stack->toggleFullScreen(set);
}

void SurveyWindow::slotFileWithDefaultApplication()
{
    if (!d->stack->thumbBar()->currentInfo().isNull())
    {
        DFileOperations::openFilesWithDefaultApplication(QList<QUrl>() << d->stack->thumbBar()->currentInfo().fileUrl());
    }
}

void SurveyWindow::slotSideBarActivateTitles()
{
    d->sideBar->setActiveTab(d->sideBar->imageDescEditTab());
    d->sideBar->imageDescEditTab()->setFocusToTitlesEdit();
}

void SurveyWindow::slotSideBarActivateComments()
{
    d->sideBar->setActiveTab(d->sideBar->imageDescEditTab());
    d->sideBar->imageDescEditTab()->setFocusToCommentsEdit();
}

void SurveyWindow::slotSideBarActivateAssignedTags()
{
    d->sideBar->setActiveTab(d->sideBar->imageDescEditTab());
    d->sideBar->imageDescEditTab()->activateAssignedTagsButton();
}

void SurveyWindow::slotToggleColorManagedView()
{
    if (!IccSettings::instance()->isEnabled())
    {
        return;
    }

    bool cmv = !IccSettings::instance()->settings().useManagedPreviews;
    IccSettings::instance()->setUseManagedPreviews(cmv);
}

DInfoInterface* SurveyWindow::infoIface(DPluginAction* const ac)
{
    OperationType aset = UnspecifiedOps;

    switch (ac->actionCategory())
    {
        case DPluginAction::GenericExport:
        case DPluginAction::GenericImport:
        {
            aset = ImportExportOps;
            break;
        }

        case DPluginAction::GenericMetadata:
        {
            aset = MetadataOps;
            break;
        }

        case DPluginAction::GenericTool:
        {
            aset = ToolsOps;
            break;
        }

        case DPluginAction::GenericView:
        {
            aset = SlideshowOps;
            break;
        }

        default:
        {
            break;
        }
    }

    DBInfoIface* const iface = new DBInfoIface(this, d->stack->thumbBar()->allUrls(), aset);

    return iface;
}

} // namespace Digikam

#include "moc_surveywindow.cpp"
