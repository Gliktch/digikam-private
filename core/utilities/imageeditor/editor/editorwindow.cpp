/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : core image editor GUI implementation
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "editorwindow_p.h"

namespace Digikam
{

EditorWindow::EditorWindow(const QString& name, QWidget* const parent)
    : DXmlGuiWindow(parent),
      d            (new Private)
{
    setConfigGroupName(QLatin1String("ImageViewer Settings"));
    setObjectName(name);
    setFullScreenOptions(FS_EDITOR);

    // Settings containers instance.

    d->exposureSettings            = new ExposureSettingsContainer();
    d->toolIface                   = new EditorToolIface(this);
    m_IOFileSettings               = new IOFileSettings();
/*
    d->waitingLoop                 = new QEventLoop(this);
*/
}

EditorWindow::~EditorWindow()
{
    delete m_canvas;
    delete m_IOFileSettings;
    delete d->toolIface;
    delete d->exposureSettings;
    delete d;
}

SidebarSplitter* EditorWindow::sidebarSplitter() const
{
    return m_splitter;
}

EditorStackView* EditorWindow::editorStackView() const
{
    return m_stackView;
}

ExposureSettingsContainer* EditorWindow::exposureSettings() const
{
    return d->exposureSettings;
}

void EditorWindow::setupContextMenu()
{
    m_contextMenu = new QMenu(this);

    addAction2ContextMenu(QLatin1String("editorwindow_fullscreen"),            true);
    addAction2ContextMenu(QLatin1String("options_show_menubar"),               true);
    m_contextMenu->addSeparator();

    // --------------------------------------------------------

    addAction2ContextMenu(QLatin1String("editorwindow_backward"),              true);
    addAction2ContextMenu(QLatin1String("editorwindow_forward"),               true);
    m_contextMenu->addSeparator();

    // --------------------------------------------------------

    addAction2ContextMenu(QLatin1String("slideshow_plugin"),                   true);
    addAction2ContextMenu(QLatin1String("editorwindow_transform_rotateleft"),  true);
    addAction2ContextMenu(QLatin1String("editorwindow_transform_rotateright"), true);
    addAction2ContextMenu(QLatin1String("editorwindow_transform_rotateauto"),  true);
    addAction2ContextMenu(QLatin1String("editorwindow_transform_crop"),        true);
    m_contextMenu->addSeparator();

    // --------------------------------------------------------

    addAction2ContextMenu(QLatin1String("editorwindow_delete"),                true);
}

void EditorWindow::setupStandardConnections()
{
    connect(m_stackView, SIGNAL(signalToggleOffFitToWindow()),
            this, SLOT(slotToggleOffFitToWindow()));

    // -- Canvas connections ------------------------------------------------

    connect(m_canvas, SIGNAL(signalShowNextImage()),
            this, SLOT(slotForward()));

    connect(m_canvas, SIGNAL(signalShowPrevImage()),
            this, SLOT(slotBackward()));

    connect(m_canvas, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));

    connect(m_stackView, SIGNAL(signalZoomChanged(bool,bool,double)),
            this, SLOT(slotZoomChanged(bool,bool,double)));

    connect(m_canvas, SIGNAL(signalChanged()),
            this, SLOT(slotChanged()));

    connect(m_canvas, SIGNAL(signalAddedDropedItems(QDropEvent*)),
            this, SLOT(slotAddedDropedItems(QDropEvent*)));

    connect(m_canvas->interface(), SIGNAL(signalUndoStateChanged()),
            this, SLOT(slotUndoStateChanged()));

    connect(m_canvas, SIGNAL(signalSelected(bool)),
            this, SLOT(slotSelected(bool)));

    connect(m_canvas, SIGNAL(signalPrepareToLoad()),
            this, SLOT(slotPrepareToLoad()));

    connect(m_canvas, SIGNAL(signalLoadingStarted(QString)),
            this, SLOT(slotLoadingStarted(QString)));

    connect(m_canvas, SIGNAL(signalLoadingFinished(QString,bool)),
            this, SLOT(slotLoadingFinished(QString,bool)));

    connect(m_canvas, SIGNAL(signalLoadingProgress(QString,float)),
            this, SLOT(slotLoadingProgress(QString,float)));

    connect(m_canvas, SIGNAL(signalSavingStarted(QString)),
            this, SLOT(slotSavingStarted(QString)));

    connect(m_canvas, SIGNAL(signalSavingFinished(QString,bool)),
            this, SLOT(slotSavingFinished(QString,bool)));

    connect(m_canvas, SIGNAL(signalSavingProgress(QString,float)),
            this, SLOT(slotSavingProgress(QString,float)));

    connect(m_canvas, SIGNAL(signalSelectionChanged(QRect)),
            this, SLOT(slotSelectionChanged(QRect)));

    connect(m_canvas, SIGNAL(signalSelectionSetText(QRect)),
            this, SLOT(slotSelectionSetText(QRect)));

    connect(m_canvas->interface(), SIGNAL(signalFileOriginChanged(QString)),
            this, SLOT(slotFileOriginChanged(QString)));

    // -- status bar connections --------------------------------------

    connect(m_nameLabel, SIGNAL(signalCancelButtonPressed()),
            this, SLOT(slotNameLabelCancelButtonPressed()));

    connect(m_nameLabel, SIGNAL(signalCancelButtonPressed()),
            d->toolIface, SLOT(slotToolAborted()));

    // -- Icc settings connections --------------------------------------

    connect(IccSettings::instance(), SIGNAL(signalSettingsChanged()),
            this, SLOT(slotColorManagementOptionsChanged()));
}

void EditorWindow::setupStandardActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    KActionCollection* const ac = actionCollection();

    m_backwardAction = buildStdAction(StdBackAction, this, SLOT(slotBackward()), this);
    ac->addAction(QLatin1String("editorwindow_backward"), m_backwardAction);
    ac->setDefaultShortcuts(m_backwardAction, QList<QKeySequence>() << Qt::Key_PageUp << Qt::Key_Backspace
                                                                    << Qt::Key_Up << Qt::Key_Left);
    m_backwardAction->setEnabled(false);

    m_forwardAction = buildStdAction(StdForwardAction, this, SLOT(slotForward()), this);
    ac->addAction(QLatin1String("editorwindow_forward"), m_forwardAction);
    ac->setDefaultShortcuts(m_forwardAction, QList<QKeySequence>() << Qt::Key_PageDown << Qt::Key_Space
                                                                   << Qt::Key_Down << Qt::Key_Right);
    m_forwardAction->setEnabled(false);

    m_firstAction = new QAction(QIcon::fromTheme(QLatin1String("go-first")),
                                i18nc("@action; go to first item", "&First"), this);
    connect(m_firstAction, SIGNAL(triggered()), this, SLOT(slotFirst()));
    ac->addAction(QLatin1String("editorwindow_first"), m_firstAction);
    ac->setDefaultShortcut(m_firstAction, QKeySequence(Qt::CTRL | Qt::Key_Home));
    m_firstAction->setEnabled(false);

    m_lastAction = new QAction(QIcon::fromTheme(QLatin1String("go-last")),
                               i18nc("@action; go to last item", "&Last"), this);
    connect(m_lastAction, SIGNAL(triggered()), this, SLOT(slotLast()));
    ac->addAction(QLatin1String("editorwindow_last"), m_lastAction);
    ac->setDefaultShortcut(m_lastAction, QKeySequence(Qt::CTRL | Qt::Key_End));
    m_lastAction->setEnabled(false);

    m_openVersionAction = new QAction(QIcon::fromTheme(QLatin1String("view-preview")),
                                      i18nc("@action", "Open Original"), this);
    connect(m_openVersionAction, SIGNAL(triggered()), this, SLOT(slotOpenOriginal()));
    ac->addAction(QLatin1String("editorwindow_openversion"), m_openVersionAction);
    ac->setDefaultShortcut(m_openVersionAction, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_End));

    m_saveAction = buildStdAction(StdSaveAction, this, SLOT(saveOrSaveAs()), this);
    ac->addAction(QLatin1String("editorwindow_save"), m_saveAction);

    m_saveAsAction = buildStdAction(StdSaveAsAction, this, SLOT(saveAs()), this);
    ac->addAction(QLatin1String("editorwindow_saveas"), m_saveAsAction);

    m_saveCurrentVersionAction = new QAction(QIcon::fromTheme(QLatin1String("dialog-ok-apply")),
                                             i18nc("@action Save changes to current version", "Save Changes"), this);
    m_saveCurrentVersionAction->setToolTip(i18nc("@info:tooltip", "Save the modifications to the current version of the file"));
    connect(m_saveCurrentVersionAction, SIGNAL(triggered()), this, SLOT(saveCurrentVersion()));
    ac->addAction(QLatin1String("editorwindow_savecurrentversion"), m_saveCurrentVersionAction);

    m_saveNewVersionAction = new KToolBarPopupAction(QIcon::fromTheme(QLatin1String("list-add")),
                                                     i18nc("@action Save changes to a newly created version", "Save As New Version"), this);
    m_saveNewVersionAction->setToolTip(i18nc("@info:tooltip", "Save the current modifications to a new version of the file"));
    connect(m_saveNewVersionAction, SIGNAL(triggered()), this, SLOT(saveNewVersion()));
    ac->addAction(QLatin1String("editorwindow_savenewversion"), m_saveNewVersionAction);

    m_saveNewVersionAsAction = new QAction(QIcon::fromTheme(QLatin1String("document-save-as")),
                                           i18nc("@action Save changes to a newly created version, specifying the filename and format",
                                                 "Save New Version As..."), this);
    m_saveNewVersionAsAction->setToolTip(i18nc("@info:tooltip", "Save the current modifications to a new version of the file, "
                                               "specifying the filename and format"));
    connect(m_saveNewVersionAsAction, SIGNAL(triggered()), this, SLOT(saveNewVersionAs()));

    m_saveNewVersionInFormatAction = new QMenu(i18nc("@action Save As New Version...Save in format...",
                                                     "Save in Format"), this);
    m_saveNewVersionInFormatAction->setIcon(QIcon::fromTheme(QLatin1String("view-preview")));
    d->plugNewVersionInFormatAction(this, m_saveNewVersionInFormatAction, i18nc("@action:inmenu", "JPEG"),       QLatin1String("JPG"));
    d->plugNewVersionInFormatAction(this, m_saveNewVersionInFormatAction, i18nc("@action:inmenu", "TIFF"),       QLatin1String("TIFF"));
    d->plugNewVersionInFormatAction(this, m_saveNewVersionInFormatAction, i18nc("@action:inmenu", "PNG"),        QLatin1String("PNG"));
    d->plugNewVersionInFormatAction(this, m_saveNewVersionInFormatAction, i18nc("@action:inmenu", "PGF"),        QLatin1String("PGF"));

#ifdef HAVE_JASPER

    d->plugNewVersionInFormatAction(this, m_saveNewVersionInFormatAction, i18nc("@action:inmenu", "JPEG 2000"),  QLatin1String("JP2"));

#endif // HAVE_JASPER

#ifdef HAVE_X265

    d->plugNewVersionInFormatAction(this, m_saveNewVersionInFormatAction, i18nc("@action:inmenu", "HEIF"),       QLatin1String("HEIF"));

