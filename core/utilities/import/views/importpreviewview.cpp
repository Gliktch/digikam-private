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

#include "importpreviewview.h"

// Qt includes

#include <QMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QToolBar>
#include <QMenu>
#include <QApplication>
#include <QIcon>
#include <QStyle>
#include <QGridLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimgpreviewitem.h"
#include "fileactionmngr.h"
#include "importcontextmenu.h"
#include "singlephotopreviewlayout.h"
#include "thememanager.h"
#include "importsettings.h"
#include "previewsettings.h"
#include "paniconwidget.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImportPreviewViewItem : public DImgPreviewItem
{
    Q_OBJECT

public:

    explicit ImportPreviewViewItem(ImportPreviewView* const view)
        : m_view (view)
    {
        setAcceptHoverEvents(true);
    }

/*
    FIXME
    void setFaceGroup(FaceGroup* group)
    {
       m_group = group;
    }
*/

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override
    {
        m_view->showContextMenu(m_info, event);
    }

    void setCamItemInfo(const CamItemInfo& info)
    {
        m_info = info;

        if (!info.isNull())
        {
            setPath(info.url().toLocalFile(), true);
        }
    }

    void hoverEnterEvent(QGraphicsSceneHoverEvent* e) override
    {
        Q_UNUSED(e)
/*
        FIXME
        m_group->itemHoverEnterEvent(e);
*/
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent* e) override
    {
        Q_UNUSED(e)
/*
        FIXME:
        m_group->itemHoverLeaveEvent(e);
*/
    }

    void hoverMoveEvent(QGraphicsSceneHoverEvent* e) override
    {
        Q_UNUSED(e)
/*
        FIXME
        m_group->itemHoverMoveEvent(e);
*/
    }

    CamItemInfo camItemInfo() const
    {
        return m_info;
    }

protected:

    ImportPreviewView*  m_view  = nullptr;
/*
    FIXME
    FaceGroup*          m_group = nullptr;
*/
    CamItemInfo         m_info;
};

// ---------------------------------------------------------------------

class Q_DECL_HIDDEN ImportPreviewView::Private
{
public:

    Private() = default;

    bool                    fullSize            = false;
    double                  scale               = 1.0;
    bool                    isValid             = false;

    ImportPreviewView::Mode mode                = ImportPreviewView::IconViewPreview;

    ImportPreviewViewItem*  item                = nullptr;

    QAction*                escapePreviewAction = nullptr;
    QAction*                prevAction          = nullptr;
    QAction*                nextAction          = nullptr;
    QAction*                rotLeftAction       = nullptr;
    QAction*                rotRightAction      = nullptr;

    QToolBar*               toolBar             = nullptr;
/*
    FIXME
    bool                    peopleTagsShown     = false;
    QAction*                peopleToggleAction  = nullptr;
    QAction*                addPersonAction     = nullptr;
    QAction*                forgetFacesAction   = nullptr;
    FaceGroup*              faceGroup           = nullptr;
*/
};

