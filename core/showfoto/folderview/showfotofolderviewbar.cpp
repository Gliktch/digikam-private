/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-08-27
 * Description : a tool bar for Showfoto folder view
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotofolderviewbar.h"

// Qt includes

#include <QDir>
#include <QApplication>
#include <QSlider>
#include <QLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QPixmap>
#include <QActionGroup>
#include <QMenu>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "drawdecoder.h"
#include "showfotofolderviewmodel.h"
#include "showfotofolderviewlist.h"
#include "showfotosetup.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoFolderViewBar::Private
{

public:

    Private() = default;

    /**
     * Identify plugins category to host in folder-view.
     */
    const QString              pluginFingerPrint    = QLatin1String("DPlugin::Generic::View");

    QToolButton*               previousBtn          = nullptr;
    QToolButton*               nextBtn              = nullptr;
    QToolButton*               upBtn                = nullptr;
    QToolButton*               homeBtn              = nullptr;
    QSlider*                   iconSizeSlider       = nullptr;
    QToolButton*               optionsBtn           = nullptr;
    QMenu*                     optionsMenu          = nullptr;
    QToolButton*               runBtn               = nullptr;
    QMenu*                     runMenu              = nullptr;
    QAction*                   shortAction          = nullptr;
    QAction*                   detailedAction       = nullptr;
    QAction*                   showBookmarksAction  = nullptr;
    QAction*                   moreSettingsAction   = nullptr;
    QComboBox*                 pathEdit             = nullptr;

    /**
     * Used to shared actions with list-view context menu.
     */
    QList<QAction*>            actionsList;

    ShowfotoFolderViewSideBar* sidebar              = nullptr;
    QComboBox*                 typeMimesCombo       = nullptr;
};