#endif // HAVE_X265

    if (DPluginLoader::instance()->canExport(QLatin1String("JXL")))
    {
        d->plugNewVersionInFormatAction(this, m_saveNewVersionInFormatAction, i18nc("@action:inmenu", "JXL"),    QLatin1String("JXL"));
    }

    if (DPluginLoader::instance()->canExport(QLatin1String("WEBP")))
    {
        d->plugNewVersionInFormatAction(this, m_saveNewVersionInFormatAction, i18nc("@action:inmenu", "WEBP"),   QLatin1String("WEBP"));
    }

    if (DPluginLoader::instance()->canExport(QLatin1String("AVIF")))
    {
        d->plugNewVersionInFormatAction(this, m_saveNewVersionInFormatAction, i18nc("@action:inmenu", "AVIF"),   QLatin1String("AVIF"));
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    m_saveNewVersionAction->popupMenu()->addAction(m_saveNewVersionAsAction);
    m_saveNewVersionAction->popupMenu()->addAction(m_saveNewVersionInFormatAction->menuAction());

#else

    m_saveNewVersionAction->menu()->addAction(m_saveNewVersionAsAction);
    m_saveNewVersionAction->menu()->addAction(m_saveNewVersionInFormatAction->menuAction());

#endif

    // This also triggers saveAs, but in the context of non-destructive we want a slightly different appearance

    m_exportAction = new QAction(QIcon::fromTheme(QLatin1String("document-export")),
                                 i18nc("@action", "Export"), this);
    m_exportAction->setToolTip(i18nc("@info:tooltip", "Save the file in a folder outside your collection"));
    connect(m_exportAction, SIGNAL(triggered()), this, SLOT(saveAs()));
    ac->addAction(QLatin1String("editorwindow_export"), m_exportAction);
    ac->setDefaultShortcut(m_exportAction, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_E)); // NOTE: Gimp shortcut

    m_revertAction = buildStdAction(StdRevertAction, this, SLOT(slotRevert()), this);
    ac->addAction(QLatin1String("editorwindow_revert"), m_revertAction);

    m_discardChangesAction = new QAction(QIcon::fromTheme(QLatin1String("task-reject")),
                                         i18nc("@action", "Discard Changes"), this);
    m_discardChangesAction->setToolTip(i18nc("@info:tooltip", "Discard all current changes to this file"));
    connect(m_discardChangesAction, SIGNAL(triggered()), this, SLOT(slotDiscardChanges()));
    ac->addAction(QLatin1String("editorwindow_discardchanges"), m_discardChangesAction);

    m_openVersionAction->setEnabled(false);
    m_saveAction->setEnabled(false);
    m_saveAsAction->setEnabled(false);
    m_saveCurrentVersionAction->setEnabled(false);
    m_saveNewVersionAction->setEnabled(false);
    m_revertAction->setEnabled(false);
    m_discardChangesAction->setEnabled(false);

    d->openWithAction = new QAction(QIcon::fromTheme(QLatin1String("preferences-desktop-filetype-association")),
                                    i18nc("@action", "Open With Default Application"), this);
    d->openWithAction->setWhatsThis(i18nc("@info", "Open the item with default assigned application."));
    connect(d->openWithAction, SIGNAL(triggered()), this, SLOT(slotFileWithDefaultApplication()));
    ac->addAction(QLatin1String("open_with_default_application"), d->openWithAction);
    ac->setDefaultShortcut(d->openWithAction, QKeySequence(Qt::CTRL | Qt::Key_F4));
    d->openWithAction->setEnabled(false);

    m_fileDeleteAction = new QAction(QIcon::fromTheme(QLatin1String("albumfolder-user-trash")),  // In red - bug 513774
                                     i18nc("@action: Non-pluralized", "Move to Trash"), this);
    connect(m_fileDeleteAction, SIGNAL(triggered()), this, SLOT(slotDeleteCurrentItem()));
    ac->addAction(QLatin1String("editorwindow_delete"), m_fileDeleteAction);
    ac->setDefaultShortcut(m_fileDeleteAction, QKeySequence(Qt::Key_Delete));
    m_fileDeleteAction->setEnabled(false);

    QAction* const closeAction = buildStdAction(StdCloseAction, this, SLOT(slotClose()), this);
    ac->addAction(QLatin1String("editorwindow_close"), closeAction);

    // -- Standard 'Edit' menu actions ---------------------------------------------

    d->copyAction = buildStdAction(StdCopyAction, m_canvas, SLOT(slotCopy()), this);
    ac->addAction(QLatin1String("editorwindow_copy"), d->copyAction);
    d->copyAction->setEnabled(false);

    m_undoAction = new KToolBarPopupAction(QIcon::fromTheme(QLatin1String("edit-undo")),
                                           i18nc("@action", "Undo"), this);
    m_undoAction->setEnabled(false);
    ac->addAction(QLatin1String("editorwindow_undo"), m_undoAction);
    ac->setDefaultShortcut(m_undoAction, QKeySequence(Qt::CTRL | Qt::Key_Z));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    m_undoAction->setMenu(m_undoAction->popupMenu());

    connect(m_undoAction->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowUndoMenu()));

#else

    connect(m_undoAction->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowUndoMenu()));

#endif

    // connect simple undo action

    connect(m_undoAction, &QAction::triggered,
            this, [this]()
        {
             m_canvas->slotUndo(1);
        }
    );

    m_redoAction = new KToolBarPopupAction(QIcon::fromTheme(QLatin1String("edit-redo")),
                                           i18nc("@action", "Redo"), this);
    m_redoAction->setEnabled(false);
    ac->addAction(QLatin1String("editorwindow_redo"), m_redoAction);
    ac->setDefaultShortcut(m_redoAction, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    m_redoAction->setMenu(m_redoAction->popupMenu());

    connect(m_redoAction->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowRedoMenu()));

#else

    connect(m_redoAction->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowRedoMenu()));

#endif

    // connect simple redo action

    connect(m_redoAction, &QAction::triggered,
            this, [this]()
        {
            m_canvas->slotRedo(1);
        }
    );

    d->selectAllAction = new QAction(i18nc("@action: create a selection containing the full image", "Select All"), this);
    connect(d->selectAllAction, SIGNAL(triggered()), m_canvas, SLOT(slotSelectAll()));
    ac->addAction(QLatin1String("editorwindow_selectAll"), d->selectAllAction);
    ac->setDefaultShortcut(d->selectAllAction, QKeySequence(Qt::CTRL | Qt::Key_A));

    d->selectNoneAction = new QAction(i18nc("@action", "Select None"), this);
    connect(d->selectNoneAction, SIGNAL(triggered()), m_canvas, SLOT(slotSelectNone()));
    ac->addAction(QLatin1String("editorwindow_selectNone"), d->selectNoneAction);
    ac->setDefaultShortcut(d->selectNoneAction, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A));

    // -- Standard 'View' menu actions ---------------------------------------------

    d->zoomPlusAction  = buildStdAction(StdZoomInAction, this, SLOT(slotIncreaseZoom()), this);
    ac->addAction(QLatin1String("editorwindow_zoomplus"), d->zoomPlusAction);

    d->zoomMinusAction = buildStdAction(StdZoomOutAction, this, SLOT(slotDecreaseZoom()), this);
    ac->addAction(QLatin1String("editorwindow_zoomminus"), d->zoomMinusAction);

    d->zoomTo100percents = new QAction(QIcon::fromTheme(QLatin1String("zoom-original")),
                                       i18nc("@action", "Zoom to 100%"), this);
    connect(d->zoomTo100percents, SIGNAL(triggered()), this, SLOT(slotZoomTo100Percents()));
    ac->addAction(QLatin1String("editorwindow_zoomto100percents"), d->zoomTo100percents);
    ac->setDefaultShortcut(d->zoomTo100percents, QKeySequence(Qt::CTRL | Qt::Key_Period));

    d->zoomFitToWindowAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-fit-best")),
                                           i18nc("@action", "Fit to &Window"), this);
    d->zoomFitToWindowAction->setCheckable(true);
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), this, SLOT(slotToggleFitToWindow()));
    ac->addAction(QLatin1String("editorwindow_zoomfit2window"), d->zoomFitToWindowAction);
    ac->setDefaultShortcut(d->zoomFitToWindowAction, QKeySequence(Qt::ALT | Qt::CTRL | Qt::Key_E));

    d->zoomFitToSelectAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-select-fit")),
                                           i18nc("@action", "Fit to &Selection"), this);
    connect(d->zoomFitToSelectAction, SIGNAL(triggered()), this, SLOT(slotFitToSelect()));
    ac->addAction(QLatin1String("editorwindow_zoomfit2select"), d->zoomFitToSelectAction);
    ac->setDefaultShortcut(d->zoomFitToSelectAction, QKeySequence(Qt::ALT | Qt::CTRL | Qt::Key_S));   // NOTE: Photoshop 7 use ALT+CTRL+0
    d->zoomFitToSelectAction->setEnabled(false);
    d->zoomFitToSelectAction->setWhatsThis(i18nc("@info", "This option can be used to zoom the image to the "
                                                          "current selection area."));

    // **********************************************************

    QList<DPluginAction*> gactions = DPluginLoader::instance()->pluginsActions(DPluginAction::Generic, this);

    for (DPluginAction* const gac : std::as_const(gactions))
    {
        if (gac->actionCategory() != DPluginAction::GenericImport)
        {
            gac->setEnabled(false);
        }
    }

    QList<DPluginAction*> eactions = DPluginLoader::instance()->pluginsActions(DPluginAction::Editor, this);

    for (DPluginAction* const eac : std::as_const(eactions))
    {
        eac->setEnabled(false);
    }

    // --------------------------------------------------------

    createFullScreenAction(QLatin1String("editorwindow_fullscreen"));
    createSidebarActions();

    d->viewUnderExpoAction = new QAction(QIcon::fromTheme(QLatin1String("underexposure")),
                                         i18nc("@action", "Under-Exposure Indicator"), this);
    d->viewUnderExpoAction->setCheckable(true);
    d->viewUnderExpoAction->setWhatsThis(i18nc("@info", "Set this option to display black "
                                               "overlaid on the image. This will help you to avoid "
                                               "under-exposing the image."));
    connect(d->viewUnderExpoAction, SIGNAL(triggered(bool)), this, SLOT(slotSetUnderExposureIndicator(bool)));
    ac->addAction(QLatin1String("editorwindow_underexposure"), d->viewUnderExpoAction);
    ac->setDefaultShortcut(d->viewUnderExpoAction, QKeySequence(Qt::Key_F11));

    d->viewOverExpoAction = new QAction(QIcon::fromTheme(QLatin1String("overexposure")),
                                        i18nc("@action", "Over-Exposure Indicator"), this);
    d->viewOverExpoAction->setCheckable(true);
    d->viewOverExpoAction->setWhatsThis(i18nc("@info", "Set this option to display white "
                                              "overlaid on the image. This will help you to avoid "
                                              "over-exposing the image."));
    connect(d->viewOverExpoAction, SIGNAL(triggered(bool)), this, SLOT(slotSetOverExposureIndicator(bool)));
    ac->addAction(QLatin1String("editorwindow_overexposure"), d->viewOverExpoAction);
    ac->setDefaultShortcut(d->viewOverExpoAction, QKeySequence(Qt::CTRL | Qt::Key_F11));

    d->viewCMViewAction = new QAction(QIcon::fromTheme(QLatin1String("video-display")),
                                      i18nc("@action", "Color-Managed View"), this);
    d->viewCMViewAction->setCheckable(true);
    connect(d->viewCMViewAction, SIGNAL(triggered()), this, SLOT(slotToggleColorManagedView()));
    ac->addAction(QLatin1String("editorwindow_cmview"), d->viewCMViewAction);
    ac->setDefaultShortcut(d->viewCMViewAction, QKeySequence(Qt::Key_F12));

    d->softProofOptionsAction = new QAction(QIcon::fromTheme(QLatin1String("document-print")),
                                            i18nc("@action", "Soft Proofing Options..."), this);
    connect(d->softProofOptionsAction, SIGNAL(triggered()), this, SLOT(slotSoftProofingOptions()));
    ac->addAction(QLatin1String("editorwindow_softproofoptions"), d->softProofOptionsAction);

    d->viewSoftProofAction = new QAction(QIcon::fromTheme(QLatin1String("document-preview-archive")),
                                         i18nc("@action", "Soft Proofing View"), this);
    d->viewSoftProofAction->setCheckable(true);
    connect(d->viewSoftProofAction, SIGNAL(triggered()), this, SLOT(slotUpdateSoftProofingState()));
    ac->addAction(QLatin1String("editorwindow_softproofview"), d->viewSoftProofAction);

    // -- Standard 'Transform' menu actions ---------------------------------------------

    d->cropAction = new QAction(QIcon::fromTheme(QLatin1String("transform-crop-and-resize")),
                                i18nc("@action", "Crop to Selection"), this);
    connect(d->cropAction, SIGNAL(triggered()), m_canvas, SLOT(slotCrop()));
    d->cropAction->setEnabled(false);
    d->cropAction->setWhatsThis(i18nc("@info", "This option can be used to crop the image. "
                                      "Select a region of the image to enable this action."));
    ac->addAction(QLatin1String("editorwindow_transform_crop"), d->cropAction);
    ac->setDefaultShortcut(d->cropAction, QKeySequence(Qt::CTRL | Qt::Key_X));

    // -- Standard 'Flip' menu actions ---------------------------------------------

    d->flipHorizAction = new QAction(QIcon::fromTheme(QLatin1String("object-flip-horizontal")),
                                     i18nc("@action", "Flip Horizontally"), this);
    connect(d->flipHorizAction, SIGNAL(triggered()), m_canvas, SLOT(slotFlipHoriz()));
    connect(d->flipHorizAction, SIGNAL(triggered()), this, SLOT(slotFlipHIntoQue()));
    ac->addAction(QLatin1String("editorwindow_transform_fliphoriz"), d->flipHorizAction);
    ac->setDefaultShortcut(d->flipHorizAction, QKeySequence(Qt::CTRL | Qt::Key_Asterisk));
    d->flipHorizAction->setEnabled(false);

    d->flipVertAction = new QAction(QIcon::fromTheme(QLatin1String("object-flip-vertical")),
                                    i18nc("@action", "Flip Vertically"), this);
    connect(d->flipVertAction, SIGNAL(triggered()), m_canvas, SLOT(slotFlipVert()));
    connect(d->flipVertAction, SIGNAL(triggered()), this, SLOT(slotFlipVIntoQue()));
    ac->addAction(QLatin1String("editorwindow_transform_flipvert"), d->flipVertAction);
    ac->setDefaultShortcut(d->flipVertAction, QKeySequence(Qt::CTRL | Qt::Key_Slash));
    d->flipVertAction->setEnabled(false);

    // -- Standard 'Rotate' menu actions ----------------------------------------

    d->rotateLeftAction = new QAction(QIcon::fromTheme(QLatin1String("object-rotate-left")),
                                      i18nc("@action", "Rotate Left"), this);
    connect(d->rotateLeftAction, SIGNAL(triggered()), m_canvas, SLOT(slotRotate270()));
    connect(d->rotateLeftAction, SIGNAL(triggered()), this, SLOT(slotRotateLeftIntoQue()));
    ac->addAction(QLatin1String("editorwindow_transform_rotateleft"), d->rotateLeftAction);
    ac->setDefaultShortcut(d->rotateLeftAction, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Left));
    d->rotateLeftAction->setEnabled(false);

    d->rotateRightAction = new QAction(QIcon::fromTheme(QLatin1String("object-rotate-right")),
                                       i18nc("@action", "Rotate Right"), this);
    connect(d->rotateRightAction, SIGNAL(triggered()), m_canvas, SLOT(slotRotate90()));
    connect(d->rotateRightAction, SIGNAL(triggered()), this, SLOT(slotRotateRightIntoQue()));
    ac->addAction(QLatin1String("editorwindow_transform_rotateright"), d->rotateRightAction);
    ac->setDefaultShortcut(d->rotateRightAction, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Right));
    d->rotateRightAction->setEnabled(false);

    if (SystemSettings(qApp->applicationName()).enableAIAutoTools)
    {
        d->rotateAutoAction = new QAction(QIcon::fromTheme(QLatin1String("image-rotate-right-symbolic")),
                                                           i18nc("@action", "Auto-Rotate"), this);
        connect(d->rotateAutoAction, SIGNAL(triggered()), m_canvas, SLOT(slotRotateAuto()));
        connect(d->rotateAutoAction, SIGNAL(triggered()), this, SLOT(slotRotateAutoIntoQue()));
        ac->addAction(QLatin1String("editorwindow_transform_rotateauto"), d->rotateAutoAction);
        ac->setDefaultShortcut(d->rotateAutoAction, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Up));
        d->rotateAutoAction->setEnabled(false);
    }

    m_showBarAction = thumbBar()->getToggleAction(this);
    ac->addAction(QLatin1String("editorwindow_showthumbs"), m_showBarAction);
    ac->setDefaultShortcut(m_showBarAction, QKeySequence(Qt::CTRL | Qt::Key_T));

    // Provides a menu entry that allows showing/hiding the toolbar(s)

    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar

    createStandardStatusBarAction();

    // Standard 'Configure' menu actions

    createSettingsActions();

    // ---------------------------------------------------------------------------------

    ThemeManager::instance()->registerThemeActions(this);

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    // -- Keyboard-only actions --------------------------------------------------------

    QAction* const altBackwardAction = new QAction(i18nc("@action", "Previous Image"), this);
    ac->addAction(QLatin1String("editorwindow_backward_shift_space"), altBackwardAction);
    ac->setDefaultShortcut(altBackwardAction, QKeySequence(Qt::SHIFT | Qt::Key_Space));
    connect(altBackwardAction, SIGNAL(triggered()), this, SLOT(slotBackward()));

    // -- Tool control actions ---------------------------------------------------------

    m_applyToolAction = new QAction(QIcon::fromTheme(QLatin1String("dialog-ok-apply")),
                                    i18nc("@action", "OK"), this);
    ac->addAction(QLatin1String("editorwindow_applytool"), m_applyToolAction);
    ac->setDefaultShortcut(m_applyToolAction, QKeySequence(Qt::Key_Return));
    connect(m_applyToolAction, SIGNAL(triggered()), this, SLOT(slotApplyTool()));

    m_closeToolAction = new QAction(QIcon::fromTheme(QLatin1String("dialog-cancel")),
                                    i18nc("@action", "Cancel"), this);
    ac->addAction(QLatin1String("editorwindow_closetool"), m_closeToolAction);
    ac->setDefaultShortcut(m_closeToolAction, QKeySequence(Qt::Key_Escape));
    connect(m_closeToolAction, SIGNAL(triggered()), this, SLOT(slotCloseTool()));

    toggleNonDestructiveActions();
    toggleToolActions();
}

