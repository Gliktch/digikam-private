/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to preview image with OpenGL.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "glviewerplugin.h"

// Qt includes

#include <QPointer>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "glviewerwidget.h"
#include "digikam_debug.h"

namespace DigikamGenericGLViewerPlugin
{

GLViewerPlugin::GLViewerPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

QString GLViewerPlugin::name() const
{
    return i18n("OpenGL Viewer");
}

QString GLViewerPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon GLViewerPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("show-gpu-effects"));
}

QString GLViewerPlugin::description() const
{
    return i18n("A tool to preview items with OpenGL");
}

QString GLViewerPlugin::details() const
{
    return i18n("<p>This tool preview a series of items using OpenGL effects.</p>"
                "<p><u>Usage:</u></p>"

                "<table>"

                    "<tr>"
                        "<td colspan=\"2\"><nobr><center>"
                            "<b><h1>Item Access</h1></b>"
                        "</center></nobr></td>"
                    "</tr>"

                    "<tr><td>Previous Item:</td>"                "<td><i>Up</i> key</td></tr>"
                    "<tr><td></td>"                              "<td><i>PgUp</i> key</td></tr>"
                    "<tr><td></td>"                              "<td><i>Left</i> key</td></tr>"
                    "<tr><td></td>"                              "<td>Mouse wheel up</td></tr>"
                    "<tr><td>Next Item:</td>"                    "<td><i>Down</i> key</td></tr>"
                    "<tr><td></td>"                              "<td><i>PgDown</i> key</td></tr>"
                    "<tr><td></td>"                              "<td><i>Right</i> key</td></tr>"
                    "<tr><td></td>"                              "<td>Mouse wheel down</td></tr>"
                    "<tr><td>Quit:</td>"                         "<td><i>Esc</i> key</td></tr>"

                    "<tr>"
                        "<td colspan=\"2\"><nobr><center>"
                            "<b><h1>Item Display</h1></b>"
                        "</center></nobr></td>"
                    "</tr>"

                    "<tr><td>Toggle fullscreen to normal:</td>"  "<td><i>f</i> key</td></tr>"
                    "<tr><td>Toggle scroll-wheel action:</td>"   "<td><i>c</i> key (either zoom or change item)</td></tr>"
                    "<tr><td>Rotation:</td>"                     "<td><i>r</i> key</td></tr>"
                    "<tr><td>Reset view:</td>"                   "<td>double click</td></tr>"
                    "<tr><td>Original size:</td>"                "<td><i>o</i> key</td></tr>"

                    "<tr><td>Zooming:</td>"                      "<td>Move mouse in up-down-direction while pressing the right mouse button</td></tr>"
                    "<tr><td></td>"                              "<td><i>c</i> key and use the scroll-wheel</td></tr>"
                    "<tr><td></td>"                              "<td><i>+</i> and <i>-</i> keys</td></tr>"
                    "<tr><td></td>"                              "<td>ctrl + scrollwheel</td></tr>"

                    "<tr><td>Panning:</td>"                      "<td>Move mouse while pressing the left button</td></tr>"

                    "<tr>"
                        "<td colspan=\"2\"><nobr><center>"
                            "<b><h1>Others</h1></b>"
                        "</center></nobr></td>"
                    "</tr>"

                    "<tr><td>Show this help:</td>"               "<td><i>F1</i> key</td></tr>"

                "</table>");
}

QString GLViewerPlugin::handbookSection() const
{
    return QLatin1String("slideshow_tools");
}

QString GLViewerPlugin::handbookChapter() const
{
    return QLatin1String("opengl_viewer");
}

QList<DPluginAuthor> GLViewerPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Markus Leuthold"),
                             QString::fromUtf8("kusi at forum dot titlis dot org"),
                             QString::fromUtf8("(C) 2007-2008"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2008-2025"))
            ;
}

void GLViewerPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "OpenGL Viewer"));
    ac->setObjectName(QLatin1String("glviewer"));
    ac->setActionCategory(DPluginAction::GenericView);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotGLViewer()));

    addAction(ac);
}

void GLViewerPlugin::slotGLViewer()
{
    DInfoInterface* const iface = infoIface(sender());

    QList<QUrl> myfiles;                                            // pics which are displayed in imageviewer
    QList<QUrl> selection       = iface->currentSelectedItems();
    QString selectedImage;                                          // selected pic in hostapp

    if      (selection.count() == 0)
    {
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "no item selected, load entire album";
        myfiles = iface->currentAlbumItems();
    }
    else if (selection.count() == 1)
    {
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "one item selected, load entire album and start with selected item";
        selectedImage = selection.first().toLocalFile();
        myfiles       = iface->currentAlbumItems();
    }
    else if (selection.count() > 1)
    {
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "load" << selection.count() << "selected images";
        myfiles = selection;
    }

    if (myfiles.isEmpty())
    {
        return;
    }

    QPointer<GLViewerWidget> view = new GLViewerWidget(this, iface, myfiles, selectedImage);

    switch (view->getOGLstate())
    {
        case oglOK:
        {
            view->show();
            break;
        }

        case oglNoRectangularTexture:
        {
            qCCritical(DIGIKAM_DPLUGIN_GENERIC_LOG) << "GL_ARB_texture_rectangle not supported";
            QMessageBox::critical(nullptr, i18n("OpenGL Error"),
                                  i18n("GL_ARB_texture_rectangle OpenGL extension is not supported on your system"));
            view->close();
            break;
        }

        case oglNoContext:
        {
            qCCritical(DIGIKAM_DPLUGIN_GENERIC_LOG) << "no OpenGL context found";
            QMessageBox::critical(nullptr, i18n("OpenGL Error"),
                                  i18n("No OpenGL context has been found"));
            view->close();
            break;
        }
    }
}

} // namespace DigikamGenericGLViewerPlugin

#include "moc_glviewerplugin.cpp"