ShowfotoFolderViewBar::ShowfotoFolderViewBar(ShowfotoFolderViewSideBar* const sidebar)
    : DVBox(sidebar),
      d    (new Private)
{
    setContentsMargins(QMargins());

    d->sidebar               = sidebar;
    DHBox* const btnBox      = new DHBox(this);
    QAction* btnAction       = nullptr;

    // ---

    btnAction                = new QAction(this);
    btnAction->setObjectName(QLatin1String("GoPrevious"));
    btnAction->setIcon(QIcon::fromTheme(QLatin1String("go-previous")));
    btnAction->setText(i18nc("@action", "Go to Previous"));
    btnAction->setToolTip(i18nc("@info", "Go to previous place in folder-view hierarchy"));

    connect(btnAction, SIGNAL(triggered(bool)),
            this, SIGNAL(signalGoPrevious()));

    d->actionsList << btnAction;

    d->previousBtn           = new QToolButton(btnBox);
    d->previousBtn->setDefaultAction(btnAction);
    d->previousBtn->setFocusPolicy(Qt::NoFocus);

    // ---

    btnAction                = new QAction(this);
    btnAction->setObjectName(QLatin1String("GoNext"));
    btnAction->setIcon(QIcon::fromTheme(QLatin1String("go-next")));
    btnAction->setText(i18nc("@action", "Go to Next"));
    btnAction->setToolTip(i18nc("@info", "Go to next place in folder-view hierarchy"));

    connect(btnAction, SIGNAL(triggered(bool)),
            this, SIGNAL(signalGoNext()));

    d->actionsList << btnAction;

    d->nextBtn               = new QToolButton(btnBox);
    d->nextBtn->setDefaultAction(btnAction);
    d->nextBtn->setFocusPolicy(Qt::NoFocus);

    // ---

    btnAction                = new QAction(this);
    btnAction->setObjectName(QLatin1String("GoUp"));
    btnAction->setIcon(QIcon::fromTheme(QLatin1String("go-up")));
    btnAction->setText(i18nc("@action", "Go Up"));
    btnAction->setToolTip(i18nc("@info", "Go up in folder-view hierarchy"));

    connect(btnAction, SIGNAL(triggered(bool)),
            this, SIGNAL(signalGoUp()));

    d->actionsList << btnAction;

    d->upBtn                 = new QToolButton(btnBox);
    d->upBtn->setDefaultAction(btnAction);
    d->upBtn->setFocusPolicy(Qt::NoFocus);

    // ---

    btnAction                = new QAction(this);
    btnAction->setObjectName(QLatin1String("GoHome"));
    btnAction->setIcon(QIcon::fromTheme(QLatin1String("go-home")));
    btnAction->setText(i18nc("@action", "Go Home"));
    btnAction->setToolTip(i18nc("@info", "Go to home directory"));

    connect(btnAction, SIGNAL(triggered(bool)),
            this, SIGNAL(signalGoHome()));

    d->actionsList << btnAction;

    d->homeBtn               = new QToolButton(btnBox);
    d->homeBtn->setDefaultAction(btnAction);
    d->homeBtn->setFocusPolicy(Qt::NoFocus);

    // ---

    d->iconSizeSlider        = new QSlider(Qt::Horizontal, btnBox);
    d->iconSizeSlider->setRange(16, ShowfotoFolderViewModel::maxIconSize());
    d->iconSizeSlider->setSingleStep(10);
    d->iconSizeSlider->setValue(32);
    d->iconSizeSlider->setFocusPolicy(Qt::NoFocus);
    btnBox->setStretchFactor(d->iconSizeSlider, 10);

    connect(d->iconSizeSlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotIconSizeChanged(int)));

    slotIconSizeChanged(32);

    // ---

    d->optionsBtn              = new QToolButton(btnBox);
    d->optionsBtn->setToolTip(i18nc("@info: folder-view", "View Options"));
    d->optionsBtn->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    d->optionsBtn->setPopupMode(QToolButton::InstantPopup);
    d->optionsBtn->setWhatsThis(i18nc("@info: folder-view", "Selection options to render folder-view."));

    d->optionsMenu             = new QMenu(d->optionsBtn);
    QActionGroup* const optGrp = new QActionGroup(this);

    d->shortAction             = d->optionsMenu->addAction(i18nc("@action:inmenu", "Short View"));
    d->shortAction->setObjectName(QLatin1String("ShortView"));
    d->shortAction->setIcon(QIcon::fromTheme(QLatin1String("view-list-text")));
    d->shortAction->setToolTip(i18nc("@info", "Switch Folder-View to Short Mode"));
    d->shortAction->setCheckable(true);
    optGrp->addAction(d->shortAction);

    d->actionsList << d->shortAction;

    d->detailedAction          = d->optionsMenu->addAction(i18nc("@action:inmenu", "Detailed View"));
    d->detailedAction->setObjectName(QLatin1String("DetailledView"));
    d->detailedAction->setIcon(QIcon::fromTheme(QLatin1String("view-list-details")));
    d->detailedAction->setToolTip(i18nc("@info", "Switch Folder-View to Detailed Mode"));
    d->detailedAction->setCheckable(true);
    optGrp->addAction(d->detailedAction);

    d->actionsList << d->detailedAction;

    optGrp->setExclusive(true);

    d->showBookmarksAction      = d->optionsMenu->addAction(i18nc("@action:inmenu", "Show Bookmarks"));
    d->showBookmarksAction->setObjectName(QLatin1String("ShowBookmarks"));
    d->showBookmarksAction->setIcon(QIcon::fromTheme(QLatin1String("bookmarks")));
    d->showBookmarksAction->setToolTip(i18nc("@info", "Show or Hide the Bookmarked places"));
    d->showBookmarksAction->setCheckable(true);

    d->actionsList << d->showBookmarksAction;

    d->moreSettingsAction       = d->optionsMenu->addAction(i18nc("@action:inmenu", "More Settings..."));
    d->moreSettingsAction->setObjectName(QLatin1String("MoreSettings"));
    d->moreSettingsAction->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    d->moreSettingsAction->setToolTip(i18nc("@info", "Open configure dialog with more settings options"));

    d->actionsList << d->moreSettingsAction;

    d->optionsBtn->setMenu(d->optionsMenu);

    connect(d->optionsMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOptionsChanged(QAction*)));

    // ---

    d->runBtn                = new QToolButton(btnBox);
    d->runBtn->setToolTip(i18nc("@info: folder-view", "Stack Options"));
    d->runBtn->setIcon(QIcon::fromTheme(QLatin1String("layer-visible-on")));
    d->runBtn->setPopupMode(QToolButton::InstantPopup);
    d->runBtn->setWhatsThis(i18nc("@info: folder-view", "Stack options to handle folder-view contents."));

    d->runMenu               = new QMenu(d->runBtn);

    btnAction                = d->runMenu->addAction(i18nc("@action:inmenu", "Load Contents"));
    btnAction->setObjectName(QLatin1String("LoadContents"));
    btnAction->setIcon(QIcon::fromTheme(QLatin1String("media-playlist-normal")));
    btnAction->setText(i18nc("@action", "Load Contents"));
    btnAction->setToolTip(i18nc("@info", "Load contents to editor stack"));

    connect(btnAction, SIGNAL(triggered(bool)),
            this, SIGNAL(signalLoadContents()));

    d->actionsList << btnAction;

    btnAction                = d->runMenu->addAction(i18nc("@action:inmenu", "Append Contents"));
    btnAction->setObjectName(QLatin1String("AppendContents"));
    btnAction->setIcon(QIcon::fromTheme(QLatin1String("media-playlist-append")));
    btnAction->setText(i18nc("@action", "Append Contents"));
    btnAction->setToolTip(i18nc("@info", "Append contents to editor stack"));

    connect(btnAction, SIGNAL(triggered(bool)),
            this, SIGNAL(signalAppendContents()));

    d->actionsList << btnAction;

    d->runBtn->setMenu(d->runMenu);

    // ---

    d->pathEdit              = new QComboBox(this);
    d->pathEdit->setEditable(true);
    d->pathEdit->setDuplicatesEnabled(false);
    d->pathEdit->setWhatsThis(i18nc("@info", "Enter the customized folder-view path or select from history"));
    d->pathEdit->lineEdit()->setPlaceholderText(i18nc("@info: folder-view path edit", "Enter local path here..."));
    d->pathEdit->lineEdit()->setClearButtonEnabled(true);

    connect(d->pathEdit->lineEdit(), SIGNAL(returnPressed()),
            this, SLOT(slotCustomPathChanged()));

    connect(d->pathEdit, SIGNAL(textActivated(QString)),
            this, SIGNAL(signalCustomPathChanged(QString)));

    // ---

    d->typeMimesCombo = new QComboBox(this);
    d->typeMimesCombo->insertItem(TYPE_MIME_JPEG,  i18nc("@item: filter type mime", "JPEG images"),         QLatin1String("*.JPEG *.JPG *.JPE"));
    d->typeMimesCombo->insertItem(TYPE_MIME_TIFF,  i18nc("@item: filter type mime", "TIFF images"),         QLatin1String("*.TIFF *.TIF"));
    d->typeMimesCombo->insertItem(TYPE_MIME_PNG,   i18nc("@item: filter type mime", "PNG images"),          QLatin1String("*.PNG"));
    d->typeMimesCombo->insertItem(TYPE_MIME_PGF,   i18nc("@item: filter type mime", "PGF images"),          QLatin1String("*.PGF"));
    d->typeMimesCombo->insertItem(TYPE_MIME_HEIF,  i18nc("@item: filter type mime", "HEIF images"),         QLatin1String("*.HEIF *.HEIC"));
    d->typeMimesCombo->insertItem(TYPE_MIME_AVIF,  i18nc("@item: filter type mime", "AVIF images"),         QLatin1String("*.AVIF"));
    d->typeMimesCombo->insertItem(TYPE_MIME_JXL,   i18nc("@item: filter type mime", "JPEG-XL images"),      QLatin1String("*.JXL"));
    d->typeMimesCombo->insertItem(TYPE_MIME_WEBP,  i18nc("@item: filter type mime", "WEBP images"),         QLatin1String("*.WEBP"));
    d->typeMimesCombo->insertItem(TYPE_MIME_DNG,   i18nc("@item: filter type mime", "DNG images"),          QLatin1String("*.DNG"));
    d->typeMimesCombo->insertItem(TYPE_MIME_RAW,   i18nc("@item: filter type mime", "RAW images"),          DRawDecoder::rawFiles());

    QString filter;
    QString patterns;
    (void)supportedImageMimeTypes(QIODevice::WriteOnly, filter);
    patterns = filter.toLower();
    patterns.append(QLatin1Char(' '));
    patterns.append(filter.toUpper());

    d->typeMimesCombo->insertItem(TYPE_MIME_NORAW, i18nc("@item: filter type mime", "No RAW images"),       patterns);

    (void)supportedImageMimeTypes(QIODevice::ReadOnly, filter);
    patterns  = filter.toLower();
    patterns.append(QLatin1Char(' '));
    patterns.append(filter.toUpper());

    d->typeMimesCombo->insertItem( TYPE_MIME_ALL,  i18nc("@item: filter type mime", "All supported files"), patterns);

    connect(d->typeMimesCombo, SIGNAL(activated(int)),
            this, SLOT(slotTypeMimesChanged(int)));
}