void EditorWindow::setupStatusBar()
{
    m_nameLabel  = new StatusProgressBar(statusBar());
    m_nameLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_nameLabel, 100);

    d->infoLabel = new DAdjustableLabel(statusBar());
    d->infoLabel->setAdjustedText(i18nc("@label", "No selection"));
    d->infoLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(d->infoLabel, 100);
    d->infoLabel->setToolTip(i18nc("@info", "Information about current image selection"));

    m_resLabel   = new DAdjustableLabel(statusBar());
    m_resLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_resLabel, 100);
    m_resLabel->setToolTip(i18nc("@info", "Information about image size"));

    d->zoomBar   = new DZoomBar(statusBar());
    d->zoomBar->setZoomToFitAction(d->zoomFitToWindowAction);
    d->zoomBar->setZoomTo100Action(d->zoomTo100percents);
    d->zoomBar->setZoomPlusAction(d->zoomPlusAction);
    d->zoomBar->setZoomMinusAction(d->zoomMinusAction);
    d->zoomBar->setBarMode(DZoomBar::PreviewZoomCtrl);
    statusBar()->addPermanentWidget(d->zoomBar);

    connect(d->zoomBar, SIGNAL(signalZoomSliderChanged(int)),
            m_stackView, SLOT(slotZoomSliderChanged(int)));

    connect(d->zoomBar, SIGNAL(signalZoomValueEdited(double)),
            m_stackView, SLOT(setZoomFactor(double)));

    d->previewToolBar = new PreviewToolBar(statusBar());
    d->previewToolBar->registerMenuActionGroup(this);
    d->previewToolBar->setEnabled(false);
    statusBar()->addPermanentWidget(d->previewToolBar);

    connect(d->previewToolBar, SIGNAL(signalPreviewModeChanged(int)),
            this, SIGNAL(signalPreviewModeChanged(int)));

    QWidget* const buttonsBox      = new QWidget(statusBar());
    QHBoxLayout* const hlay        = new QHBoxLayout(buttonsBox);
    QButtonGroup* const buttonsGrp = new QButtonGroup(buttonsBox);
    buttonsGrp->setExclusive(false);

    d->underExposureIndicator = new QToolButton(buttonsBox);
    d->underExposureIndicator->setDefaultAction(d->viewUnderExpoAction);
    d->underExposureIndicator->setFocusPolicy(Qt::NoFocus);

    d->overExposureIndicator  = new QToolButton(buttonsBox);
    d->overExposureIndicator->setDefaultAction(d->viewOverExpoAction);
    d->overExposureIndicator->setFocusPolicy(Qt::NoFocus);

    d->cmViewIndicator        = new QToolButton(buttonsBox);
    d->cmViewIndicator->setDefaultAction(d->viewCMViewAction);
    d->cmViewIndicator->setFocusPolicy(Qt::NoFocus);

    buttonsGrp->addButton(d->underExposureIndicator);
    buttonsGrp->addButton(d->overExposureIndicator);
    buttonsGrp->addButton(d->cmViewIndicator);

    hlay->setSpacing(0);
    hlay->setContentsMargins(QMargins());
    hlay->addWidget(d->underExposureIndicator);
    hlay->addWidget(d->overExposureIndicator);
    hlay->addWidget(d->cmViewIndicator);

    statusBar()->addPermanentWidget(buttonsBox);
}

void EditorWindow::slotAboutToShowUndoMenu()
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    m_undoAction->popupMenu()->clear();

#else

    m_undoAction->menu()->clear();

#endif

    QStringList titles = m_canvas->interface()->getUndoHistory();

    for (int i = 0 ; i < titles.size() ; ++i)
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        QAction* const action = m_undoAction->popupMenu()->addAction(titles.at(i));

#else

        QAction* const action = m_undoAction->menu()->addAction(titles.at(i));

#endif

        int id                = i + 1;

        connect(action, &QAction::triggered,
                this, [this, id]()
            {
                m_canvas->slotUndo(id);
            }
        );
    }
}

void EditorWindow::slotAboutToShowRedoMenu()
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    m_redoAction->popupMenu()->clear();

#else

    m_redoAction->menu()->clear();

#endif

    QStringList titles = m_canvas->interface()->getRedoHistory();

    for (int i = 0 ; i < titles.size() ; ++i)
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        QAction* const action = m_redoAction->popupMenu()->addAction(titles.at(i));

#else

        QAction* const action = m_redoAction->menu()->addAction(titles.at(i));

#endif

        int id                = i + 1;

        connect(action, &QAction::triggered,
                this, [this, id]()
            {
                m_canvas->slotRedo(id);
            }
        );
    }
}

void EditorWindow::slotIncreaseZoom()
{
    m_stackView->increaseZoom();
}

void EditorWindow::slotDecreaseZoom()
{
    m_stackView->decreaseZoom();
}

void EditorWindow::slotToggleFitToWindow()
{
    d->zoomPlusAction->setEnabled(true);
    d->zoomBar->setEnabled(true);
    d->zoomMinusAction->setEnabled(true);
    m_stackView->toggleFitToWindow();
}

void EditorWindow::slotFitToSelect()
{
    d->zoomPlusAction->setEnabled(true);
    d->zoomBar->setEnabled(true);
    d->zoomMinusAction->setEnabled(true);
    m_stackView->fitToSelect();
}

void EditorWindow::slotZoomTo100Percents()
{
    d->zoomPlusAction->setEnabled(true);
    d->zoomBar->setEnabled(true);
    d->zoomMinusAction->setEnabled(true);
    m_stackView->zoomTo100Percent();
}

void EditorWindow::slotZoomChanged(bool isMax, bool isMin, double zoom)
{
/*
    qCDebug(DIGIKAM_GENERAL_LOG) << "EditorWindow::slotZoomChanged";
*/
    d->zoomPlusAction->setEnabled(!isMax);
    d->zoomMinusAction->setEnabled(!isMin);

    double zmin = m_stackView->zoomMin();
    double zmax = m_stackView->zoomMax();
    d->zoomBar->setZoom(zoom, zmin, zmax);
}

void EditorWindow::slotToggleOffFitToWindow()
{
    d->zoomFitToWindowAction->blockSignals(true);
    d->zoomFitToWindowAction->setChecked(false);
    d->zoomFitToWindowAction->blockSignals(false);
}

void EditorWindow::readStandardSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());

    // Restore full screen Mode

    readFullScreenSettings(group);

    // Restore Auto zoom action

    bool autoZoom = group.readEntry(d->configAutoZoomEntry, true);

    if (autoZoom)
    {
        d->zoomFitToWindowAction->trigger();
    }

    slotSetUnderExposureIndicator(group.readEntry(d->configUnderExposureIndicatorEntry, false));
    slotSetOverExposureIndicator(group.readEntry(d->configOverExposureIndicatorEntry, false));
    d->previewToolBar->readSettings(group);
}

void EditorWindow::applyStandardSettings()
{
    applyColorManagementSettings();
    d->toolIface->updateICCSettings();

    applyIOSettings();

    // -- GUI Settings -------------------------------------------------------

    KConfigGroup group = KSharedConfig::openConfig()->group(configGroupName());

    m_splitter->restoreState(group);
    readFullScreenSettings(group);

    slotThemeChanged();

    // -- Exposure Indicators Settings ---------------------------------------

    d->exposureSettings->underExposureColor    = group.readEntry(d->configUnderExposureColorEntry,    QColor(Qt::white));
    d->exposureSettings->underExposurePercent  = group.readEntry(d->configUnderExposurePercentsEntry, 1.0);
    d->exposureSettings->overExposureColor     = group.readEntry(d->configOverExposureColorEntry,     QColor(Qt::black));
    d->exposureSettings->overExposurePercent   = group.readEntry(d->configOverExposurePercentsEntry,  1.0);
    d->exposureSettings->exposureIndicatorMode = group.readEntry(d->configExpoIndicatorModeEntry,     true);
    d->toolIface->updateExposureSettings();

    // -- Metadata Settings --------------------------------------------------

    MetaEngineSettingsContainer writeSettings = MetaEngineSettings::instance()->settings();
    m_setExifOrientationTag                   = writeSettings.exifSetOrientation;
    m_canvas->setExifOrient(writeSettings.exifRotate);
}

