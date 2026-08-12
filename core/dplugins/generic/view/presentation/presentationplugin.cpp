/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to render presentation.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "presentationplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "presentationmngr.h"

namespace DigikamGenericPresentationPlugin
{

PresentationPlugin::PresentationPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void PresentationPlugin::cleanUp()
{
    delete m_presentationMngr;
}

QString PresentationPlugin::name() const
{
    return i18nc("@title", "Presentation");
}

QString PresentationPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon PresentationPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("view-presentation"));
}

QString PresentationPlugin::description() const
{
    return i18nc("@info", "A tool to render presentation");
}

QString PresentationPlugin::details() const
{
    return i18nc("@info", "This tool render a series of items as an advanced slide-show.<br><br>"
                 "Plenty of transition effects are available are ones based on OpenGL and the famous Ken Burns effect.<br><br>"
                 "You can add a sound-track in background while your presentation.");
}

QString PresentationPlugin::handbookSection() const
{
    return QLatin1String("slideshow_tools");
}

QString PresentationPlugin::handbookChapter() const
{
    return QLatin1String("presentation_tool");
}

QList<DPluginAuthor> PresentationPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Renchi Raju"),
                             QString::fromUtf8("renchi dot raju at gmail dot com"),
                             QString::fromUtf8("(C) 2003-2004"))
            << DPluginAuthor(QString::fromUtf8("Valerio Fuoglio"),
                             QString::fromUtf8("valerio dot fuoglio at gmail dot com"),
                             QString::fromUtf8("(C) 2006-2009"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2026"))
            << DPluginAuthor(QString::fromUtf8("Phuoc Khanh Le"),
                             QString::fromUtf8("phuockhanhnk94 at gmail dot com"),
                             QString::fromUtf8("(C) 2021"))
            << DPluginAuthor(QString::fromUtf8("Fady Khalaf"),
                             QString::fromUtf8("fadykhalaf01 at gmail dot com"),
                             QString::fromUtf8("(C) 2019"))
            ;
}

void PresentationPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Presentation..."));
    ac->setObjectName(QLatin1String("presentation"));
    ac->setActionCategory(DPluginAction::GenericView);
    ac->setShortcut(Qt::ALT | Qt::SHIFT | Qt::Key_F9);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotPresentation()));

    addAction(ac);
}

void PresentationPlugin::slotPresentation()
{
    DInfoInterface* const iface = infoIface(sender());

    delete m_presentationMngr;
    m_presentationMngr = new PresentationMngr(this, iface);

    m_presentationMngr->addFiles(iface->currentSelectedItems());
    m_presentationMngr->setPlugin(this);
    m_presentationMngr->showConfigDialog();
}

} // namespace DigikamGenericPresentationPlugin

#include "moc_presentationplugin.cpp"