ShowfotoFolderViewBar::~ShowfotoFolderViewBar()
{
    delete d;
}

void ShowfotoFolderViewBar::slotTypeMimesChanged(int index)
{
    Q_EMIT signalTypeMimesChanged(d->typeMimesCombo->itemData(index).toString());
}

QAction* ShowfotoFolderViewBar::toolBarAction(const QString& name) const
{
    for (QAction* const act : std::as_const(d->actionsList))
    {
        if (act && (act->objectName() == name))
        {
            return act;
        }
    }

    return nullptr;
}

QList<QAction*> ShowfotoFolderViewBar::pluginActions() const
{
    QList<QAction*> lst;

    for (QAction* const act : std::as_const(d->actionsList))
    {
        if (act && (act->data() == d->pluginFingerPrint))
        {
            lst << act;
        }
    }

    return lst;
}

void ShowfotoFolderViewBar::setFolderViewTypeMime(int mime)
{
    d->typeMimesCombo->setCurrentIndex(mime);
    slotTypeMimesChanged(d->typeMimesCombo->currentIndex());
}

int ShowfotoFolderViewBar::folderViewTypeMime() const
{
    return d->typeMimesCombo->currentIndex();
}

void ShowfotoFolderViewBar::setFolderViewMode(int mode)
{
    if (mode == ShowfotoFolderViewList::ShortView)
    {
        d->shortAction->setChecked(true);
    }
    else
    {
        d->detailedAction->setChecked(true);
    }
}