void EditorWindow::applyIOSettings()
{
    // -- JPEG, PNG, TIFF, JPEG2000, PGF files format settings ----------------

    KConfigGroup group = KSharedConfig::openConfig()->group(configGroupName());

    m_IOFileSettings->JPEGCompression     = DImgLoader::convertCompressionForLibJpeg(group.readEntry(d->configJpegCompressionEntry, 75));

    // Medium subsampling

    m_IOFileSettings->JPEGSubSampling     = group.readEntry(d->configJpegSubSamplingEntry, 1);

    m_IOFileSettings->PNGCompression      = DImgLoader::convertCompressionForLibPng(group.readEntry(d->configPngCompressionEntry,    9));

    // TIFF compression setting.

    m_IOFileSettings->TIFFCompression     = group.readEntry(d->configTiffCompressionEntry,     false);

    // JPEG2000 quality slider settings : 1 - 100

    m_IOFileSettings->JPEG2000Compression = group.readEntry(d->configJpeg2000CompressionEntry, 75);

    // JPEG2000 LossLess setting.

    m_IOFileSettings->JPEG2000LossLess    = group.readEntry(d->configJpeg2000LossLessEntry,    true);

    // PGF quality slider settings : 1 - 9

    m_IOFileSettings->PGFCompression      = group.readEntry(d->configPgfCompressionEntry,      3);

    // PGF LossLess setting.

    m_IOFileSettings->PGFLossLess         = group.readEntry(d->configPgfLossLessEntry,         true);

    // HEIF quality slider settings : 1 - 100

    m_IOFileSettings->HEIFCompression     = group.readEntry(d->configHeifCompressionEntry,     75);

    // HEIF LossLess setting.

    m_IOFileSettings->HEIFLossLess        = group.readEntry(d->configHeifLossLessEntry,        true);

    // JXL quality slider settings : 1 - 99

    m_IOFileSettings->JXLCompression      = group.readEntry(d->configJxlCompressionEntry,      75);

    // JXL LossLess setting.

    m_IOFileSettings->JXLLossLess         = group.readEntry(d->configJxlLossLessEntry,         true);

    // WEBP quality slider settings : 1 - 99

    m_IOFileSettings->WEBPCompression     = group.readEntry(d->configWebpCompressionEntry,     75);

    // WEBP LossLess setting.

    m_IOFileSettings->WEBPLossLess         = group.readEntry(d->configWebpLossLessEntry,       true);

    // AVIF quality slider settings : 1 - 99

    m_IOFileSettings->AVIFCompression     = group.readEntry(d->configAvifCompressionEntry,     75);

    // AVIF LossLess setting.

    m_IOFileSettings->AVIFLossLess        = group.readEntry(d->configAvifLossLessEntry,        true);

    // -- RAW images decoding settings ------------------------------------------------------

    m_IOFileSettings->useRAWImport        = group.readEntry(d->configUseRawImportToolEntry, false);
    m_IOFileSettings->rawImportToolIid    = group.readEntry(d->configRawImportToolIidEntry, QString::fromLatin1("org.kde.digikam.plugin.rawimport.Native"));
    DRawDecoderWidget::readSettings(m_IOFileSettings->rawDecodingSettings.rawPrm, group);

    // Raw Color Management settings:
    // If digiKam Color Management is enabled, no need to correct color of decoded RAW image,
    // else, sRGB color workspace will be used.

    ICCSettingsContainer settings = IccSettings::instance()->settings();

    if (settings.enableCM)
    {
        if (settings.defaultUncalibratedBehavior & ICCSettingsContainer::AutomaticColors)
        {
            m_IOFileSettings->rawDecodingSettings.rawPrm.outputColorSpace = DRawDecoderSettings::CUSTOMOUTPUTCS;
            m_IOFileSettings->rawDecodingSettings.rawPrm.outputProfile    = settings.workspaceProfile;
        }
        else
        {
            m_IOFileSettings->rawDecodingSettings.rawPrm.outputColorSpace = DRawDecoderSettings::RAWCOLOR;
        }
    }
    else
    {
        m_IOFileSettings->rawDecodingSettings.rawPrm.outputColorSpace = DRawDecoderSettings::SRGB;
    }
}

void EditorWindow::applyColorManagementSettings()
{
    ICCSettingsContainer settings = IccSettings::instance()->settings();

    d->toolIface->updateICCSettings();
    m_canvas->setICCSettings(settings);

    d->viewCMViewAction->blockSignals(true);
    d->viewCMViewAction->setEnabled(settings.enableCM);
    d->viewCMViewAction->setChecked(settings.useManagedView);
    setColorManagedViewIndicatorToolTip(settings.enableCM, settings.useManagedView);
    d->viewCMViewAction->blockSignals(false);

    d->viewSoftProofAction->setEnabled(settings.enableCM && !settings.defaultProofProfile.isEmpty());
    d->softProofOptionsAction->setEnabled(settings.enableCM);
}

void EditorWindow::saveStandardSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());

    group.writeEntry(d->configAutoZoomEntry, d->zoomFitToWindowAction->isChecked());
    m_splitter->saveState(group);

    group.writeEntry("Show Thumbbar", thumbBar()->shouldBeVisible());
    group.writeEntry(d->configUnderExposureIndicatorEntry, d->exposureSettings->underExposureIndicator);
    group.writeEntry(d->configOverExposureIndicatorEntry,  d->exposureSettings->overExposureIndicator);
    d->previewToolBar->writeSettings(group);
    config->sync();
}

/**
 * Method used by Editor Tools. Only tools based on imageregionwidget support zooming.
 * TODO: Fix this behavior when editor tool preview widgets will be factored.
 */
void EditorWindow::toggleZoomActions(bool val)
{
    d->zoomMinusAction->setEnabled(val);
    d->zoomPlusAction->setEnabled(val);
    d->zoomTo100percents->setEnabled(val);
    d->zoomFitToWindowAction->setEnabled(val);
    d->zoomBar->setEnabled(val);
}

void EditorWindow::readSettings()
{
    readStandardSettings();
}

void EditorWindow::saveSettings()
{
    saveStandardSettings();
}

void EditorWindow::toggleActions(bool val)
{
    toggleStandardActions(val);
}

bool EditorWindow::actionEnabledState() const
{
    return m_actionEnabledState;
}

void EditorWindow::toggleStandardActions(bool val)
{
    d->zoomFitToSelectAction->setEnabled(val);
    toggleZoomActions(val);

    m_actionEnabledState = val;

    m_forwardAction->setEnabled(val);
    m_backwardAction->setEnabled(val);
    m_firstAction->setEnabled(val);
    m_lastAction->setEnabled(val);
    d->rotateLeftAction->setEnabled(val);
    d->rotateRightAction->setEnabled(val);
    if (d->rotateAutoAction)
    {
        d->rotateAutoAction->setEnabled(val);
    }
    d->flipHorizAction->setEnabled(val);
    d->flipVertAction->setEnabled(val);
    m_fileDeleteAction->setEnabled(val);
    m_saveAsAction->setEnabled(val);
    d->openWithAction->setEnabled(val);
    m_exportAction->setEnabled(val);
    d->selectAllAction->setEnabled(val);
    d->selectNoneAction->setEnabled(val);

    QList<DPluginAction*> actions = DPluginLoader::instance()->pluginsActions(DPluginAction::Generic, this);

    for (DPluginAction* const ac : std::as_const(actions))
    {
        if (ac->actionCategory() != DPluginAction::GenericImport)
        {
            ac->setEnabled(val);
        }
    }

    // these actions are special: They are turned off if val is false,
    // but if val is true, they may be turned on or off.

    if (val)
    {
        // Update actions by retrieving current values

        slotUndoStateChanged();
    }
    else
    {
        m_openVersionAction->setEnabled(false);
        m_revertAction->setEnabled(false);
        m_saveAction->setEnabled(false);
        m_saveCurrentVersionAction->setEnabled(false);
        m_saveNewVersionAction->setEnabled(false);
        m_discardChangesAction->setEnabled(false);
        m_undoAction->setEnabled(false);
        m_redoAction->setEnabled(false);
    }

    // Editor Tools actions

    QList<DPluginAction*> actions2 = DPluginLoader::instance()->pluginsActions(DPluginAction::Editor, this);

    for (DPluginAction* const ac : std::as_const(actions2))
    {
        ac->setEnabled(val);
    }
}

void EditorWindow::toggleNonDestructiveActions()
{
    m_saveAction->setVisible(!m_nonDestructive);
    m_saveAsAction->setVisible(!m_nonDestructive);
    m_revertAction->setVisible(!m_nonDestructive);

    m_openVersionAction->setVisible(m_nonDestructive);
    m_saveCurrentVersionAction->setVisible(m_nonDestructive);
    m_saveNewVersionAction->setVisible(m_nonDestructive);
    m_exportAction->setVisible(m_nonDestructive);
    m_discardChangesAction->setVisible(m_nonDestructive);
}

void EditorWindow::toggleToolActions(EditorTool* tool)
{
    if (tool)
    {
        m_applyToolAction->setText(tool->toolSettings()->button(EditorToolSettings::Ok)->text());
        m_applyToolAction->setIcon(tool->toolSettings()->button(EditorToolSettings::Ok)->icon());
        m_applyToolAction->setToolTip(tool->toolSettings()->button(EditorToolSettings::Ok)->toolTip());

        m_closeToolAction->setText(tool->toolSettings()->button(EditorToolSettings::Cancel)->text());
        m_closeToolAction->setIcon(tool->toolSettings()->button(EditorToolSettings::Cancel)->icon());
        m_closeToolAction->setToolTip(tool->toolSettings()->button(EditorToolSettings::Cancel)->toolTip());
    }

    m_applyToolAction->setVisible(tool);
    m_closeToolAction->setVisible(tool);
}

void EditorWindow::slotLoadingProgress(const QString&, float progress)
{
    m_nameLabel->setProgressValue((int)(progress * 100.0));
}

void EditorWindow::slotSavingProgress(const QString&, float progress)
{
    m_nameLabel->setProgressValue((int)(progress * 100.0));

    if (m_savingProgressDialog)
    {
        m_savingProgressDialog->setValue((int)(progress * 100.0));
    }
}

void EditorWindow::execSavingProgressDialog()
{
    if (m_savingProgressDialog)
    {
        return;
    }

    m_savingProgressDialog = new QProgressDialog(this);
    m_savingProgressDialog->setWindowTitle(i18nc("@title:window", "Saving Image..."));
    m_savingProgressDialog->setLabelText(i18nc("@label", "Please wait for the image to be saved..."));
    m_savingProgressDialog->setAttribute(Qt::WA_DeleteOnClose);
    m_savingProgressDialog->setAutoClose(true);
    m_savingProgressDialog->setMinimumDuration(1000);
    m_savingProgressDialog->setMaximum(100);

    // we must enter a fully modal dialog, no QEventLoop is sufficient for KWin to accept longer waiting times

    m_savingProgressDialog->setModal(true);
    m_savingProgressDialog->exec();
}

bool EditorWindow::promptForOverWrite()
{

    QUrl destination = saveDestinationUrl();

    if (destination.isLocalFile())
    {

        QFileInfo fi(m_canvas->currentImageFilePath());
        QString warnMsg(i18nc("@info", "About to overwrite file \"%1\"\nAre you sure?",
                              QDir::toNativeSeparators(fi.fileName())));

        return (DMessageBox::showContinueCancel(QMessageBox::Warning,
                                                this,
                                                i18nc("@title:window, warning while saving progress", "Warning"),
                                                warnMsg,
                                                QLatin1String("editorWindowSaveOverwrite"))
                ==  QMessageBox::Yes);

    }
    else
    {
        // in this case it will handle the overwrite request

        return true;
    }
}

void EditorWindow::slotUndoStateChanged()
{
    UndoState state = m_canvas->interface()->undoState();

    // RAW conversion qualifies as a "non-undoable" action
    // You can save as new version, but cannot undo or revert

    m_undoAction->setEnabled(state.hasUndo);
    m_redoAction->setEnabled(state.hasRedo);
    m_revertAction->setEnabled(state.hasUndoableChanges);

    m_saveAction->setEnabled(state.hasChanges);
    m_saveCurrentVersionAction->setEnabled(state.hasChanges);
    m_saveNewVersionAction->setEnabled(state.hasChanges);
    m_discardChangesAction->setEnabled(state.hasUndoableChanges);

    m_openVersionAction->setEnabled(hasOriginalToRestore());
}

bool EditorWindow::hasOriginalToRestore()
{
    return m_canvas->interface()->getResolvedInitialHistory().hasOriginalReferredImage();
}

DImageHistory EditorWindow::resolvedImageHistory(const DImageHistory& history)
{
    // simple, database-less version

    DImageHistory r = history;
    QList<DImageHistory::Entry>::iterator it;

    for (it = r.entries().begin() ; it != r.entries().end() ; ++it)
    {
        QList<HistoryImageId>::iterator hit;

        for (hit = it->referredImages.begin() ; hit != it->referredImages.end() ; )
        {
            QFileInfo info(hit->m_filePath + QLatin1Char('/') + hit->m_fileName);

            if (!info.exists())
            {
                hit = it->referredImages.erase(hit);
            }
            else
            {
                ++hit;
            }
        }
    }

    return r;
}

