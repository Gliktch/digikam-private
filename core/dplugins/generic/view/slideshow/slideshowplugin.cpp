/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to render slideshow.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "slideshowplugin.h"

// Qt includes

#include <QPointer>
#include <QMenu>
#include <QDebug>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <kxmlguiwindow.h>

// Local includes

#include "metaenginesettings.h"
#include "setupslideshow_dialog.h"
#include "slideshowloader.h"

using namespace Digikam;

namespace DigikamGenericSlideShowPlugin
{

SlideShowPlugin::SlideShowPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

QString SlideShowPlugin::name() const
{
    return i18n("SlideShow");
}

QString SlideShowPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon SlideShowPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("view-presentation"));
}

QString SlideShowPlugin::description() const
{
    return i18n("A tool to render slideshow");
}

QString SlideShowPlugin::details() const
{
    return i18n("<p>This tool render a series of items as a simple slide-show.</p>"
                "<p>Plenty of items properties can be displayed as overlay while running.</p>"
                "<p>This tool can play album contents in recursive mode with children albums if any.</p>"
                "<p><u>Usage:</u></p>"

                "<table>"

                    "<tr>"
                        "<td colspan=\"2\"><nobr><center>"
                            "<b><h1>Item Access</h1></b>"
                        "</center></nobr></td>"
                    "</tr>"

                    "<tr><td>Previous Item:</td>"        "<td><i>Up</i> key</td></tr>"
                    "<tr><td></td>"                      "<td><i>PgUp</i> key</td></tr>"
                    "<tr><td></td>"                      "<td><i>Left</i> key</td></tr>"
                    "<tr><td></td>"                      "<td>Mouse wheel up</td></tr>"
                    "<tr><td></td>"                      "<td>Left mouse button</td></tr>"
                    "<tr><td>Next Item:</td>"            "<td><i>Down</i> key</td></tr>"
                    "<tr><td></td>"                      "<td><i>PgDown</i> key</td></tr>"
                    "<tr><td></td>"                      "<td><i>Right</i> key</td></tr>"
                    "<tr><td></td>"                      "<td>Mouse wheel down</td></tr>"
                    "<tr><td></td>"                      "<td>Right mouse button</td></tr>"
                    "<tr><td>Pause/Start:</td>"          "<td><i>Space</i> key</td></tr>"
                    "<tr><td>Delete (to trash):</td>"     "<td><i>Delete</i> key</td></tr>"
                    "<tr><td>Video forward:</td>"        "<td><i>Ctrl+Right</i> key</td></tr>"
                    "<tr><td>Video backward:</td>"       "<td><i>Ctrl+Left</i> key</td></tr>"
                    "<tr><td>Slideshow Settings:</td>"   "<td><i>Alt+P</i> key</td></tr>"
                    "<tr><td>Hide/Show Overlays:</td>"   "<td><i>Alt+Y</i> key</td></tr>"
                    "<tr><td>Quit:</td>"                 "<td><i>Esc</i> key</td></tr>"

                    "<tr>"
                        "<td colspan=\"2\"><nobr><center>"
                            "<b><h1>Item Properties</h1></b>"
                        "</center></nobr></td>"
                    "</tr>"

                    "<tr><td>Change Tags:</td>"          "<td>Use Tags keyboard shortcuts</td></tr>"
                    "<tr><td>Change Rating:</td>"        "<td>Use Rating keyboard shortcuts</td></tr>"
                    "<tr><td>Change Color Label:</td>"   "<td>Use Color label keyboard shortcuts</td></tr>"
                    "<tr><td>Change Pick Label:</td>"    "<td>Use Pick label keyboard shortcuts</td></tr>"

                    "<tr>"
                        "<td colspan=\"2\"><nobr><center>"
                            "<b><h1>Others</h1></b>"
                        "</center></nobr></td>"
                    "</tr>"

                    "<tr><td>Show this help:</td>"       "<td><i>F1</i> key</td></tr>"

                "</table>");
}

QString SlideShowPlugin::handbookSection() const
{
    return QLatin1String("slideshow_tools");
}