int ShowfotoFolderViewBar::folderViewMode() const
{
    if (d->shortAction->isChecked())
    {
        return ShowfotoFolderViewList::ShortView;
    }

    return ShowfotoFolderViewList::DetailledView;
}

void ShowfotoFolderViewBar::setBookmarksVisible(bool b)
{
    d->showBookmarksAction->setChecked(b);

    Q_EMIT signalShowBookmarks(b);
}

bool ShowfotoFolderViewBar::bookmarksVisible() const
{
    return (d->showBookmarksAction->isChecked());
}

void ShowfotoFolderViewBar::setCurrentPath(const QString& path)
{
    d->pathEdit->lineEdit()->setText(path);

    if (d->pathEdit->findText(path) == -1)
    {
        d->pathEdit->addItem(path);
    }

    QDir dir(path);
    d->upBtn->setEnabled(dir.cdUp());
}

QString ShowfotoFolderViewBar::currentPath() const
{
    return d->pathEdit->lineEdit()->text();
}

void ShowfotoFolderViewBar::setIconSize(int size)
{
    d->iconSizeSlider->setValue(size);
}

int ShowfotoFolderViewBar::iconSize() const
{
    return d->iconSizeSlider->value();
}

void ShowfotoFolderViewBar::slotCustomPathChanged()
{
    QString path = d->pathEdit->lineEdit()->text();

    if (d->pathEdit->findText(path) == -1)
    {
        d->pathEdit->addItem(path);
    }

    Q_EMIT signalCustomPathChanged(path);
}

void ShowfotoFolderViewBar::slotPreviousEnabled(bool b)
{
    d->previousBtn->setEnabled(b);
}

void ShowfotoFolderViewBar::slotNextEnabled(bool b)
{
    d->nextBtn->setEnabled(b);
}

void ShowfotoFolderViewBar::slotIconSizeChanged(int size)
{
    d->iconSizeSlider->setToolTip(i18nc("@info", "Icon Size: %1", size));

    Q_EMIT signalIconSizeChanged(size);
}

void ShowfotoFolderViewBar::slotOptionsChanged(QAction* action)
{
    if      (action == d->shortAction)
    {
        Q_EMIT signalViewModeChanged(ShowfotoFolderViewList::ShortView);

        return;
    }
    else if (action == d->detailedAction)
    {
        Q_EMIT signalViewModeChanged(ShowfotoFolderViewList::DetailledView);

        return;
    }
    else if (action == d->showBookmarksAction)
    {
        Q_EMIT signalShowBookmarks(d->showBookmarksAction->isChecked());

        return;
    }
    else if (action == d->moreSettingsAction)
    {
        Q_EMIT signalSetup();

        return;
    }
}

void ShowfotoFolderViewBar::registerPluginActions(const QList<DPluginAction*>& actions)
{
    if (!actions.isEmpty())
    {
        d->runMenu->addSeparator();

        connect(d->runMenu, SIGNAL(triggered(QAction*)),
                this, SIGNAL(signalPluginActionTriggered(QAction*)));

        for (QAction* const dpact : std::as_const(actions))
        {
            QAction* const act = d->runMenu->addAction(dpact->text());
            act->setObjectName(dpact->objectName());
            act->setIcon(dpact->icon());
            act->setToolTip(dpact->toolTip());
            act->setData(d->pluginFingerPrint);

            d->actionsList << act;

            d->runMenu->addAction(act);
        }
    }
}

} // namespace ShowFoto

#include "moc_showfotofolderviewbar.cpp"