bool EditorWindow::promptUserSave(const QUrl& url, SaveAskMode mode, bool allowCancel)
{
    if (d->currentWindowModalDialog)
    {
        d->currentWindowModalDialog->reject();
    }

    if (m_canvas->interface()->undoState().hasUndoableChanges)
    {
        // if window is minimized, show it

        unminimizeAndActivateWindow();

        bool shallSave    = true;
        bool shallDiscard = false;
        bool newVersion   = false;

        if (mode == AskIfNeeded)
        {
            if (m_nonDestructive)
            {
                if (versionManager()->settings().editorClosingMode == VersionManagerSettings::AutoSave)
                {
                    shallSave = true;
                }
                else
                {
                    QPointer<VersioningPromptUserSaveDialog> dialog = new VersioningPromptUserSaveDialog(this);

                    if (dialog->exec() == QDialog::Rejected)
                    {
                        return false;
                    }

                    shallSave    = dialog->shallSave() || dialog->newVersion();
                    shallDiscard = dialog->shallDiscard();
                    newVersion   = dialog->newVersion();
                }
            }
            else
            {
                QString boxMessage;
                boxMessage = i18nc("@info",
                                   "The image \"%1\" has been modified.\n"
                                   "Do you want to save it?", url.fileName());

                int result;

                if (allowCancel)
                {
                    result = QMessageBox::warning(this, qApp->applicationName(), boxMessage,
                                                  QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
                }
                else
                {
                    result = QMessageBox::warning(this, qApp->applicationName(), boxMessage,
                                                  QMessageBox::Save | QMessageBox::Discard);
                }

                shallSave    = (result == QMessageBox::Save);
                shallDiscard = (result == QMessageBox::Discard);
            }
        }

        if (shallSave)
        {
            bool saving = false;

            switch (mode)
            {
                case AskIfNeeded:
                {
                    if (m_nonDestructive)
                    {
                        if (newVersion)
                        {
                            saving = saveNewVersion();
                        }
                        else
                        {
                            // will know on its own if new version is required

                            saving = saveCurrentVersion();
                        }
                    }
                    else
                    {
                        if      (m_canvas->isReadOnly())
                        {
                            saving = saveAs();
                        }
                        else if (promptForOverWrite())
                        {
                            saving = save();
                        }
                    }

                    break;
                }

                case OverwriteWithoutAsking:
                {
                    if (m_nonDestructive)
                    {
                        if (newVersion)
                        {
                            saving = saveNewVersion();
                        }
                        else
                        {
                            // will know on its own if new version is required

                            saving = saveCurrentVersion();
                        }
                    }
                    else
                    {
                        if (m_canvas->isReadOnly())
                        {
                            saving = saveAs();
                        }
                        else
                        {
                            saving = save();
                        }
                    }

                    break;
                }

                case AlwaysSaveAs:
                {
                    if (m_nonDestructive)
                    {
                        saving = saveNewVersion();
                    }
                    else
                    {
                        saving = saveAs();
                    }

                    break;
                }
            }

            // save and saveAs return false if they were canceled and did not enter saving at all
            // In this case, do not call enterWaitingLoop because quitWaitingloop will not be called.

            if (saving)
            {
                // Waiting for asynchronous image file saving operation running in separate thread.

                m_savingContext.synchronizingState = SavingContext::SynchronousSaving;
                enterWaitingLoop();
                m_savingContext.synchronizingState = SavingContext::NormalSaving;

                return m_savingContext.synchronousSavingResult;
            }
            else
            {
                return false;
            }
        }
        else if (shallDiscard)
        {
            // Discard

            m_saveAction->setEnabled(false);

            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool EditorWindow::promptUserDelete(const QUrl& url)
{
    if (d->currentWindowModalDialog)
    {
        d->currentWindowModalDialog->reject();
    }

    if (m_canvas->interface()->undoState().hasUndoableChanges)
    {
        // if window is minimized, show it

        unminimizeAndActivateWindow();

        QString boxMessage = i18nc("@info",
                                   "The image \"%1\" has been modified.\n"
                                   "All changes will be lost.", url.fileName());

        int result = DMessageBox::showContinueCancel(QMessageBox::Warning,
                                                     this,
                                                     QString(),
                                                     boxMessage);

        if (result == QMessageBox::Cancel)
        {
            return false;
        }
    }

    return true;
}

bool EditorWindow::waitForSavingToComplete()
{
    // avoid reentrancy - return false means we have reentered the loop already.

    if (m_savingContext.synchronizingState == SavingContext::SynchronousSaving)
    {
        return false;
    }

    if (m_savingContext.savingState != SavingContext::SavingStateNone)
    {
        // Waiting for asynchronous image file saving operation running in separate thread.

        m_savingContext.synchronizingState = SavingContext::SynchronousSaving;

        enterWaitingLoop();
        m_savingContext.synchronizingState = SavingContext::NormalSaving;
    }

    return true;
}

void EditorWindow::enterWaitingLoop()
{
    //d->waitingLoop->exec(QEventLoop::ExcludeUserInputEvents);

    execSavingProgressDialog();
}

void EditorWindow::quitWaitingLoop()
{
    //d->waitingLoop->quit();

    if (m_savingProgressDialog)
    {
        m_savingProgressDialog->close();
    }
}

void EditorWindow::slotSelected(bool val)
{
    // Update menu actions.

    d->cropAction->setEnabled(val);
    d->zoomFitToSelectAction->setEnabled(val);
    d->copyAction->setEnabled(val);

    QRect sel = m_canvas->getSelectedArea();

    // Update histogram into sidebar.

    Q_EMIT signalSelectionChanged(sel);

    // Update status bar

    if (val)
    {
        slotSelectionSetText(sel);
    }
    else
    {
        setToolInfoMessage(i18nc("@info", "No selection"));
    }
}

void EditorWindow::slotPrepareToLoad()
{
    // Disable actions as appropriate during loading

    Q_EMIT signalNoCurrentItem();
    unsetCursor();
    m_animLogo->stop();
    toggleActions(false);
    slotUpdateItemInfo();
}

void EditorWindow::slotLoadingStarted(const QString& /*filename*/)
{
    setCursor(Qt::WaitCursor);
    toggleActions(false);
    m_animLogo->start();
    m_nameLabel->setProgressBarMode(StatusProgressBar::ProgressBarMode, i18nc("@label", "Loading:"));
}

void EditorWindow::slotLoadingFinished(const QString& filename, bool success)
{
    m_nameLabel->setProgressBarMode(StatusProgressBar::TextMode);

    // Enable actions as appropriate after loading
    // No need to re-enable image properties sidebar here, it's will be done
    // automatically by a signal from canvas

    toggleActions(success);
    slotUpdateItemInfo();
    unsetCursor();
    m_animLogo->stop();

    if (success)
    {
        colorManage();

        // Set a history which contains all available files as referredImages

        DImageHistory resolved = resolvedImageHistory(m_canvas->interface()->getInitialImageHistory());
        m_canvas->interface()->setResolvedInitialHistory(resolved);
    }
    else
    {
        DNotificationPopup::message(DNotificationPopup::Boxed,
                                    i18nc("@info", "Cannot load \"%1\"", filename),
                                    m_canvas, m_canvas->mapToGlobal(QPoint(30, 30)));
    }
}

void EditorWindow::resetOrigin()
{
    // With versioning, "only" resetting undo history does not work anymore
    // as we calculate undo state based on the initial history stored in the DImg

    resetOriginSwitchFile();
}

void EditorWindow::resetOriginSwitchFile()
{
    DImageHistory resolved = resolvedImageHistory(m_canvas->interface()->getItemHistory());
    m_canvas->interface()->switchToLastSaved(resolved);
}

void EditorWindow::colorManage()
{
    if (!IccSettings::instance()->isEnabled())
    {
        return;
    }

    DImg image = m_canvas->currentImage();

    if (image.isNull())
    {
        return;
    }

    if (!IccManager::needsPostLoadingManagement(image))
    {
        return;
    }

    IccPostLoadingManager manager(image, m_canvas->currentImageFilePath());

    if (!manager.hasValidWorkspace())
    {
        QString message = i18nc("@info", "Cannot open the specified working space profile (\"%1\"). "
                                "No color transformation will be applied. "
                                "Please check the color management "
                                "configuration in digiKam's setup.",
                                IccSettings::instance()->settings().workspaceProfile);
        QMessageBox::information(this, qApp->applicationName(), message);
    }

    // Show dialog and get transform from user choice

    IccTransform trans = manager.postLoadingManage(this);

    // apply transform in thread.
    // Do _not_ test for willHaveEffect() here - there are more side effects when calling this method

    m_canvas->applyTransform(trans);
    slotUpdateItemInfo();
}

void EditorWindow::slotNameLabelCancelButtonPressed()
{
    // If we saving an image...

    if (m_savingContext.savingState != SavingContext::SavingStateNone)
    {
        m_savingContext.abortingSaving = true;
        m_canvas->abortSaving();
    }
}

void EditorWindow::slotFileOriginChanged(const QString&)
{
    // implemented in subclass
}

bool EditorWindow::saveOrSaveAs()
{
    if (m_canvas->isReadOnly())
    {
        return saveAs();
    }

    return save();
}

void EditorWindow::slotSavingStarted(const QString& /*filename*/)
{
    setCursor(Qt::WaitCursor);
    m_animLogo->start();

    // Disable actions as appropriate during saving

    Q_EMIT signalNoCurrentItem();

    toggleActions(false);

    m_nameLabel->setProgressBarMode(StatusProgressBar::CancelProgressBarMode,
                                    i18nc("@label: saving progress on status bar", "Saving:"));
}

void EditorWindow::slotSavingFinished(const QString& filename, bool success)
{
    Q_UNUSED(filename);

    qCDebug(DIGIKAM_GENERAL_LOG) << filename << success
                                 << (m_savingContext.savingState != SavingContext::SavingStateNone);

    // only handle this if we really wanted to save a file...

    if (m_savingContext.savingState != SavingContext::SavingStateNone)
    {
        m_savingContext.executedOperation = m_savingContext.savingState;
        m_savingContext.savingState = SavingContext::SavingStateNone;

        if (!success)
        {
            if (!m_savingContext.abortingSaving)
            {
                QMessageBox::critical(this, qApp->applicationName(),
                                      i18nc("@info", "Failed to save file\n\"%1\"\nto\n\"%2\".",
                                            m_savingContext.destinationURL.fileName(),
                                            m_savingContext.destinationURL.toLocalFile()));
            }

            finishSaving(false);
            return;
        }

        moveFile();

    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Why was slotSavingFinished called if we did not want to save a file?";
    }
}

void EditorWindow::movingSaveFileFinished(bool successful)
{
    if (!successful)
    {
        finishSaving(false);
        return;
    }

    // now that we know the real destination file name, pass it to be recorded in image history

    m_canvas->interface()->setLastSaved(m_savingContext.destinationURL.toLocalFile());

    // remove image from cache since it has changed

    LoadingCacheInterface::fileChanged(m_savingContext.destinationURL.toLocalFile());
    ThumbnailLoadThread::deleteThumbnail(m_savingContext.destinationURL.toLocalFile());

    // restore state of disabled actions. saveIsComplete can start any other task
    // (loading!) which might itself in turn change states

    finishSaving(true);

    switch (m_savingContext.executedOperation)
    {
        case SavingContext::SavingStateNone:
        {
            break;
        }

        case SavingContext::SavingStateSave:
        {
            saveIsComplete();
            break;
        }

        case SavingContext::SavingStateSaveAs:
        {
            saveAsIsComplete();
            break;
        }

        case SavingContext::SavingStateVersion:
        {
            saveVersionIsComplete();
            break;
        }
    }

    // Take all actions necessary to update information and re-enable sidebar

    slotChanged();
}

void EditorWindow::finishSaving(bool success)
{
    m_savingContext.synchronousSavingResult = success;

    delete m_savingContext.saveTempFile;
    m_savingContext.saveTempFile = nullptr;

    // Exit of internal Qt event loop to unlock promptUserSave() method.

    if (m_savingContext.synchronizingState == SavingContext::SynchronousSaving)
    {
        quitWaitingLoop();
    }

    // Enable actions as appropriate after saving

    toggleActions(true);
    unsetCursor();
    m_animLogo->stop();

    m_nameLabel->setProgressBarMode(StatusProgressBar::TextMode);
/*
    if (m_savingProgressDialog)
    {
        m_savingProgressDialog->close();
    }
*/

    // On error, continue using current image

    if (!success)
    {
        /* Why this?
         * m_canvas->switchToLastSaved(m_savingContext.srcURL.toLocalFile());
         */
    }
}

void EditorWindow::setupTempSaveFile(const QUrl& url)
{
    // if the destination url is on local file system, try to set the temp file
    // location to the destination folder, otherwise use a local default

    QString tempDir = url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toLocalFile();

    if (!url.isLocalFile() || tempDir.isEmpty())
    {
        tempDir = QDir::tempPath();
    }

    QFileInfo fi(url.toLocalFile());
    QString suffix = fi.suffix();

    // use magic file extension which tells the digikamalbums ioslave to ignore the file

    m_savingContext.saveTempFile = new SafeTemporaryFile(tempDir + QLatin1String("/EditorWindow-XXXXXX"
                                                                                 ".digikamtempfile.") + suffix);
    m_savingContext.saveTempFile->setAutoRemove(false);

    if (!m_savingContext.saveTempFile->open())
    {
        QMessageBox::critical(this, qApp->applicationName(),
                              i18nc("@info", "Could not open a temporary file in the folder \"%1\": %2 (%3)",
                                    QDir::toNativeSeparators(tempDir), m_savingContext.saveTempFile->errorString(),
                                    m_savingContext.saveTempFile->error()));
        return;
    }

    m_savingContext.saveTempFileName = m_savingContext.saveTempFile->safeFilePath();
    delete m_savingContext.saveTempFile;
    m_savingContext.saveTempFile = nullptr;
}

void EditorWindow::startingSave(const QUrl& url)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "startSaving url = " << url;

    // avoid any reentrancy. Should be impossible anyway since actions will be disabled.

    if (m_savingContext.savingState != SavingContext::SavingStateNone)
    {
        return;
    }

    m_savingContext = SavingContext();

    if (!checkPermissions(url))
    {
        return;
    }

    setupTempSaveFile(url);

    m_savingContext.srcURL             = url;
    m_savingContext.destinationURL     = m_savingContext.srcURL;
    m_savingContext.destinationExisted = true;
    m_savingContext.originalFormat     = m_canvas->currentImageFileFormat();
    m_savingContext.format             = m_savingContext.originalFormat;
    m_savingContext.abortingSaving     = false;
    m_savingContext.savingState        = SavingContext::SavingStateSave;
    m_savingContext.executedOperation  = SavingContext::SavingStateNone;

    m_canvas->interface()->saveAs(m_savingContext.saveTempFileName, m_IOFileSettings,
                                  m_setExifOrientationTag && m_canvas->exifRotated(), m_savingContext.format,
                                  m_savingContext.destinationURL.toLocalFile());
}

bool EditorWindow::showFileSaveDialog(const QUrl& initialUrl, QUrl& newURL)
{
    QString all;
    QStringList list                          = supportedImageMimeTypes(QIODevice::WriteOnly, all);
    QPointer<DFileDialog> imageFileSaveDialog = new DFileDialog(this);
    imageFileSaveDialog->setWindowTitle(i18nc("@title:window", "New Image File Name"));
    imageFileSaveDialog->setDirectoryUrl(initialUrl.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash));
    imageFileSaveDialog->setOption(QFileDialog::DontConfirmOverwrite);
    imageFileSaveDialog->setAcceptMode(QFileDialog::AcceptSave);
    imageFileSaveDialog->setFileMode(QFileDialog::AnyFile);
    imageFileSaveDialog->setNameFilters(list);

    // Restore old settings for the dialog

    KSharedConfig::Ptr config         = KSharedConfig::openConfig();
    KConfigGroup group                = config->group(configGroupName());
    const QString optionLastExtension = QLatin1String("LastSavedImageExtension");
    QString ext                       = group.readEntry(optionLastExtension, "png");

    for (const QString& s : std::as_const(list))
    {
        if (s.contains(QString::fromLatin1("*.%1").arg(ext)))
        {   // cppcheck-suppress useStlAlgorithm
            imageFileSaveDialog->selectNameFilter(s);
            break;
        }
    }

    // Adjust extension of proposed filename

    QString fileName             = initialUrl.fileName();

    if (!fileName.isNull())
    {
        int lastDot              = fileName.lastIndexOf(QLatin1Char('.'));
        QString completeBaseName = (lastDot == -1) ? fileName : fileName.left(lastDot);
        fileName                 = completeBaseName + QLatin1Char('.') + ext;
    }

    if (!fileName.isNull())
    {
        imageFileSaveDialog->selectFile(fileName);
    }

    // Start dialog and check if canceled.

    if (d->currentWindowModalDialog)
    {
        // go application-modal - we will create utter confusion if descending into more than one window-modal dialog

        imageFileSaveDialog->setModal(true);
        imageFileSaveDialog->exec();
    }
    else
    {
        imageFileSaveDialog->setWindowModality(Qt::WindowModal);
        d->currentWindowModalDialog = imageFileSaveDialog;
        imageFileSaveDialog->exec();
        d->currentWindowModalDialog = nullptr;
    }

    qApp->processEvents();

    if (!imageFileSaveDialog || !imageFileSaveDialog->hasAcceptedUrls())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "File Save Dialog rejected";
        delete imageFileSaveDialog;

        return false;
    }

    newURL = imageFileSaveDialog->selectedUrls().constFirst();
    newURL.setPath(QDir::cleanPath(newURL.path()));

    QFileInfo fi(newURL.fileName());

    if (fi.suffix().isEmpty())
    {
        ext = imageFileSaveDialog->selectedNameFilter().section(QLatin1String("*."), 1, 1);
        ext = ext.left(ext.length() - 1);

        if (ext.isEmpty())
        {
            ext = QLatin1String("jpg");
        }

        newURL.setPath(newURL.path() + QLatin1Char('.') + ext);
    }

    delete imageFileSaveDialog;

    qCDebug(DIGIKAM_GENERAL_LOG) << "Writing file to " << newURL;

    //-- Show Settings Dialog ----------------------------------------------

    const QString configShowImageSettingsDialog = QLatin1String("ShowImageSettingsDialog");
    bool showDialog                             = group.readEntry(configShowImageSettingsDialog, true);
    QPointer<FileSaveOptionsBox> options        = new FileSaveOptionsBox();

    if (showDialog && (options->discoverFormat(newURL.fileName(), FileSaveOptionsBox::NONE) != FileSaveOptionsBox::NONE))
    {
        QPointer<FileSaveOptionsDlg> fileSaveOptionsDialog = new FileSaveOptionsDlg(this, options);
        options->setImageFileFormat(newURL.fileName());
        int result;

        if (d->currentWindowModalDialog)
        {
            // go application-modal - we will create utter confusion if descending into more than one window-modal dialog

            fileSaveOptionsDialog->setModal(true);
            result = fileSaveOptionsDialog->exec();
        }
        else
        {
            fileSaveOptionsDialog->setWindowModality(Qt::WindowModal);
            d->currentWindowModalDialog = fileSaveOptionsDialog;
            result                      = fileSaveOptionsDialog->exec();
            d->currentWindowModalDialog = nullptr;
        }

        if ((result != QDialog::Accepted) || !fileSaveOptionsDialog)
        {
            delete fileSaveOptionsDialog;

            return false;
        }

        // Write settings to config

        options->applySettings();

        delete fileSaveOptionsDialog;

        // Options is now also deleted
    }
    else
    {
        delete options;
    }

    // Read settings from config to local container

    applyIOSettings();

    // Select the format to save the image with

    m_savingContext.format = selectValidSavingFormat(newURL);

    if (m_savingContext.format.isNull())
    {
        QMessageBox::critical(this, qApp->applicationName(),
                              i18nc("@info", "Unable to determine the format to save the target image with."));
        return false;
    }

    if (!newURL.isValid())
    {
        QMessageBox::critical(this, qApp->applicationName(),
                              i18nc("@info", "Cannot Save: Found file path \"%1\" is invalid.", newURL.toDisplayString()));
        qCWarning(DIGIKAM_GENERAL_LOG) << "target URL is not valid !";
        return false;
    }

    group.writeEntry(optionLastExtension, m_savingContext.format);
    config->sync();

    return true;
}

QString EditorWindow::selectValidSavingFormat(const QUrl& targetUrl)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Trying to find a saving format from targetUrl = " << targetUrl;

    // Build a list of valid types

    QString all;
    supportedImageMimeTypes(QIODevice::WriteOnly, all);
    qCDebug(DIGIKAM_GENERAL_LOG) << "Qt Offered types: " << all;

    QStringList validTypes = all.split(QLatin1String("*."), Qt::SkipEmptyParts);
    validTypes.replaceInStrings(QLatin1String(" "), QString());

    qCDebug(DIGIKAM_GENERAL_LOG) << "Writable formats: " << validTypes;

    // Determine the format to use the format provided in the filename

    QString suffix;

    if (targetUrl.isLocalFile())
    {
        // For local files QFileInfo can be used

        QFileInfo fi(targetUrl.toLocalFile());
        suffix = fi.suffix();
        qCDebug(DIGIKAM_GENERAL_LOG) << "Possible format from local file: " << suffix;
    }
    else
    {
        // For remote files string manipulation is needed unfortunately

        QString fileName         = targetUrl.fileName();
        const int periodLocation = fileName.lastIndexOf(QLatin1Char('.'));

        if (periodLocation >= 0)
        {
            suffix = fileName.right(fileName.size() - periodLocation - 1);
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "Possible format from remote file: " << suffix;
    }

    if (!suffix.isEmpty() && validTypes.contains(suffix, Qt::CaseInsensitive))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Using format from target url " << suffix;
        return suffix;
    }

    // Another way to determine the format is to use the original file
    {
        QString originalFormat = QString::fromUtf8(QImageReader::imageFormat(m_savingContext.srcURL.toLocalFile()));

        if (validTypes.contains(originalFormat, Qt::CaseInsensitive))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Using format from original file: " << originalFormat;
            return originalFormat;
        }
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "No suitable format found";

    return QString();
}