ImportPreviewView::ImportPreviewView(QWidget* const parent, Mode mode)
    : GraphicsDImgView(parent),
      d               (new Private)
{
    d->mode = mode;
    d->item = new ImportPreviewViewItem(this);
    setItem(d->item);

/*
    FIXME
    d->faceGroup = new FaceGroup(this);
    d->faceGroup->setShowOnHover(true);
    d->item->setFaceGroup(d->faceGroup);
*/
    connect(d->item, SIGNAL(loaded()),
            this, SLOT(camItemLoaded()));

    connect(d->item, SIGNAL(loadingFailed()),
            this, SLOT(camItemLoadingFailed()));

    // set default zoom

    layout()->fitToWindow();

    setupOverlays();

    // ------------------------------------------------------------

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ------------------------------------------------------------

    d->escapePreviewAction = new QAction(QIcon::fromTheme(QLatin1String("folder-pictures")),          i18nc("@action",                       "Escape preview"),                 this);
    d->prevAction          = new QAction(QIcon::fromTheme(QLatin1String("go-previous")),              i18nc("@action: go to previous image", "Back"),         this);
    d->nextAction          = new QAction(QIcon::fromTheme(QLatin1String("go-next")),                  i18nc("@action: go to next image",     "Forward"),      this);
    d->rotLeftAction       = new QAction(QIcon::fromTheme(QLatin1String("object-rotate-left")),       i18nc("@action",                       "Rotate Left"),  this);
    d->rotRightAction      = new QAction(QIcon::fromTheme(QLatin1String("object-rotate-right")),      i18nc("@action",                       "Rotate Right"), this);
/*
    FIXME
    d->addPersonAction    = new QAction(QIcon::fromTheme(QLatin1String("list-add-user")),    i18n("Add a Face Tag"),                 this);
    d->forgetFacesAction  = new QAction(QIcon::fromTheme(QLatin1String("list-remove-user")), i18n("Clear all faces on this image"),  this);
    d->peopleToggleAction = new Qaction(QIcon::fromTheme(QLatin1String("im-user")),          i18n("Show Face Tags"),                 this);
    d->peopleToggleAction->setCheckable(true);
*/

    connect(d->prevAction, SIGNAL(triggered()),
            this, SIGNAL(toPreviousImage()));

    connect(d->nextAction, SIGNAL(triggered()),
            this, SIGNAL(toNextImage()));

    connect(d->escapePreviewAction, SIGNAL(triggered()),
            this, SIGNAL(signalEscapePreview()));

    connect(d->rotLeftAction, SIGNAL(triggered()),
            this, SLOT(slotRotateLeft()));

    connect(d->rotRightAction, SIGNAL(triggered()),
            this, SLOT(slotRotateRight()));
/*
    FIXME
    connect(d->peopleToggleAction, SIGNAL(toggled(bool)),
            d->faceGroup, SLOT(setVisible(bool)));

    connect(d->addPersonAction, SIGNAL(triggered()),
            d->faceGroup, SLOT(addFace()));

    connect(d->forgetFacesAction, SIGNAL(triggered()),
            d->faceGroup, SLOT(rejectAll()));
*/
    // ------------------------------------------------------------

    connect(this, SIGNAL(toNextImage()),
            this, SIGNAL(signalNextItem()));

    connect(this, SIGNAL(toPreviousImage()),
            this, SIGNAL(signalPrevItem()));

    connect(this, SIGNAL(activated()),
            this, SIGNAL(signalEscapePreview()));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(ImportSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    slotSetupChanged();
}

ImportPreviewView::~ImportPreviewView()
{
    delete d->item;
    delete d;
}

void ImportPreviewView::setupOverlays()
{
    d->toolBar = new QToolBar(this);

    if (d->mode == IconViewPreview)
    {
        d->toolBar->addAction(d->prevAction);
        d->toolBar->addAction(d->nextAction);
        d->toolBar->addAction(d->escapePreviewAction);
    }

    d->toolBar->addAction(d->rotLeftAction);
    d->toolBar->addAction(d->rotRightAction);
/*
    FIXME
    d->toolBar->addAction(d->peopleToggleAction);
    d->toolBar->addAction(d->addPersonAction);
*/

    // ---

    PanIconWidget* const pan = installPanIcon();

    connect(d->item, &DImgPreviewItem::stateChanged,
            this, [this]()
        {
            updatePanIconWidget();
        }
    );

    // ---

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(d->toolBar,   0, 0, 1, 1);
    grid->addWidget(pan,          2, 2, 1, 1);
    grid->setContentsMargins(QMargins(0, 0,
                                      QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent),
                                      QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent)));
    grid->setSpacing(layoutSpacing());
    grid->setRowStretch(1, 1);
    grid->setColumnStretch(1, 1);
}

void ImportPreviewView::reload()
{
    previewItem()->reload();
}

void ImportPreviewView::camItemLoaded()
{
    Q_EMIT signalPreviewLoaded(true);

    d->rotLeftAction->setEnabled(true);
    d->rotRightAction->setEnabled(true);
/*
    FIXME
    d->faceGroup->setInfo(d->item->camItemInfo());
*/
}

void ImportPreviewView::camItemLoadingFailed()
{
    Q_EMIT signalPreviewLoaded(false);

    d->rotLeftAction->setEnabled(false);
    d->rotRightAction->setEnabled(false);
/*
    FIXME
    d->faceGroup->setInfo(CamItemInfo());
*/
}

void ImportPreviewView::setCamItemInfo(const CamItemInfo& info, const CamItemInfo& previous, const CamItemInfo& next)
{
/*
    FIXME
    d->faceGroup->aboutToSetInfo(info);
*/
    d->item->setCamItemInfo(info);

    d->prevAction->setEnabled(!previous.isNull());
    d->nextAction->setEnabled(!next.isNull());

    QStringList previewPaths;

    if (identifyCategoryforMime(next.mime) == QLatin1String("image"))
    {
        previewPaths << next.url().toLocalFile();
    }

    if (identifyCategoryforMime(previous.mime) == QLatin1String("image"))
    {
        previewPaths << previous.url().toLocalFile();
    }

    d->item->setPreloadPaths(previewPaths);
}

QString ImportPreviewView::identifyCategoryforMime(const QString& mime)
{
    return mime.split(QLatin1Char('/')).at(0);
}

CamItemInfo ImportPreviewView::getCamItemInfo() const
{
    return d->item->camItemInfo();
}