QString SlideShowPlugin::handbookChapter() const
{
    return QLatin1String("slide_tool");
}

QList<DPluginAuthor> SlideShowPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Enrico Ros"),
                             QString::fromUtf8("eros dot kde at email dot it"),
                             QString::fromUtf8("(C) 2004"))
            << DPluginAuthor(QString::fromUtf8("Renchi Raju"),
                             QString::fromUtf8("renchi dot raju at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2005"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2025"))
            << DPluginAuthor(QString::fromUtf8("Minh Nghia Duong"),
                             QString::fromUtf8("minhnghiaduong997 at gmail dot com"),
                             QString::fromUtf8("(C) 2019-2021"))
            << DPluginAuthor(QString::fromUtf8("Phuoc Khanh Le"),
                             QString::fromUtf8("phuockhanhnk94 at gmail dot com"),
                             QString::fromUtf8("(C) 2021"))
            << DPluginAuthor(QString::fromUtf8("Fady Khalaf"),
                             QString::fromUtf8("fadykhalaf01 at gmail dot com"),
                             QString::fromUtf8("(C) 2019"))
            ;
}
void SlideShowPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac     = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Slideshow"));
    ac->setObjectName(QLatin1String("slideshow_plugin"));
    ac->setActionCategory(DPluginAction::GenericView);

    const DInfoInterface* const iface = infoIface(ac);

    if (iface && (parent->objectName() == QLatin1String("Digikam")))
    {

        QMenu* const slideShowActions = new QMenu(i18n("Slideshow"));
        slideShowActions->setIcon(icon());
        ac->setMenu(slideShowActions);

        // Action show all

        QAction* const slideShowAllAction = new QAction(i18n("Play All"), ac);
        slideShowAllAction->setObjectName(QLatin1String("slideshow_all"));
        slideShowActions->addAction(slideShowAllAction);

        connect(slideShowAllAction, SIGNAL(triggered()),
                this, SLOT(slotMenuSlideShowAll()));

        // Action show selection

        QAction* const slideShowSelectionAction = new QAction(i18n("Play Selection"), ac);
        slideShowSelectionAction->setObjectName(QLatin1String("slideshow_selected"));
        slideShowActions->addAction(slideShowSelectionAction);

        connect(slideShowSelectionAction, SIGNAL(triggered()),
                this, SLOT(slotMenuSlideShowSelection()));

        // Action show recursive

        QAction* const slideShowRecursiveAction = new QAction(i18n("Play With Sub-Albums"), ac);
        slideShowRecursiveAction->setObjectName(QLatin1String("slideshow_recursive"));
        slideShowActions->addAction(slideShowRecursiveAction);

        connect(slideShowRecursiveAction, SIGNAL(triggered()),
                this, SLOT(slotMenuSlideShowRecursive()));

        connect(ac, SIGNAL(triggered(bool)),
                this, SLOT(slotShowManual()));

        // See bug #425425: register all sub-actions to collection instance to be able to edit keyboard shortcuts

        KXmlGuiWindow* const gui = dynamic_cast<KXmlGuiWindow*>(parent);

        if (gui)
        {
            KActionCollection* const collection = gui->actionCollection();
            collection->setShortcutsConfigurable(slideShowActions->menuAction(), false);

            collection->addAction(slideShowAllAction->objectName(),       slideShowAllAction);
            collection->addAction(slideShowSelectionAction->objectName(), slideShowSelectionAction);
            collection->addAction(slideShowRecursiveAction->objectName(), slideShowRecursiveAction);

            collection->setDefaultShortcut(slideShowAllAction,       Qt::Key_F9);
            collection->setDefaultShortcut(slideShowSelectionAction, Qt::ALT | Qt::Key_F9);
            collection->setDefaultShortcut(slideShowRecursiveAction, Qt::SHIFT | Qt::Key_F9);
        }
    }
    else
    {
        ac->setShortcut(Qt::Key_F9);

        connect(ac, SIGNAL(triggered(bool)),
                this, SLOT(slotMenuSlideShow()));
    }

    addAction(ac);
}