bool EditorWindow::startingSaveAs(const QUrl& url)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "startSavingAs called";

    if (m_savingContext.savingState != SavingContext::SavingStateNone)
    {
        return false;
    }

    m_savingContext        = SavingContext();
    m_savingContext.srcURL = url;
    QUrl suggested         = m_savingContext.srcURL;

    // Run dialog -------------------------------------------------------------------

    QUrl newURL;

    if (!showFileSaveDialog(suggested, newURL))
    {
        return false;
    }

    // If new and original URL are equal use save() ------------------------------

    QUrl currURL(m_savingContext.srcURL);
    currURL.setPath(QDir::cleanPath(currURL.path()));
    newURL.setPath(QDir::cleanPath(newURL.path()));

    if (currURL.matches(newURL, QUrl::None))
    {
        save();
        return false;
    }

    // Check for overwrite ----------------------------------------------------------

    QFileInfo fi(newURL.toLocalFile());
    m_savingContext.destinationExisted = fi.exists();

    if (m_savingContext.destinationExisted)
    {
        if (!checkOverwrite(newURL))
        {
            return false;
        }

        // There will be two message boxes if the file is not writable.
        // This may be controversial, and it may be changed, but it was a deliberate decision.

        if (!checkPermissions(newURL))
        {
            return false;
        }
    }

    // Now do the actual saving -----------------------------------------------------

    setupTempSaveFile(newURL);

    m_savingContext.destinationURL = newURL;
    m_savingContext.originalFormat = m_canvas->currentImageFileFormat();
    m_savingContext.savingState    = SavingContext::SavingStateSaveAs;
    m_savingContext.executedOperation = SavingContext::SavingStateNone;
    m_savingContext.abortingSaving = false;

    // In any case, destructive (Save as) or non (Export), mark as New Version

    m_canvas->interface()->setHistoryIsBranch(true);

    m_canvas->interface()->saveAs(m_savingContext.saveTempFileName, m_IOFileSettings,
                                  m_setExifOrientationTag && m_canvas->exifRotated(),
                                  m_savingContext.format.toLower(),
                                  m_savingContext.destinationURL.toLocalFile());

    return true;
}

bool EditorWindow::startingSaveCurrentVersion(const QUrl& url)
{
    return startingSaveVersion(url, false, false, QString());
}

bool EditorWindow::startingSaveNewVersion(const QUrl& url)
{
    return startingSaveVersion(url, true, false, QString());
}

bool EditorWindow::startingSaveNewVersionAs(const QUrl& url)
{
    return startingSaveVersion(url, true, true, QString());
}

bool EditorWindow::startingSaveNewVersionInFormat(const QUrl& url, const QString& format)
{
    return startingSaveVersion(url, true, false, format);
}

VersionFileOperation EditorWindow::saveVersionFileOperation(const QUrl& url, bool fork)
{
    DImageHistory resolvedHistory = m_canvas->interface()->getResolvedInitialHistory();
    DImageHistory history         = m_canvas->interface()->getItemHistory();

    VersionFileInfo currentName(url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toLocalFile(),
                                url.fileName(), m_canvas->currentImageFileFormat());

    return versionManager()->operation(fork ? VersionManager::NewVersionName : VersionManager::CurrentVersionName,
                                       currentName, resolvedHistory, history);
}

VersionFileOperation EditorWindow::saveAsVersionFileOperation(const QUrl& url, const QUrl& saveUrl, const QString& format)
{
    DImageHistory resolvedHistory = m_canvas->interface()->getResolvedInitialHistory();
    DImageHistory history         = m_canvas->interface()->getItemHistory();

    VersionFileInfo currentName(url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toLocalFile(),
                                url.fileName(), m_canvas->currentImageFileFormat());

    VersionFileInfo saveLocation(saveUrl.adjusted(QUrl::RemoveFilename).toLocalFile(),
                                 saveUrl.fileName(), format);

    return versionManager()->operationNewVersionAs(currentName, saveLocation, resolvedHistory, history);
}