bool ImportPreviewView::acceptsMouseClick(QMouseEvent* e)
{
    if (!GraphicsDImgView::acceptsMouseClick(e))
    {
        return false;
    }

    return true;
/*
    FIXME
    return d->faceGroup->acceptsMouseClick(mapToScene(e->pos()));
*/
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

void ImportPreviewView::enterEvent(QEnterEvent* e)

#else

void ImportPreviewView::enterEvent(QEvent* e)

#endif

{
    Q_UNUSED(e)
/*
    FIXME
    d->faceGroup->enterEvent(e);
*/
}

void ImportPreviewView::leaveEvent(QEvent* e)
{
    Q_UNUSED(e)
/*
    FIXME
    d->faceGroup->leaveEvent(e);
*/
}

void ImportPreviewView::showEvent(QShowEvent* e)
{
    GraphicsDImgView::showEvent(e);
/*
    FIXME
    d->faceGroup->setVisible(d->peopleToggleAction->isChecked());
*/
}

void ImportPreviewView::showContextMenu(const CamItemInfo& info, QGraphicsSceneContextMenuEvent* event)
{
    if (info.isNull())
    {
        return;
    }

    event->accept();

    QList<qlonglong> idList;
    idList << info.id;
    QList<QUrl> selectedItems;
    selectedItems << info.url();

    // --------------------------------------------------------

    QMenu popmenu(this);
    ImportContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction(QLatin1String("importui_fullscreen"));
    cmhelper.addAction(QLatin1String("options_show_menubar"));
    cmhelper.addSeparator();

    // --------------------------------------------------------

    if (d->mode == IconViewPreview)
    {
        cmhelper.addAction(d->prevAction, true);
        cmhelper.addAction(d->nextAction, true);
        cmhelper.addAction(QLatin1String("importui_icon_view"));
/*
        FIXME
        //cmhelper.addGotoMenu(idList);
*/
        cmhelper.addSeparator();
    }

    // --------------------------------------------------------
/*
    FIXME
    cmhelper.addAction(d->peopleToggleAction, true);
    cmhelper.addAction(d->addPersonAction, true);
    cmhelper.addAction(d->forgetFacesAction, true);
    cmhelper.addSeparator();
*/
    // --------------------------------------------------------

    cmhelper.addServicesMenu(selectedItems);
    cmhelper.addRotateMenu(idList);
    cmhelper.addSeparator();

    // --------------------------------------------------------

    cmhelper.addAction(QLatin1String("importui_delete"));
    cmhelper.addSeparator();

    // --------------------------------------------------------
/*
    FIXME
    cmhelper.addAssignTagsMenu(idList);
    cmhelper.addRemoveTagsMenu(idList);
    cmhelper.addSeparator();
*/
    // --------------------------------------------------------

    cmhelper.addLabelsAction();

    // special action handling --------------------------------
/*
    FIXME
    connect(&cmhelper, SIGNAL(signalAssignTag(int)),
           this, SLOT(slotAssignTag(int)));

    connect(&cmhelper, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalPopupTagsView()));

    connect(&cmhelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(slotRemoveTag(int)));

    connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(slotAssignPickLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(slotAssignColorLabel(int)));
*/
    connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
            this, SIGNAL(signalAssignPickLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
            this, SIGNAL(signalAssignColorLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SIGNAL(signalAssignRating(int)));
/*
    FIXME
    connect(&cmhelper, SIGNAL(signalAddToExistingQueue(int)),
            this, SIGNAL(signalAddToExistingQueue(int)));

    connect(&cmhelper, SIGNAL(signalGotoTag(int)),
            this, SIGNAL(signalGotoTagAndItem(int)));
*/
    cmhelper.exec(event->screenPos());
}

/*
FIXME
void ImportPreviewView::slotAssignTag(int tagID)
{
   FileActionMngr::instance()->assignTag(d->item->camItemInfo(), tagID);
}

void ImportPreviewView::slotRemoveTag(int tagID)
{
   FileActionMngr::instance()->removeTag(d->item->camItemInfo(), tagID);
}
*/

void ImportPreviewView::slotThemeChanged()
{
    QPalette plt(palette());
    plt.setColor(backgroundRole(), qApp->palette().color(QPalette::Base));
    setPalette(plt);
}

void ImportPreviewView::slotSetupChanged()
{
    PreviewSettings settings;
    settings.quality = ImportSettings::instance()->getPreviewLoadFullImageSize() ? PreviewSettings::HighQualityPreview
                                                                                 : PreviewSettings::FastPreview;
    previewItem()->setPreviewSettings(settings);

    d->toolBar->setVisible(ImportSettings::instance()->getPreviewShowIcons());
    setShowText(ImportSettings::instance()->getPreviewShowIcons());

    // pass auto-suggest?
}

void ImportPreviewView::slotRotateLeft()
{
/*
    FIXME
    ItemInfo info(d->item->camItemInfo().url().toLocalFile());

    FileActionMngr::instance()->transform(QList<ItemInfo>() << info, MetaEngineRotation::Rotate270);
*/
}

void ImportPreviewView::slotRotateRight()
{
/*
    FIXME
    ItemInfo info(d->item->camItemInfo().url().toLocalFile());

    FileActionMngr::instance()->transform(QList<ItemInfo>() << info, MetaEngineRotation::Rotate90);
*/
}

void ImportPreviewView::slotDeleteItem()
{
    Q_EMIT signalDeleteItem();
}

} // namespace Digikam

#include "importpreviewview.moc"

#include "moc_importpreviewview.cpp"