void SlideShowPlugin::addConnectionSlideEnd(QObject* const obj)
{
    Q_UNUSED(obj);
}

void SlideShowPlugin::slotMenuSlideShow()
{
    QUrl startFrom;
    DPluginAction* const ac = dynamic_cast<DPluginAction*>(sender());

    if (ac)
    {
        startFrom = ac->data().toUrl();
        ac->setData(QVariant());
    }

    SlideShowSettings* const settings = new SlideShowSettings();
    settings->iface                   = infoIface(ac);
    settings->readFromConfig();
    settings->exifRotate              = MetaEngineSettings::instance()->settings().exifRotate;
    settings->fileList                = settings->iface->currentAlbumItems();

    slideshow(settings, true, startFrom);
}

void SlideShowPlugin::slotMenuSlideShowAll()
{
    SlideShowSettings* const settings = new SlideShowSettings();
    settings->iface                   = infoIface(sender()->parent());
    settings->readFromConfig();
    settings->fileList                = settings->iface->currentAlbumItems();

    slideshow(settings);
}

void SlideShowPlugin::slotMenuSlideShowSelection()
{
    SlideShowSettings* const settings = new SlideShowSettings();
    settings->iface                   = infoIface(sender()->parent());
    settings->readFromConfig();
    settings->fileList                = settings->iface->currentSelectedItems();

    slideshow(settings);
}

void SlideShowPlugin::slotMenuSlideShowRecursive()
{
    DInfoInterface* const iface = infoIface(sender()->parent());

    connect(iface, SIGNAL(signalAlbumItemsRecursiveCompleted(QList<QUrl>)),
            this, SLOT(slotShowRecursive(QList<QUrl>)));

    iface->parseAlbumItemsRecursive();
}

void SlideShowPlugin::slotShowRecursive(const QList<QUrl>& imageList)
{
    SlideShowSettings* const settings = new SlideShowSettings();
    settings->iface                   = dynamic_cast<DInfoInterface*>(sender());
    settings->readFromConfig();
    settings->fileList                = imageList;

    slideshow(settings);
}

void SlideShowPlugin::slotShowManual()
{
    DPluginAction* const ac = dynamic_cast<DPluginAction*>(sender());

    if (!ac)
    {
        return;
    }

    QUrl startFrom(ac->data().toUrl());
    ac->setData(QVariant());

    SlideShowSettings* const settings = new SlideShowSettings();
    settings->iface                   = infoIface(ac);
    settings->readFromConfig();
    settings->exifRotate              = MetaEngineSettings::instance()->settings().exifRotate;
    settings->fileList                = settings->iface->currentAlbumItems();

    slideshow(settings, !startFrom.isValid(), startFrom);
}

void SlideShowPlugin::slideshow(SlideShowSettings* const settings, bool autoPlayEnabled, const QUrl& startFrom)
{
    settings->autoPlayEnabled = autoPlayEnabled;
    settings->plugin          = this;

    // TODO: preview settings for digikam
    //settings.previewSettings = ApplicationSettings::instance()->getPreviewSettings();

    if (startFrom.isValid())
    {
        settings->imageUrl = startFrom;
    }

    SlideShowLoader* const slide = new SlideShowLoader(settings);
    slide->setShortCutPrefixes(settings->iface->passShortcutActionsToWidget(slide));

    if      (settings->imageUrl.isValid())
    {
        slide->setCurrentItem(settings->imageUrl);
    }
    else if (settings->startWithCurrent)
    {
        if (!settings->iface->currentSelectedItems().isEmpty())
        {
            slide->setCurrentItem(settings->iface->currentSelectedItems().constFirst());
        }
        else
        {
            // no current selection, do nothing.

            return;
        }
    }

    connect(slide, SIGNAL(signalLastItemUrl(QUrl)),
            settings->iface, SIGNAL(signalLastItemUrl(QUrl)));

    connect(settings->iface, SIGNAL(signalShortcutPressed(QString,int)),
            slide, SLOT(slotHandleShortcut(QString,int)));

    slide->show();
}

} // namespace DigikamGenericSlideShowPlugin

#include "moc_slideshowplugin.cpp"