VersionFileOperation EditorWindow::saveInFormatVersionFileOperation(const QUrl& url, const QString& format)
{
    DImageHistory resolvedHistory = m_canvas->interface()->getResolvedInitialHistory();
    DImageHistory history         = m_canvas->interface()->getItemHistory();

    VersionFileInfo currentName(url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toLocalFile(),
                                url.fileName(), m_canvas->currentImageFileFormat());

    return versionManager()->operationNewVersionInFormat(currentName, format, resolvedHistory, history);
}

bool EditorWindow::startingSaveVersion(const QUrl& url, bool fork, bool saveas, const QString& format)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Saving image" << url << "non-destructive, new version:"
                                 << fork << ", saveas:" << saveas << "format:" << format;

    if (m_savingContext.savingState != SavingContext::SavingStateNone)
    {
        return false;
    }

    m_savingContext                      = SavingContext();
    m_savingContext.versionFileOperation = saveVersionFileOperation(url, fork);
    m_canvas->interface()->setHistoryIsBranch(fork);

    if      (saveas)
    {
        QUrl suggested = m_savingContext.versionFileOperation.saveFile.fileUrl();
        QUrl selectedUrl;

        if (!showFileSaveDialog(suggested, selectedUrl))
        {
            return false;
        }

        m_savingContext.versionFileOperation = saveAsVersionFileOperation(url, selectedUrl, m_savingContext.format);
    }
    else if (!format.isNull())
    {
        m_savingContext.versionFileOperation = saveInFormatVersionFileOperation(url, format);
    }

    const QUrl newURL = m_savingContext.versionFileOperation.saveFile.fileUrl();
    qCDebug(DIGIKAM_GENERAL_LOG) << "Writing file to " << newURL;

    if (!newURL.isValid())
    {
        QMessageBox::critical(this, qApp->applicationName(),
                              i18nc("@info",
                                    "Cannot save file \"%1\" to "
                                    "the suggested version file name \"%2\"",
                                    url.fileName(),
                                    newURL.fileName()));
        qCWarning(DIGIKAM_GENERAL_LOG) << "target URL is not valid !";

        return false;
    }

    QFileInfo fi(newURL.toLocalFile());
    m_savingContext.destinationExisted = fi.exists();

    // Check for overwrite (saveAs only) --------------------------------------------

    if (m_savingContext.destinationExisted)
    {
        // So, should we refuse to overwrite the original?
        // It's a frontal crash against non-destructive principles.
        // It is tempting to refuse, yet I think the user has to decide in the end
/*
        QUrl currURL(m_savingContext.srcURL);
        currURL.cleanPath();
        newURL.cleanPath();

        if (currURL.equals(newURL))
        {
            ...
            return false;
        }
*/
        // check for overwrite, unless the operation explicitly tells us to overwrite

        if (
            !(m_savingContext.versionFileOperation.tasks & VersionFileOperation::Replace) &&
            !checkOverwrite(newURL)
           )
        {
            return false;
        }

        // There will be two message boxes if the file is not writable.
        // This may be controversial, and it may be changed, but it was a deliberate decision.

        if (!checkPermissions(newURL))
        {
            return false;
        }
    }

    setupTempSaveFile(newURL);

    m_savingContext.srcURL             = url;
    m_savingContext.destinationURL     = newURL;
    m_savingContext.originalFormat     = m_canvas->currentImageFileFormat();
    m_savingContext.format             = m_savingContext.versionFileOperation.saveFile.format;
    m_savingContext.abortingSaving     = false;
    m_savingContext.savingState        = SavingContext::SavingStateVersion;
    m_savingContext.executedOperation  = SavingContext::SavingStateNone;

    m_canvas->interface()->saveAs(m_savingContext.saveTempFileName, m_IOFileSettings,
                                  m_setExifOrientationTag && m_canvas->exifRotated(),
                                  m_savingContext.format.toLower(),
                                  m_savingContext.versionFileOperation);

    return true;
}

bool EditorWindow::checkPermissions(const QUrl& url)
{
    // TODO: Check that the permissions can actually be changed
    //       if write permissions are not available.

    QFileInfo fi(url.toLocalFile());

    if (fi.exists() && !fi.isWritable())
    {
        int result = QMessageBox::warning(this, i18nc("@title:window", "Overwrite File?"),
                                          i18nc("@info", "You do not have write permissions "
                                                "for the file named \"%1\". "
                                                "Are you sure you want "
                                                "to overwrite it?",
                                                url.fileName()),
                                          QMessageBox::Save | QMessageBox::Cancel);

        if (result != QMessageBox::Save)
        {
            return false;
        }
    }

    return true;
}

bool EditorWindow::checkOverwrite(const QUrl& url)
{
    int result = QMessageBox::warning(this, i18nc("@title:window", "Overwrite File?"),
                                      i18nc("@info", "A file named \"%1\" already "
                                            "exists. Are you sure you want "
                                            "to overwrite it?",
                                            url.fileName()),
                                      QMessageBox::Save | QMessageBox::Cancel);

    return (result == QMessageBox::Save);
}

bool EditorWindow::moveLocalFile(const QString& org, const QString& dst)
{
    QString sidecarOrg = DMetadata::sidecarPath(org);
    QString source     = m_savingContext.srcURL.toLocalFile();

    if (QFileInfo::exists(sidecarOrg))
    {
        QString sidecarDst = DMetadata::sidecarPath(dst);

        if (!DFileOperations::localFileRename(source, sidecarOrg, sidecarDst))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Failed to move sidecar file";
        }
    }

    if (!DFileOperations::localFileRename(source, org, dst))
    {
        QMessageBox::critical(this, i18nc("@title:window", "Error Saving File"),
                              i18nc("@info", "Failed to overwrite original file"));
        return false;
    }

    return true;
}

void EditorWindow::moveFile()
{
    // Move local file.

    if (m_savingContext.executedOperation == SavingContext::SavingStateVersion)
    {
        // Check if we need to move the current file to an intermediate name

        if (m_savingContext.versionFileOperation.tasks & VersionFileOperation::MoveToIntermediate)
        {
            //qCDebug(DIGIKAM_GENERAL_LOG) << "MoveToIntermediate: Moving " << m_savingContext.srcURL.toLocalFile() << "to"
            //                             << m_savingContext.versionFileOperation.intermediateForLoadedFile.filePath()

            moveLocalFile(m_savingContext.srcURL.toLocalFile(),
                          m_savingContext.versionFileOperation.intermediateForLoadedFile.filePath());

            LoadingCacheInterface::fileChanged(m_savingContext.destinationURL.toLocalFile());
            ThumbnailLoadThread::deleteThumbnail(m_savingContext.destinationURL.toLocalFile());
        }
    }

    bool moveSuccessful = moveLocalFile(m_savingContext.saveTempFileName,
                                        m_savingContext.destinationURL.toLocalFile());

    if (m_savingContext.executedOperation == SavingContext::SavingStateVersion)
    {
        if (moveSuccessful &&
            m_savingContext.versionFileOperation.tasks & VersionFileOperation::SaveAndDelete)
        {
            QFile file(m_savingContext.versionFileOperation.loadedFile.filePath());
            file.remove();
        }
    }

    movingSaveFileFinished(moveSuccessful);
}

void EditorWindow::slotDiscardChanges()
{
    m_canvas->interface()->rollbackToOrigin();
}

void EditorWindow::slotOpenOriginal()
{
    // No-op in this base class
}

void EditorWindow::slotColorManagementOptionsChanged()
{
    applyColorManagementSettings();
    applyIOSettings();
}

void EditorWindow::slotToggleColorManagedView()
{
    if (!IccSettings::instance()->isEnabled())
    {
        return;
    }

    bool cmv = !IccSettings::instance()->settings().useManagedView;
    IccSettings::instance()->setUseManagedView(cmv);
}

void EditorWindow::setColorManagedViewIndicatorToolTip(bool available, bool cmv)
{
    QString tooltip;

    if (available)
    {
        if (cmv)
        {
            tooltip = i18nc("@info", "Color-Managed View is enabled.");
        }
        else
        {
            tooltip = i18nc("@info", "Color-Managed View is disabled.");
        }
    }
    else
    {
        tooltip = i18nc("@info", "Color Management is not configured, so the Color-Managed View is not available.");
    }

    d->cmViewIndicator->setToolTip(tooltip);
}

void EditorWindow::slotSoftProofingOptions()
{
    // Adjusts global settings

    QPointer<SoftProofDialog> dlg = new SoftProofDialog(this);
    dlg->exec();

    d->viewSoftProofAction->setChecked(dlg->shallEnableSoftProofView());
    slotUpdateSoftProofingState();
    delete dlg;
}

void EditorWindow::slotUpdateSoftProofingState()
{
    bool on = d->viewSoftProofAction->isChecked();
    m_canvas->setSoftProofingEnabled(on);
    d->toolIface->updateICCSettings();
}

void EditorWindow::slotSetUnderExposureIndicator(bool on)
{
    d->exposureSettings->underExposureIndicator = on;
    d->toolIface->updateExposureSettings();
    d->viewUnderExpoAction->setChecked(on);
    setUnderExposureToolTip(on);
}

void EditorWindow::setUnderExposureToolTip(bool on)
{
    d->underExposureIndicator->setToolTip(
        on ? i18nc("@info", "Under-Exposure indicator is enabled")
           : i18nc("@info", "Under-Exposure indicator is disabled"));
}

void EditorWindow::slotSetOverExposureIndicator(bool on)
{
    d->exposureSettings->overExposureIndicator = on;
    d->toolIface->updateExposureSettings();
    d->viewOverExpoAction->setChecked(on);
    setOverExposureToolTip(on);
}

void EditorWindow::setOverExposureToolTip(bool on)
{
    d->overExposureIndicator->setToolTip(
        on ? i18nc("@info", "Over-Exposure indicator is enabled")
           : i18nc("@info", "Over-Exposure indicator is disabled"));
}

void EditorWindow::slotSelectionChanged(const QRect& sel)
{
    slotSelectionSetText(sel);
    Q_EMIT signalSelectionChanged(sel);
}

void EditorWindow::slotSelectionSetText(const QRect& sel)
{
    setToolInfoMessage(QString::fromLatin1("(%1, %2) (%3 x %4)")
                       .arg(sel.x())
                       .arg(sel.y())
                       .arg(sel.width())
                       .arg(sel.height()));
}

void EditorWindow::slotComponentsInfo()
{
    LibsInfoDlg* const dlg = new LibsInfoDlg(this);
    dlg->show();
}

void EditorWindow::setToolStartProgress(const QString& toolName)
{
    m_animLogo->start();
    m_nameLabel->setProgressValue(0);
    m_nameLabel->setProgressBarMode(StatusProgressBar::CancelProgressBarMode,
                                    QString::fromUtf8("%1:").arg(toolName));
}

void EditorWindow::setToolProgress(int progress)
{
    m_nameLabel->setProgressValue(progress);
}

void EditorWindow::setToolStopProgress()
{
    m_animLogo->stop();
    m_nameLabel->setProgressValue(0);
    m_nameLabel->setProgressBarMode(StatusProgressBar::TextMode);
    slotUpdateItemInfo();
}

void EditorWindow::slotCloseTool()
{
    if (d->toolIface)
    {
        d->toolIface->slotCloseTool();
    }
}

void EditorWindow::slotApplyTool()
{
    if (d->toolIface)
    {
        d->toolIface->slotApplyTool();
    }
}

void EditorWindow::setPreviewModeMask(int mask)
{
    d->previewToolBar->setPreviewModeMask(mask);
}

PreviewToolBar::PreviewMode EditorWindow::previewMode() const
{
    return d->previewToolBar->previewMode();
}

void EditorWindow::setToolInfoMessage(const QString& txt)
{
    d->infoLabel->setAdjustedText(txt);
}

VersionManager* EditorWindow::versionManager() const
{
    return &d->defaultVersionManager;
}

void EditorWindow::setupSelectToolsAction()
{
    // Create action model

    ActionItemModel* const actionModel = new ActionItemModel(this);
    actionModel->setMode((ActionItemModel::MenuCategoryMode)(ActionItemModel::ToplevelMenuCategory |
                                                             ActionItemModel::SortCategoriesByInsertionOrder));

    // Builtin actions

    QString transformCategory = i18nc("@title Image Transform",       "Transform");

    {
        // See bug #447687

        actionModel->addAction(d->rotateLeftAction,  transformCategory);
        actionModel->addAction(d->rotateRightAction, transformCategory);
        if (d->rotateAutoAction)
        {
            actionModel->addAction(d->rotateAutoAction,  transformCategory);
        }
        actionModel->addAction(d->flipHorizAction,   transformCategory);
        actionModel->addAction(d->flipVertAction,    transformCategory);
        actionModel->addAction(d->cropAction,        transformCategory);

        const auto acs = DPluginLoader::instance()->pluginsActions(DPluginAction::EditorTransform, this);

        for (DPluginAction* const ac : acs)
        {
            actionModel->addAction(ac, transformCategory);
        }
    }

    QString decorateCategory  = i18nc("@title Image Decorate",        "Decorate");

    {
        const auto acs = DPluginLoader::instance()->pluginsActions(DPluginAction::EditorDecorate, this);

        for (DPluginAction* const ac : acs)
        {
            actionModel->addAction(ac, decorateCategory);
        }
    }

    QString effectsCategory   = i18nc("@title Image Effect",          "Effects");

    {
        const auto acs = DPluginLoader::instance()->pluginsActions(DPluginAction::EditorFilters, this);

        for (DPluginAction* const ac : acs)
        {
            actionModel->addAction(ac, effectsCategory);
        }
    }

    QString colorsCategory    = i18nc("@title Image Colors",          "Colors");

    {
        const auto acs = DPluginLoader::instance()->pluginsActions(DPluginAction::EditorColors, this);

        for (DPluginAction* const ac : acs)
        {
            actionModel->addAction(ac, colorsCategory);
        }
    }

    QString enhanceCategory   = i18nc("@title Image Enhance",         "Enhance");

    {
        const auto acs = DPluginLoader::instance()->pluginsActions(DPluginAction::EditorEnhance, this);

        for (DPluginAction* const ac : acs)
        {
            actionModel->addAction(ac, enhanceCategory);
        }
    }

    QString postCategory      = i18nc("@title Post Processing Tools", "Post-Processing");

    {
        const auto acs = DPluginLoader::instance()->pluginsActions(DPluginAction::GenericTool, this);

        for (DPluginAction* const ac : acs)
        {
            actionModel->addAction(ac, postCategory);
        }
    }

    {
        const auto acs = DPluginLoader::instance()->pluginsActions(DPluginAction::EditorFile, this);

        for (DPluginAction* const ac : acs)
        {
            actionModel->addAction(ac, postCategory);
        }
    }

    {
        const auto acs = DPluginLoader::instance()->pluginsActions(DPluginAction::GenericMetadata, this);

        for (DPluginAction* const ac : acs)
        {
            actionModel->addAction(ac, postCategory);
        }
    }

    QString exportCategory    = i18nc("@title Export Tools",          "Export");

    {
        const auto acs = DPluginLoader::instance()->pluginsActions(DPluginAction::GenericExport, this);

        for (DPluginAction* const ac : acs)
        {
            actionModel->addAction(ac, exportCategory);
        }
    }

    QString importCategory    = i18nc("@title Import Tools",          "Import");

    {
        const auto acs = DPluginLoader::instance()->pluginsActions(DPluginAction::GenericImport, this);

        for (DPluginAction* const ac : acs)
        {
            actionModel->addAction(ac, importCategory);
        }
    }

    // setup categorized view

    DCategorizedSortFilterProxyModel* const filterModel = actionModel->createFilterModel();

    ActionCategorizedView* const selectToolsActionView  = new ActionCategorizedView;
    selectToolsActionView->setIconSize(QSize(22, 22));
    selectToolsActionView->setModel(filterModel);
    selectToolsActionView->setupIconMode();
    selectToolsActionView->adjustGridSize();

    connect(selectToolsActionView, SIGNAL(clicked(QModelIndex)),
            actionModel, SLOT(trigger(QModelIndex)));

    EditorToolIface::editorToolIface()->setToolsIconView(selectToolsActionView);
}

void EditorWindow::slotThemeChanged()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());

    if (!group.readEntry(d->configUseThemeBackgroundColorEntry, true))
    {
        m_bgColor = group.readEntry(d->configBackgroundColorEntry, QColor(Qt::black));
    }
    else
    {
        m_bgColor = palette().color(QPalette::Base);
    }

    m_canvas->setBackgroundBrush(QBrush(m_bgColor));
    d->toolIface->themeChanged();
}

void EditorWindow::addAction2ContextMenu(const QString& actionName, bool addDisabled)
{
    if (!m_contextMenu)
    {
        return;
    }

    QAction* const action = actionCollection()->action(actionName);

    if (action && (action->isEnabled() || addDisabled))
    {
        m_contextMenu->addAction(action);
    }
}

void EditorWindow::showSideBars(bool visible)
{
    if (visible)
    {
        rightSideBar()->restore(QList<QWidget*>() << thumbBar(), d->fullscreenSizeBackup);
    }
    else
    {
        // See bug #166472, a simple backup()/restore() will hide non-sidebar splitter child widgets
        // in horizontal mode thumbbar wont be member of the splitter, it is just ignored then

        rightSideBar()->backup(QList<QWidget*>() << thumbBar(), &d->fullscreenSizeBackup);
    }
}

void EditorWindow::slotToggleRightSideBar()
{
    rightSideBar()->isExpanded() ? rightSideBar()->shrink()
                                 : rightSideBar()->expand();
}

void EditorWindow::slotPreviousRightSideBarTab()
{
    rightSideBar()->activePreviousTab();
}

void EditorWindow::slotNextRightSideBarTab()
{
    rightSideBar()->activeNextTab();
}

void EditorWindow::showThumbBar(bool visible)
{
    visible ? thumbBar()->restoreVisibility()
            : thumbBar()->hide();
}

bool EditorWindow::thumbbarVisibility() const
{
    return thumbBar()->isVisible();
}

void EditorWindow::customizedFullScreenMode(bool set)
{
    set ? m_canvas->setBackgroundBrush(QBrush(Qt::black))
        : m_canvas->setBackgroundBrush(QBrush(m_bgColor));

    showStatusBarAction()->setEnabled(!set);
    toolBarMenuAction()->setEnabled(!set);
    showMenuBarAction()->setEnabled(!set);
    m_showBarAction->setEnabled(!set);
}

void EditorWindow::addServicesMenuForUrl(const QUrl& url)
{

#ifdef HAVE_KIO

    KService::List offers = DServiceMenu::servicesForOpenWith(QList<QUrl>() << url);

    qCDebug(DIGIKAM_GENERAL_LOG) << offers.count() << " services found to open " << url;

    if (m_servicesMenu)
    {
        delete m_servicesMenu;
        m_servicesMenu = nullptr;
    }

    if (m_serviceAction)
    {
        delete m_serviceAction;
        m_serviceAction = nullptr;
    }

    if (!offers.isEmpty())
    {
        m_servicesMenu = new QMenu(this);

        QAction* const serviceAction = m_servicesMenu->menuAction();
        serviceAction->setText(i18nc("@action", "Open With"));

        for (const KService::Ptr& service : std::as_const(offers))
        {
            QString name          = service->name().replace(QLatin1Char('&'), QLatin1String("&&"));
            QAction* const action = m_servicesMenu->addAction(name);
            action->setIcon(QIcon::fromTheme(service->icon()));
            action->setData(service->name());
            d->servicesMap[name]  = service;
        }

        m_servicesMenu->addSeparator();
        m_servicesMenu->addAction(i18nc("@action: open with other application", "Other..."));

        m_contextMenu->addAction(serviceAction);

        connect(m_servicesMenu, SIGNAL(triggered(QAction*)),
                this, SLOT(slotOpenWith(QAction*)));
    }
    else
    {
        m_serviceAction = new QAction(i18nc("@action: open with desktop application", "Open With..."), this);
        m_contextMenu->addAction(m_serviceAction);

        connect(m_servicesMenu, SIGNAL(triggered()),
                this, SLOT(slotOpenWith()));
    }

#else

    QList<DServiceInfo> offers = DServiceMenu::servicesForOpen(QList<QUrl>() << url);

    qCDebug(DIGIKAM_GENERAL_LOG) << offers.count() << " services found to open " << url;

    if (m_servicesMenu)
    {
        delete m_servicesMenu;
        m_servicesMenu = nullptr;
    }

    if (m_serviceAction)
    {
        delete m_serviceAction;
        m_serviceAction = nullptr;
    }

    if (!offers.isEmpty())
    {
        m_servicesMenu = new QMenu(this);

        QAction* const serviceAction = m_servicesMenu->menuAction();
        serviceAction->setText(i18nc("@action", "Open With"));

        for (const DServiceInfo& sinfo : std::as_const(offers))
        {
            QAction* const action = m_servicesMenu->addAction(sinfo.name);
            action->setIcon(DServiceMenu::getIconFromService(sinfo));
            action->setData(sinfo.name);
            d->newServicesMap[sinfo.name] = sinfo;
        }

        m_contextMenu->addAction(serviceAction);

        connect(m_servicesMenu, SIGNAL(triggered(QAction*)),
                this, SLOT(slotOpenWith(QAction*)));
    }

#endif // HAVE_KIO

}

void EditorWindow::openWith(const QUrl& url, QAction* action)
{
    QString name = action ? action->data().toString() : QString();

#ifdef HAVE_KIO

    KService::Ptr service;

    if (name.isEmpty())
    {
        QPointer<KOpenWithDialog> dlg = new KOpenWithDialog(QList<QUrl>() << url);

        if (dlg->exec() != KOpenWithDialog::Accepted)
        {
            delete dlg;
            return;
        }

        service = dlg->service();

        if (!service)
        {
            // User entered a custom command

            if (!dlg->text().isEmpty())
            {
                DServiceMenu::runFiles(dlg->text(), QList<QUrl>() << url);
            }

            delete dlg;
            return;
        }

        delete dlg;
    }
    else
    {
        service = d->servicesMap[name];
    }

    DServiceMenu::runFiles(service, QList<QUrl>() << url);

#else

    if (name.isEmpty())
    {
        return;
    }

    DServiceMenu::runFiles(d->newServicesMap[name], QList<QUrl>() << url);

#endif // HAVE_KIO

}

void EditorWindow::loadTool(EditorTool* const tool)
{
    EditorToolIface::editorToolIface()->loadTool(tool);

    connect(tool, SIGNAL(okClicked()),
            this, SLOT(slotToolDone()));

    connect(tool, SIGNAL(cancelClicked()),
            this, SLOT(slotToolDone()));
}

void EditorWindow::slotToolDone()
{
    EditorToolIface::editorToolIface()->unLoadTool();
}

void EditorWindow::slotRotateLeftIntoQue()
{
    m_transformQue.append(TransformType::RotateLeft);
}

void EditorWindow::slotRotateRightIntoQue()
{
    m_transformQue.append(TransformType::RotateRight);
}

void EditorWindow::slotRotateAutoIntoQue()
{
    m_transformQue.append(TransformType::RotateAuto);
}

void EditorWindow::slotFlipHIntoQue()
{
    m_transformQue.append(TransformType::FlipHorizontal);
}

void EditorWindow::slotFlipVIntoQue()
{
    m_transformQue.append(TransformType::FlipVertical);
}

void EditorWindow::registerExtraPluginsActions(QString& dom)
{
    DPluginLoader* const dpl      = DPluginLoader::instance();
    dpl->registerEditorPlugins(this);
    dpl->registerRawImportPlugins(this);

    QList<DPluginAction*> actions = dpl->pluginsActions(DPluginAction::Editor, this);

    for (DPluginAction* const ac : std::as_const(actions))
    {
        actionCollection()->addActions(QList<QAction*>() << ac);
        actionCollection()->setDefaultShortcuts(ac, ac->shortcuts());
    }

    dom.replace(QLatin1String("<!-- _DPLUGINS_EDITOR_FILES_ACTIONS_ -->"),     dpl->pluginXmlSections(DPluginAction::EditorFile,     this));
    dom.replace(QLatin1String("<!-- _DPLUGINS_EDITOR_COLORS_ACTIONS_ -->"),    dpl->pluginXmlSections(DPluginAction::EditorColors,   this));
    dom.replace(QLatin1String("<!-- _DPLUGINS_EDITOR_ENHANCE_ACTIONS_ -->"),   dpl->pluginXmlSections(DPluginAction::EditorEnhance,  this));

    QString transformActions;
    transformActions.append(QLatin1String("<Action name=\"editorwindow_transform_rotateauto\" />\n"));
    transformActions.append(QLatin1String("<Action name=\"editorwindow_transform_rotateleft\" />\n"));
    transformActions.append(QLatin1String("<Action name=\"editorwindow_transform_rotateright\" />\n"));
    transformActions.append(QLatin1String("<Action name=\"editorwindow_transform_fliphoriz\" />\n"));
    transformActions.append(QLatin1String("<Action name=\"editorwindow_transform_flipvert\" />\n"));
    transformActions.append(QLatin1String("<Action name=\"editorwindow_transform_crop\" />\n"));
    transformActions.append(QLatin1String("<Separator/>\n"));
    transformActions.append(dpl->pluginXmlSections(DPluginAction::EditorTransform, this));

    dom.replace(QLatin1String("<!-- _DPLUGINS_EDITOR_TRANSFORM_ACTIONS_ -->"), transformActions);
    dom.replace(QLatin1String("<!-- _DPLUGINS_EDITOR_DECORATE_ACTIONS_ -->"),  dpl->pluginXmlSections(DPluginAction::EditorDecorate, this));
    dom.replace(QLatin1String("<!-- _DPLUGINS_EDITOR_FILTERS_ACTIONS_ -->"),   dpl->pluginXmlSections(DPluginAction::EditorFilters,  this));
}

} // namespace Digikam

#include "moc_editorwindow.cpp"
